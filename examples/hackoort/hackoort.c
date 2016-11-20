/* hackoort.c
 *
 * Copyright (C) 2016 Mariusz Woloszyn <emsi@nosuchdomain.example>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <endian.h>

#include "gattlib.h"

short verbose=1,force=0,dry_run=0;

gatt_connection_t* connection;
unsigned int seq=1; // oort devices are using sequence numbers for each request

char* bt_address="84:eb:18:7d:f8:4f";
char* password="D000000"; // default password used when no password is set

char *command=NULL, **arguments=NULL;
unsigned short arguments_count=0;

typedef char hackoort_cmd[2];

static void usage() {
	printf("USAGE: hackoort [OPTION] COMMAND [command arguments]\n\n");
	printf("Commands:\n");
	printf("  ON                    Turn the OORT device ON\n");
	printf("  OFF                   Turn the OORT device OFF\n");
	printf("  BRIGHTNESS pct	Set the OORT bulb brightness to 'pct' percents\n");
	printf("\n");
	printf("Options:\n"
	"  -d, --devide_address ADDR    OORT Device address\n"
	"  -p, --password PASSWOD       Unlock the OORT device using PASSWORD\n"
	"                               \n"
	"  -f, --force                  Try to force operation\n"
	"  -v, --verbose LEVEL          Be verbose (print more debug)\n"
	"  --dry-run                    Dry run (do not communicat eover BT)\n"
	"  -h, --help                   Print this help message\n"
	"\n");
	exit (-1);
}

int parse_opts(int argc, char **argv)
{
	int option_index = 0,opt,i;
	static struct option loptions[] = {
		{"help",0,0,'h'},
		{"device_address",1,0,'d'},
		{"password",1,0,'p'},
		{"force",0,0,'f'},
		{"verbose",2,0,'v'},
		{"dry-run",0,0,0},
		{0,0,0,0}
	};
	while ((opt = getopt_long(argc,argv,"hd:p:fv:", loptions, &option_index))!=-1) {
		switch (opt) {
			case 0:
			    if (!(strcmp("dry-run", loptions[option_index].name)))
				dry_run=1;
			case 'f':
				force=~force;
				break;
			case 'v':
				if (optarg) 
					verbose=strtol(optarg,NULL,10);
				else
					verbose=1;
				/*printf("VERBOSE LEVEL  = %i\n",verbose);*/
				break;
			case 'h':
				usage();
				break;
		}
	}
	if (optind<=argc) {
		command=argv[optind];
	}
	if (optind++<=argc) {
		arguments=&(argv[optind]);
		arguments_count=argc-optind;
	}
	if (verbose>3) printf("CMD: %s, arg[last]: %s, argc: %i\n", command, arguments[arguments_count-1], arguments_count);
}			

int check_lock_status()
{
	int len;
	unsigned char ret;
	static bt_uuid_t g_uuid;

	/* read pass status, handle 0x2a */
	bt_string_to_uuid(&g_uuid, "a8b3fff4-4834-4051-89d0-3de95cddd318"); 
	len = gattlib_read_char_by_uuid(connection, &g_uuid, &ret, sizeof(ret));
	if (verbose) printf("Unlocked: %02x\n", ret);
	return ret;
}

char* hackoort_read_characteristic()
{
	int i,len;
	static unsigned char ret[5];
	static bt_uuid_t g_uuid;

	/* read pass status, handle 0x2a */
	bt_string_to_uuid(&g_uuid, "a8b3fff2-4834-4051-89d0-3de95cddd318");
	len = gattlib_read_char_by_uuid(connection, &g_uuid, &ret, sizeof(ret));
	if (verbose) {
		printf("Characteristic ");
		for (i = 0; i < len; i++)
			printf("%02x ", ret[i]);
		printf("\n");
	}
	return ret;
}

int aa0afc3a8600(hackoort_cmd cmd, char* data, int datalen)
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
	buffer[i++]=sum; // checksum
	buffer[i++]='\x0d'; // terminator?

	if (verbose>1) {
	    printf("CMD: %02x%02x, data:", cmd[0],cmd[1]);
	    for (j=0;j<datalen;j++)
		printf(" %02x", data[j]);
	    printf(", SEQ: %02x, SUM: %02x\n", seq, sum);
	}
	if (verbose>3) {
	    printf("RAW data: ");
	    for (j=0;j<i;j++) 
			printf("%02x", buffer[j]);
	    printf(" EOF datalen: %u\n",datalen);
	}


	/* handle 0x21 */
	if (!dry_run) {
	    ret = gattlib_write_char_by_handle(connection, 0x21, buffer, i);
	    assert(ret == 0);
	}
	seq++;
	return ret;
}

