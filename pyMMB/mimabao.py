#coding=utf-8

import hid
import random
import time

DEVICE_IDS = [
    (0x0483, 0x5710)
]

class NotImplementedException(Exception):
    pass

class ConnectionError(Exception):
    pass


class _HidTransport(object):
    def __init__(self):
        self.hid = None

        mmbs = self.enumerate()
        devices_num = len(mmbs)
        if devices_num == 0:
            raise Exception("Not find MMB device.")
        elif devices_num != 1:
            raise Exception("Please insert one MMB device.")
        
        self.device = mmbs[0]

        super(_HidTransport, self).__init__()

    def enumerate(self):
        """ return a list of MMB devices"""
        devices = {}
        for d in hid.enumerate(0, 0):
            vendor_id = d['vendor_id']
            product_id = d['product_id']
            serial_number = d['serial_number']
            interface_number = d['interface_number']
            path = d['path']

            if (vendor_id, product_id) in DEVICE_IDS:
                devices[serial_number] = path

        return list(devices.values())

    def is_connected(self):
        """
        Check if the device is still connected.
        """
        for d in hid.enumerate(0, 0):
            if d['path'] == self.device:
                return True
        return False

    def _open(self):
        self.hid = hid.device()
        self.hid.open_path(self.device)
        self.hid.set_nonblocking(True)

        # determine MMB
        r = self.hid.write([0x2, ord('#')] + [0xFF] * 62)
        if r == 64:
            time.sleep(0.1)
            data = self.hid.read(64)
            if data[1] != ord('@'):
                raise ConnectionError("Unknown HID version")        
            return
        raise ConnectionError("Unknown HID version")

    def _close(self):
        self.hid.close()
        self.hid = None

    def _write_chunk(self, chunk):
        if len(chunk) != 62:
            raise Exception("Unexpected data length")

        self.hid.write([2, ord('#')] + chunk)

    def _read_chunk(self):
        start = time.time()

        while True:
            data = self.hid.read(64)
            if not len(data):
                if time.time() - start > 10:
                    # Over 10 s of no response, let's check if
                    # device is still alive
                    if not self.is_connected():
                        raise ConnectionError("Connection failed")

                    # Restart timer
                    start = time.time()

                time.sleep(0.001)
                continue

            break

        if len(data) != 64:
            raise Exception("Unexpected chunk size: %d" % len(data))

        return bytearray(data[2:])


class MiMaBao(_HidTransport):
    """docstring for MiMaBao"""
    def __init__(self):
    	self.status_bits = 0
        super(MiMaBao, self).__init__()

    def status(self):
        self._open()
        self._write_chunk([ord('s'), ord('t')] + [0]*60)
        self.status_bits = self._read_chunk()[0]
        self._close()
        
    def permit(self, password):
    	return True

    def ispermit(self):
    	pass
        

if __name__ == '__main__':
    mmb = MiMaBao()
    mmb.status()
    print mmb.status_bits
