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

    def _close(self):
        self.hid.close()
        self.hid = None

    def _write_chunk(self, chunk):
        l = len(chunk)
        if l > 62:
            raise Exception("Unexpected data length")

        data = [2, ord('#')] + chunk + [0xFF]*(62-l)
        #print 'write:'
        #print data
        self.hid.write(data)

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

        #print 'read:'
        #print data
        return data[2:]


class Msg_T(object):
    """docstring for Msg_T"""
    MSG_NONE   = 0x00
    MSG_STATUS = 0xE1
    MSG_PERMIT = 0xD2
    MSG_SET_PERMIT = 0xC3
    MSG_ADD    = 0xB4
    MSG_GET_USEDTO = 0xA5
    MSG_GET_ITEM = 0x69
    MSG_MMB    = 0x78
    MSG_PASSWORD = 0xF0

def check_permit(fn):
    def wrapper(*args, **kw):
        args[0].status()
        if args[0].st_ispermit():
            return fn(*args, **kw)
        else:
            raise Exception("not permit.")
    return wrapper
        
class MiMaBao(_HidTransport):
    """docstring for MiMaBao"""
    def __init__(self):
        self.status_bits = 0
        super(MiMaBao, self).__init__()

    def cmd(self, cmd):
        self._open()
        self._write_chunk(cmd)
        rep = self._read_chunk()
        self._close()

        if rep[0] != (~cmd[0])&0xFF:
            raise Exception("cmd respond error")
        return rep[1:]

    def status(self):
        self.status_bits = self.cmd([Msg_T.MSG_STATUS])[0]

    def st_isinitpermit(self):
        return (self.status_bits & 0x01) == 0x01

    def st_ispermit(self):
        return (self.status_bits & 0x02) == 0x02

    def permit(self, password):
        """ 校验密码
         如果初次使用没有设置密码，返回None
         密码正确返回True，否则False
         密码宝每次上电，只需输入一次密码，如果校验正确，将一直保持操作许可，直到拔出。
         如果校验错误，将删除密码宝上一切内容。
        """
        self.status()
        if not self.st_isinitpermit():
            return None

        tmp = self.cmd([Msg_T.MSG_PERMIT] + map(ord, list(password)))
        return tmp[0] == 1

    def set_permit(self, password):
        '''设置密码'''
        l = len(password)
        if l<3 or l>60:
            raise Exception("password len error")

        self.cmd([Msg_T.MSG_SET_PERMIT] + map(ord, list(password)))

    @check_permit
    def add(self, usedto, name, password):
        '''增加一条密码项'''
        lu = len(usedto)
        if lu<3 or lu>20:
            raise Exception("usedto len error")
        ln = len(name)
        if ln<3 or ln>20:
            raise Exception("name len error")
        lp = len(password)
        if lp<3 or lp>20:
            raise Exception("password len error")

        tmp = self.cmd([Msg_T.MSG_ADD] \
            + map(ord, list(usedto)) + [0xFF]*(20-lu) \
            + map(ord, list(name)) + [0xFF]*(20-ln) \
            + map(ord, list(password)) + [0xFF]*(20-lp) )

        return tmp[0] == 1

    @check_permit
    def get_usedto(self, usedto):
        '''根据输入字符串，获取匹配的密码项
        函数返回匹配项的数量
        迭代调用get_item，返回具体 usedto 字符串 '''
        lu = len(usedto)
        if lu<1 or lu>20:
            raise Exception("usedto len error")

        tmp = self.cmd([Msg_T.MSG_GET_USEDTO] + map(ord, list(usedto)))

        return tmp[0] + tmp[1]*256

    def get_item(self):
        '''获取usedto迭代项。需要先调用get_usedto'''
        tmp = self.cmd([Msg_T.MSG_GET_ITEM])
        it = filter(lambda x: x!=0xFF, tmp)
        return reduce(lambda x,y:x + chr(y), it, '')

    @check_permit
    def prepare_password(self, usedto):
        '''准备密码，与MMB配对使用'''
        lu = len(usedto)
        if lu<1 or lu>20:
            raise Exception("usedto len error")

        tmp = self.cmd([Msg_T.MSG_PASSWORD] + map(ord, list(usedto)))
        name = filter(lambda x: x!=0xFF, tmp)
        return reduce(lambda x,y:x + chr(y), name, '')

    @check_permit
    def MMB(self):
        '''获取当前密码，与prepare_password配对使用'''
        tmp = self.cmd([Msg_T.MSG_MMB])


if __name__ == '__main__':
    mmb = MiMaBao()

    # test read status bits
    mmb.status()
    print "status_bits: 0x%02x" % mmb.status_bits

    rep = mmb.permit("apeng")
    if rep == None:
        print "no init password"

        print "set permit..."
        mmb.set_permit("apeng")
        mmb.permit("apeng")
    elif rep:
        print "correct password"
    else:
        print "wrong password"
        exit()

    if mmb.add("usedto", "name", "password"):
        print "add one item"
    else:
        print "add cmd error"

    cnt = mmb.get_usedto('u')
    print cnt

    for x in range(0,cnt):
        print mmb.get_item()

    print mmb.prepare_password("usedto")
