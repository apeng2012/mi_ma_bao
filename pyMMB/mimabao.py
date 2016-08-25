#coding=utf-8

import hid
import random

DEVICE_IDS = [
    (0x0483, 0x5710)
]

class MiMaBao(object):
    """docstring for MiMaBao"""
    def __init__(self):
    	mmbs = self.enumerate()
    	devices_num = len(mmbs)
    	if devices_num == 0:
    		raise Exception("Not find MMB device.")
    	elif devices_num != 1:
    		raise Exception("Please insert one MMB device.")
        
        self.device = mmbs[0]


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


    def permit(self, password):
    	return True


    def ispermit(self):
    	pass
        

if __name__ == '__main__':
    mmb = MiMaBao()
