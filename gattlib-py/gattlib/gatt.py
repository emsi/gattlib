from gattlib import *
from .uuid import gattlib_uuid_to_uuid, gattlib_uuid_to_int


class GattService():

    def __init__(self, device, gattlib_primary_service):
        self._device = device
        self._gattlib_primary_service = gattlib_primary_service

    @property
    def uuid(self):
        return gattlib_uuid_to_uuid(self._gattlib_primary_service.uuid)

    @property
    def short_uuid(self):
        return gattlib_uuid_to_int(self._gattlib_primary_service.uuid)


class GattCharacteristic():

    def __init__(self, device, gattlib_characteristic):
        self._device = device
        self._gattlib_characteristic = gattlib_characteristic

    @property
    def uuid(self):
        return gattlib_uuid_to_uuid(self._gattlib_characteristic.uuid)

    @property
    def short_uuid(self):
        return gattlib_uuid_to_int(self._gattlib_characteristic.uuid)

    @property
    def connection(self):
        return self._device.connection

    def read(self, callback=None):
        if callback:
            raise RuntimeError("Not supported yet")
        else:
            # TODO: Solve buffer allocation
            ret = gattlib.gattlib_read_char_by_uuid(self.connection, self._gattlib_characteristic.uuid, void * buffer, size_t * buffer_len)

    def write(self, data):
        buffer_type = ctypes.c_char * len(data)
        buffer = data
        buffer_len = len(data)

        ret = gattlib.gattlib_write_char_by_uuid(self.connection, self._gattlib_characteristic.uuid, buffer_type.from_buffer(buffer), buffer_len)

    def register_notification(self, callback, user_data=None):
        gattlib.gattlib_register_notification(self.connection, gattlib_event_handler_type(callback), user_data)

    def notification_start(self):
        ret = gattlib.gattlib_notification_start(self.connection, self._gattlib_characteristic.uuid)

    def notification_stop(self):
        ret = gattlib.gattlib_notification_stop(self.connection, self._gattlib_characteristic.uuid)
