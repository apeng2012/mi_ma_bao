#ifndef __USBKB_H__
#define __USBKB_H__

#include "stm32f1xx_hal.h"

void usbKB_init(void);
void USB_KB_type(const char *str, uint8_t len);

#endif
