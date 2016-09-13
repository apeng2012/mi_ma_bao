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
        l = len(chunk)
        if l > 62:
            raise Exception("Unexpected data length")

        self.hid.write([2, ord('#')] + chunk + [0xFF]*(62-l))

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

        return data[2:]


class Msg_T(object):
    """docstring for Msg_T"""
    MSG_NONE   = 0x00
    MSG_STATUS = 0xE1
    MSG_PERMIT = 0xD2
    MSG_SET_PERMIT = 0xC3
    MSG_ADD    = 0xB4
    MSG_ERROR  = 0xFF

        
class MiMaBao(_HidTransport):
    """docstring for MiMaBao"""
    def __init__(self):
    	self.status_bits = 0
        super(MiMaBao, self).__init__()

    def status(self):
        self._open()
        self._write_chunk([Msg_T.MSG_STATUS])
        self.status_bits = self._read_chunk()[1]
        self._close()

    def st_isinitpermit(self):
        return (self.status_bits & 0x01) == 0x01

    def st_ispermit(self):
        return (self.status_bits & 0x02) == 0x02
        
    def permit(self, password):
        self.status()
        if not self.st_isinitpermit():
            return None

        self._open()
        self._write_chunk([Msg_T.MSG_PERMIT] + map(ord, list(password)))
        tmp = self._read_chunk()[0]
        self._close()

        if tmp != (~Msg_T.MSG_PERMIT)&0xFF:
            return False
    	return True

    def set_permit(self, password):
        l = len(password)
        if l<3 or l>60:
            raise Exception("password len error")

        self._open()
        self._write_chunk([Msg_T.MSG_SET_PERMIT] + map(ord, list(password)))
        tmp = self._read_chunk()[0]
        self._close()

        if tmp != (~Msg_T.MSG_SET_PERMIT)&0xFF:
            return False
        return True
        
    def add(self, usedto, name, password):
        lu = len(usedto)
        if lu<3 or lu>20:
            raise Exception("usedto len error")
        ln = len(name)
        if ln<3 or ln>20:
            raise Exception("name len error")
        lp = len(password)
        if lp<3 or lp>20:
            raise Exception("password len error")

        self._open()
        self._write_chunk([Msg_T.MSG_ADD] \
            + map(ord, list(usedto)) + [0xFF]*(20-lu) \
            + map(ord, list(name)) + [0xFF]*(20-ln) \
            + map(ord, list(password)) + [0xFF]*(20-lp) )
        tmp = self._read_chunk()[0]
        self._close()

        if tmp != (~Msg_T.MSG_ADD)&0xFF:
            return False
        return True


if __name__ == '__main__':
    mmb = MiMaBao()
    mmb.status()
    print "status_bits: 0x%02x" % mmb.status_bits

    rep = mmb.permit("apeng")
    if rep == None:
        print "no init password"

        print "set permit..."
        mmb.set_permit("apeng")
    elif rep:
        print "correct password"
    else:
        print "wrong password"


