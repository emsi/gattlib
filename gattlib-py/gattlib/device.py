import logging

from gattlib import *
from .gatt import GattService, GattCharacteristic

BDADDR_LE_PUBLIC = 0x1
BDADDR_LE_RANDOM = 0x02

BT_SEC_SDP = 0,
BT_SEC_LOW = 1
BT_SEC_MEDIUM = 2
BT_SEC_HIGH = 3


class Device:

    def __init__(self, adapter, addr, name=None):
        self._adapter = adapter
        self._addr = addr
        self._name = name
        self._connection = c_void_p(None)

    @property
    def id(self):
        return self._addr

    @property
    def connection(self):
        return self._connection

    def connect(self):
        adapter_name = self._adapter.name

        self._connection = gattlib.gattlib_connect(adapter_name, self._addr, BDADDR_LE_PUBLIC, BT_SEC_LOW, 0, 0)

    def disconnect(self):
        gattlib.gattlib_disconnect(self._connection)

    def discover(self):
        #
        # Discover GATT Services
        #
        _services = POINTER(GattlibPrimaryService)()
        _services_count = c_int(0)
        ret = gattlib_discover_primary(self._connection, byref(_services), byref(_services_count))

        self._services = {}
        for i in range(0, _services_count.value):
            service = GattService(self, _services[i])
            self._services[service.short_uuid] = service

            logging.debug("Service UUID:0x%x" % service.short_uuid)

        #
        # Discover GATT Characteristics
        #
        _characteristics = POINTER(GattlibCharacteristic)()
        _characteristics_count = c_int(0)
        ret = gattlib_discover_char(self._connection, byref(_characteristics), byref(_characteristics_count))

        self._characteristics = {}
        for i in range(0, _characteristics_count.value):
            characteristic = GattCharacteristic(self, _characteristics[i])
            self._characteristics[characteristic.short_uuid] = characteristic

            logging.debug("Characteristic UUID:0x%x" % characteristic.short_uuid)

        return ret

    @property
    def services(self):
        if not hasattr(self, '_services'):
            logging.warning("Start GATT discovery implicitly")
            self.discover()

        return self._services

    @property
    def characteristics(self):
        if not hasattr(self, '_characteristics'):
            logging.warning("Start GATT discovery implicitly")
            self.discover()

        return self._characteristics

    def __str__(self):
        name = self._name
        if name:
            return str(name)
        else:
            return str(self._addr)
