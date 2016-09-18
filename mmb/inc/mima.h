#ifndef __MIMA_H__
#define __MIMA_H__

#include "stm32f1xx_hal.h"

#define MIMA_NUM (512)
#define MIMA_PER_SIZE (64)
#define MIMA_BASE (FLASH_BASE + 0x10000)
#define MIMA_END (MIMA_BASE + 0x10000 - 1)
#define PERMIT_BASE (MIMA_BASE - 1024)

typedef union {
    uint8_t b;
    struct {
        uint8_t initpermit:1;
        uint8_t permit:1;
        uint8_t mmb:1;
        uint8_t ver:5;
    }s;
} G_FLAG;

typedef enum {
    MSG_NONE   = 0x00,
    MSG_STATUS = 0xE1,
    MSG_PERMIT = 0xD2,
    MSG_SET_PERMIT = 0xC3,
    MSG_ADD    = 0xB4,
    MSG_GET_USEDTO = 0xA5,
    MSG_GET_ITEM = 0x69,
    MSG_MMB    = 0x78,
    MSG_PASSWORD = 0xF0,
} MSG_T;

typedef struct {
    uint8_t usedto[20];
    uint8_t name[20];
    uint8_t password[20];
    uint32_t tmp;
}MiMa_T;

#define mima_table ((MiMa_T *) MIMA_BASE)


void mima_init(void);
void mima_loop(void);

#endif // __MIMA_H__
