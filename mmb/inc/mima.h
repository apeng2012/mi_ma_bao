#ifndef __MIMA_H__
#define __MIMA_H__

#include "stm32f1xx_hal.h"

typedef union {
    uint8_t b;
    struct {
        uint8_t initpermit:1;
        uint8_t permit:1;
    }s;
} G_FLAG;

typedef enum {
    MSG_NONE   = 0x00,
    MSG_STATUS = 0xE1,
    MSG_PERMIT = 0xD2,
    MSG_ERROR  = 0xFF,
} MSG_T;

#define MIMA_BASE (FLASH_BASE + 0x10000)
#define PERMIT_BASE (MIMA_BASE - 1024)

void mima_init(void);
void mima_loop(void);
void msg_rsp_error(void);

#endif // __MIMA_H__
