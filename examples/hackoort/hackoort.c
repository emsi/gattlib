/*
 *
 *
 */

#include <assert.h>
#include <stdio.h>

#include "gattlib.h"

int debug=1;

unsigned int seq=1;

char* bt_address="84:eb:18:7d:f8:4f";
char* password="D000000";

typedef char hackoort_cmd[2];

static void usage(char *argv[]) {
	printf("%s <device_address> <read|write> <uuid> [<hex-value-to-write>]\n", argv[0]);
}


int check_lock_status(gatt_connection_t* connection)
{
	int len;
	unsigned char ret;
	static bt_uuid_t g_uuid;

	/* read pass status, handle 0x2a */
	bt_string_to_uuid(&g_uuid, "a8b3fff4-4834-4051-89d0-3de95cddd318"); 
	len = gattlib_read_char_by_uuid(connection, &g_uuid, &ret, sizeof(ret));
	if (debug) printf("%02x\n", ret);
	return ret;
}

char* hackoort_read_characteristic(gatt_connection_t* connection)
{
	int i,len;
	static unsigned char ret[5];
	static bt_uuid_t g_uuid;

	/* read pass status, handle 0x2a */
	bt_string_to_uuid(&g_uuid, "a8b3fff2-4834-4051-89d0-3de95cddd318");
	len = gattlib_read_char_by_uuid(connection, &g_uuid, &ret, sizeof(ret));
	if (debug) {
		for (i = 0; i < len; i++)
			printf("%02x ", ret[i]);
		printf("\n");
	}
	return ret;
}

int aa0afc3a8600(gatt_connection_t* connection, hackoort_cmd cmd, char* data, int datalen)
{
	unsigned char buffer[]="\xaa\x0a\xfc\x3a\x86\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	int i,j,ret,len=12;
	unsigned char sum=0xab;

	buffer[6]=cmd[0];
	buffer[7]=cmd[1];

	for (i=0;i<datalen;i++)
	    buffer[8+i]=((char *)data)[i];
	i=i+++8;  // this is plain evil ;)
	buffer[i++]=seq; // Sequence number
	
	for (j=0;j<i;j++) {
			sum+=buffer[j];
	}
	buffer[i++]=sum;
	buffer[i++]='\x0d';

	for (j=0;j<i;j++) {
			if (debug) printf("%02x", buffer[j]);
	}
	if (debug) printf(" EOF\n");

	/* handle 0x21 */
	ret = gattlib_write_char_by_handle(connection, 0x21, buffer, i);
	assert(ret == 0);
	seq++;
	return ret;
}

int hackoort_onoff(gatt_connection_t* connection, unsigned char on)
{
    aa0afc3a8600(connection, "\x0a\x01", (void*) &on, 1);
    // read status!
}



int main(int argc, char *argv[]) {
	uint8_t buffer[100];
	int i, len, ret;
	static bt_uuid_t g_uuid;
	gatt_connection_t* connection;


	//aa0afc3a8600(connection, "\x0a\x01", (void*) "\x01",1, 0x46);
	// 0d06 01ff00 3c0000
	//aa0afc3a8600(connection, "\x0d\x06", (void*) "\x01\xff\x00\x3c\x00\x00",6, 0x01);
	//return 0;
	

	connection = gattlib_connect(NULL, bt_address, BDADDR_LE_PUBLIC, BT_IO_SEC_LOW, 0, 0);
	if (connection == NULL) {
		fprintf(stderr, "Fail to connect to the bluetooth device.\n");
		return 1;
	}

	/* read pass status */
	check_lock_status(connection);

	// char-write-req 27 44303030303030
	ret = gattlib_write_char_by_handle(connection, 0x27, password, strlen(password));
	assert(ret == 0);

	/* read pass status */
	check_lock_status(connection);

	hackoort_read_characteristic(connection);
	hackoort_onoff(connection, 0);

	sleep (3);
	hackoort_read_characteristic(connection);
	hackoort_onoff(connection,1);

	gattlib_disconnect(connection);
	return 0;
}