int hackoort_onoff(unsigned char on)
{
    if (verbose) {
	printf("Setting device %s\n",on?"ON":"OFF");
    }
    return aa0afc3a8600("\x0a\x01", (void*) &on, 1);
}

int hackoort_set_luminance(unsigned char lum)
{
    if (verbose) printf ("Setting luminance to %02x\n", lum);
    return aa0afc3a8600("\x0c\x01", (void*) &lum, 1);
}

int hackoort_set_luminance_pct(unsigned char pct)
{
    unsigned char lum=2+(9.0*(pct>100?100:pct)/100.0);
    return hackoort_set_luminance(lum);
}

// Take string input like 0x1222 or 0a0406 and return bufer with bytes of such value
char* read_hex_data(char* str,unsigned char len)
{
    static uint64_t num;
    long unsigned l=0;
    l=sscanf(str, "%12llx", &num);
    num=htole64(num);
    if (verbose>4) printf("HEX: 0x%llx l: %lx \n", num, l);
    char* const buff = (char*)malloc(len);
    memcpy(buff,&num,len);
    return buff;
}

int parse_command(int argc, char** argv)
{
	static unsigned short prime=0;
	unsigned short args=0; // number of arguments for current command including command

	if (verbose >2) printf("EXECUTING COMMAND: %s\n", command);

	if (strcmp(command, "ON") == 0)	
		hackoort_onoff(1);
	else if (strcmp(command, "OFF") == 0)
		hackoort_onoff(0);
	else if (strcmp(command, "LUMINANCE") == 0) {
		args++;
		hackoort_set_luminance_pct(strtol(arguments[0],NULL,10));
		}
	else if (strcmp(command, "RAW") == 0) {
		unsigned char len=strlen(arguments[1])/2;
		char* cmd=read_hex_data(arguments[0],2);
		char* data=read_hex_data(arguments[1],len);
		args+=2;
		aa0afc3a8600(cmd, data, len);
		free(cmd);free(data);
		}
	if (!prime) { 
		prime=1;
		if (verbose) printf("BUG WORKAROUND, repeating first command\n");
		parse_command(argc,argv);
	}
	optind+=args;
	// Execute next command in line if any
	if (optind<argc) {
		command=argv[optind];
		if (optind++<=argc) {
			arguments=&(argv[optind]);
			arguments_count=argc-optind;
		}
		parse_command(argc,argv);
	}
}


int main(int argc, char *argv[]) {
	uint8_t buffer[100];
	int i, len, ret;
	static bt_uuid_t g_uuid;


	//aa0afc3a8600("\x0a\x01", (void*) "\x01",1, 0x46);
	// 0d06 01ff00 3c0000
	//aa0afc3a8600("\x0d\x06", (void*) "\x01\xff\x00\x3c\x00\x00",6, 0x01);
	//return 0;

	parse_opts(argc,argv);

	connection = gattlib_connect(NULL, bt_address, BDADDR_LE_PUBLIC, BT_IO_SEC_LOW, 0, 0);
	if (connection == NULL) {
		fprintf(stderr, "Fail to connect to the bluetooth device.\n");
		return 1;
	}

	/* read pass status */
	check_lock_status();

	// char-write-req 27 44303030303030
	ret = gattlib_write_char_by_handle(connection, 0x27, password, strlen(password));
	assert(ret == 0);

	/* read pass status */
	check_lock_status();
	
	parse_command(argc, argv);

	/*
	for (i=0;i<120; i++) {
		hackoort_set_luminance_pct(i);
		usleep(1000);
	}


	hackoort_onoff(0);
	hackoort_read_characteristic();

	sleep (2);
	hackoort_onoff(1);
	hackoort_read_characteristic();


	for (i=0x0b; i>1; i--) {
	    hackoort_set_luminance(i);
	    hackoort_read_characteristic();
	    usleep(10000);
	}
	*/
	
	gattlib_disconnect(connection);
	return 0;
}
