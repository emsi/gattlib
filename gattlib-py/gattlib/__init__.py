from ctypes import *

gattlib = CDLL("libgattlib.so")


# typedef struct {
#    uint8_t data[16];
# } uint128_t;
class GattlibUuid128(Structure):
    _fields_ = [("data", c_byte * 16)]


# typedef struct {
#    uint8_t type;
#    union {
#        uint16_t  uuid16;
#        uint32_t  uuid32;
#        uint128_t uuid128;
#    } value;
# } uuid_t;
class GattlibUuidValue(Union):
    _fields_ = [("uuid16", c_ushort), ("uuid32", c_uint), ("uuid128", GattlibUuid128)]


class GattlibUuid(Structure):
    _fields_ = [("type", c_byte), ("value", GattlibUuidValue)]


# typedef struct {
#    uint16_t  attr_handle_start;
#    uint16_t  attr_handle_end;
#    uuid_t    uuid;
# } gattlib_primary_service_t;
class GattlibPrimaryService(Structure):
    _fields_ = [("attr_handle_start", c_ushort),
                ("attr_handle_end", c_ushort),
                ("uuid", GattlibUuid)]


# typedef struct {
#    uint16_t  handle;
#    uint8_t   properties;
#    uint16_t  value_handle;
#    uuid_t    uuid;
# } gattlib_characteristic_t;
class GattlibCharacteristic(Structure):
    _fields_ = [("handle", c_ushort),
                ("properties", c_byte),
                ("value_handle", c_ushort),
                ("uuid", GattlibUuid)]


# int gattlib_adapter_open(const char* adapter_name, void** adapter);
gattlib_adapter_open = gattlib.gattlib_adapter_open
gattlib_adapter_open.argtypes = [c_char_p, POINTER(c_void_p)]

# typedef void (*gattlib_discovered_device_t)(const char* addr, const char* name)
gattlib_discovered_device_type = CFUNCTYPE(None, c_char_p, c_char_p)

# typedef void (*gattlib_event_handler_t)(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data);
gattlib_event_handler_type = CFUNCTYPE(None, POINTER(GattlibUuid), POINTER(c_byte), c_int, c_void_p)

# int gattlib_discover_primary(gatt_connection_t* connection, gattlib_primary_service_t** services, int* services_count);
gattlib_discover_primary = gattlib.gattlib_discover_primary
gattlib_discover_primary.argtypes = [c_void_p, POINTER(POINTER(GattlibPrimaryService)), POINTER(c_int)]

# int gattlib_discover_char(gatt_connection_t* connection, gattlib_characteristic_t** characteristics, int* characteristic_count);
gattlib_discover_char = gattlib.gattlib_discover_char
gattlib_discover_char.argtypes = [c_void_p, POINTER(POINTER(GattlibCharacteristic)), POINTER(c_int)]
