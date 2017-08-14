#include "mima.h"
#include "usbd_conf.h"
#include "usbd_hid.h"
#include "SEGGER_RTT.h"
#include "usbKB.h"

static char acLogBuf[BUFFER_SIZE_UP];

G_FLAG gFlag;
MSG_T gMsg;
__ALIGN_BEGIN static uint8_t outBuf[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] __ALIGN_END;

static uint8_t usedto_search_table[MIMA_NUM];
static uint16_t usedto_item_index;

/* USB handler declaration */
/* Handle for USB Full Speed IP */
extern USBD_HandleTypeDef USBD_Device;


static void msg_rsp_status(void);
static void msg_rsp_permit(void);
static void msg_rsp_set_permit(void);
static void msg_rsp_add(void);
static void msg_rsp_get_usedto(void);
static void msg_rsp_get_item(void);
static void msg_rsp_password(void);
static void msg_rsp_MMB(void);
static uint8_t * my_strstr(uint8_t * str1, uint8_t * str2);
static uint8_t my_strcmp(uint8_t *str1, uint8_t *str2);


void mima_init(void) {
    SEGGER_RTT_ConfigUpBuffer(1, NULL, acLogBuf, sizeof(acLogBuf), SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    SEGGER_RTT_WriteString(1, "log start!\r\n");

    gMsg = MSG_NONE;
    gFlag.s.permit = 0;
    gFlag.s.mmb = 0;
    gFlag.s.ver = 1;
    usedto_item_index = 0;

    if (*(uint8_t *)PERMIT_BASE != 0xFF) {
        gFlag.s.initpermit = 1;
    }
}

uint8_t g_test = 0;
void mima_loop(void) {
    switch (gMsg) {
    case MSG_NONE:
        if (g_test != 0) {
            g_test = 0;
            USB_KB_type("Hello", 11);
        }
        break;
    case MSG_STATUS:
        g_test = 1;
        SEGGER_RTT_WriteString(0, "MSG_STATUS\r\n");
        msg_rsp_status();
        gMsg = MSG_NONE;
        break;
    case MSG_PERMIT:
        SEGGER_RTT_WriteString(0, "MSG_PERMIT\r\n");
        msg_rsp_permit();
        gMsg = MSG_NONE;
        break;
    case MSG_SET_PERMIT:
        SEGGER_RTT_WriteString(0, "MSG_SET_PERMIT\r\n");
        msg_rsp_set_permit();
        gMsg = MSG_NONE;
        break;
    case MSG_ADD:
        SEGGER_RTT_WriteString(0, "MSG_ADD\r\n");
        msg_rsp_add();
        gMsg = MSG_NONE;
        break;
    case MSG_GET_USEDTO:
        SEGGER_RTT_WriteString(0, "MSG_GET_USEDTO\r\n");
        msg_rsp_get_usedto();
        gMsg = MSG_NONE;
        break;
    case MSG_GET_ITEM:
        SEGGER_RTT_WriteString(0, "MSG_GET_ITEM\r\n");
        msg_rsp_get_item();
        gMsg = MSG_NONE;
        break;
    case MSG_PASSWORD:
        SEGGER_RTT_WriteString(0, "MSG_PASSWORD\r\n");
        msg_rsp_password();
        gMsg = MSG_NONE;
        break;
    case MSG_MMB:
        SEGGER_RTT_WriteString(0, "MSG_MMB\r\n");
        msg_rsp_MMB();
        gMsg = MSG_NONE;
        break;
    default:
        gMsg = MSG_NONE;
        break;
    }
}

static void msg_rsp_status(void) {
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_STATUS;
    outBuf[3] = gFlag.b;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_permit(void) {
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    uint8_t *pSrc = (uint8_t *)PERMIT_BASE;
    uint8_t *pDst = &(((USBD_HID_HandleTypeDef*)(USBD_Device.pClassData))->Report_buf[3]);
    uint8_t tmp;

    while (1) {
        tmp = *pSrc;
        if (*pSrc++ != *pDst++) {
            HAL_FLASH_Unlock();
            EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
            EraseInitStruct.PageAddress = PERMIT_BASE;
            EraseInitStruct.NbPages     = 33;
            HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
            HAL_FLASH_Lock();
            tmp = 0;
            break;
        }
        if (tmp == 0xFF) {
            break;
        }
    }

    HAL_Delay(10000);

    if (tmp == 0) {
        gFlag.s.initpermit = 0;
        gFlag.s.permit = 0;
    } else {
        gFlag.s.permit = 1;
        tmp = 1;
    }

    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_PERMIT;
    outBuf[3] = tmp;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_set_permit(void) {
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    uint32_t *pSrc = (uint32_t *)(&(((USBD_HID_HandleTypeDef*)(USBD_Device.pClassData))->Report_buf[3]));
    uint8_t i;
    uint32_t addr;
    uint32_t tmp;

    HAL_FLASH_Unlock();
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = PERMIT_BASE;
    EraseInitStruct.NbPages     = 33;
    HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

    addr = PERMIT_BASE;
    PAGEError = 0;
    for (i=0; i<64; i+=4) {
        PAGEError += HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *pSrc);
        addr += 4;
        tmp = *pSrc++;
        tmp >>= 24;
        if (tmp == 0xFF) {
            break;
        }
    }
    HAL_FLASH_Lock();

    if (PAGEError != 0) {
        i=0;
    } else {
        gFlag.s.initpermit = 1;
        i=1;
    }
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_SET_PERMIT;
    outBuf[3] = i;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_add(void) {
    uint32_t *pSrc = (uint32_t *)(&(((USBD_HID_HandleTypeDef*)(USBD_Device.pClassData))->Report_buf[3]));
    uint32_t PAGEError = 0;
    uint8_t i;
    uint32_t addr;

    if (gFlag.s.permit == 0) {
        PAGEError = 0xFF;
        goto MSG_RSP_ADD_END;
    }

    for (addr=MIMA_BASE; addr<MIMA_END; addr+=MIMA_PER_SIZE) {
        if ((*(uint8_t*)addr) == 0xFF) {
            break;
        }
    }
    if (addr > MIMA_END) {
        PAGEError = 0xFF;
        goto MSG_RSP_ADD_END;
    }
    HAL_FLASH_Unlock();
    PAGEError = 0;
    for (i=0; i<60; i+=4) {
        PAGEError += HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *pSrc++);
        addr += 4;
    }
    HAL_FLASH_Lock();

MSG_RSP_ADD_END:
    if (PAGEError != 0) {
        i = 0;
    } else {
        i = 1;
    }
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_ADD;
    outBuf[3] = i;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_get_usedto(void) {
    uint8_t *pSrc = &(((USBD_HID_HandleTypeDef*)(USBD_Device.pClassData))->Report_buf[3]);
    uint16_t ret = 0;
    uint16_t i;
    uint8_t *p;

    if (gFlag.s.permit == 0) {
        goto MSG_RSP_GET_USEDTO_END;
    }

    for (i=0; i<MIMA_NUM; i++) {
        if (mima_table[i].usedto[0] == 0xFF) {
            break;
        }
        p = my_strstr(mima_table[i].usedto, pSrc);
        if (p == 0) {
            usedto_search_table[i] = 0;
            continue;
        } else if (p == mima_table[i].usedto) {
            ret++;
            usedto_search_table[i] = 2;
        } else {
            ret++;
            usedto_search_table[i] = 1;
        }
    }

MSG_RSP_GET_USEDTO_END:
    usedto_item_index = 0;
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_GET_USEDTO;
    outBuf[3] = ret&0xFF;
    outBuf[4] = ret>>8;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_get_item(void) {
    uint8_t i;
    uint8_t *p;

    for (i=0; i<USBD_CUSTOMHID_OUTREPORT_BUF_SIZE; i++) {
        outBuf[i] = 0xFF;
    }

    if (gFlag.s.permit == 0) {
        goto MSG_RSP_GET_ITEM_END;
    }

    while(usedto_item_index<MIMA_NUM) {
        if (usedto_search_table[usedto_item_index] != 0) {
            p = &outBuf[3];
            for (i=0; i<20; i++) {
                *p++ = mima_table[usedto_item_index].usedto[i];
            }
            usedto_item_index++;
            break;
        }
        usedto_item_index++;
    }


MSG_RSP_GET_ITEM_END:
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_GET_ITEM;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_password(void) {
    uint8_t *pSrc = &(((USBD_HID_HandleTypeDef*)(USBD_Device.pClassData))->Report_buf[3]);
    uint16_t i;
    uint8_t j;
    uint8_t *p, *pOut;

    for (j=0; j<USBD_CUSTOMHID_OUTREPORT_BUF_SIZE; j++) {
        outBuf[j] = 0xFF;
    }

    if (gFlag.s.permit == 0) {
        goto MSG_RSP_PASSWORD_END;
    }

    for (i=0; i<MIMA_NUM; i++) {
        if (mima_table[i].usedto[0] == 0xFF) {
            break;
        }
        if (my_strcmp(pSrc, mima_table[i].usedto) == 1) {
            p = (uint8_t*)&mima_table[i];
            pOut = &outBuf[3];
            for (j=0; j<60; j++) {
                *pOut++ = *p++;
            }
            gFlag.s.mmb = 1;
            break;
        }
    }

MSG_RSP_PASSWORD_END:
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_PASSWORD;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_MMB(void) {
    MiMa_T *pmm;
    uint8_t buf[21];
    uint8_t i=0;

    if ((gFlag.s.permit == 0) || (gFlag.s.mmb != 1)) {
        goto MSG_RSP_MMB_END;
    }

    pmm = (MiMa_T*)&outBuf[3];
    for (i=0; i<20; i++) {
        if (pmm->password[i] == 0xFF) {
            buf[i] = 0;
            break;
        } else {
            buf[i] = pmm->password[i];
        }
    }

MSG_RSP_MMB_END:
    gFlag.s.mmb = 0;
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = ~(uint8_t)MSG_MMB;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

    if (i != 0) {
        HAL_Delay(10000);
        SEGGER_RTT_printf(0, "MMB password len %d " , i);
        SEGGER_RTT_printf(0, "%s \r\n" , buf);
        USB_KB_type((char*)buf, i);
    }
}

static uint8_t * my_strstr(uint8_t * str1, uint8_t * str2) {
    uint8_t *cp = str1;
    uint8_t *s1, *s2;
    uint8_t i = 0;

    if (*str2 == 0xFF) {
        return NULL;
    }

    while(*cp != 0xFF) {
        s1 = cp;
        s2 = str2;

        while((*s1 != 0xFF) && (*s2 != 0xFF) && (*s1 == *s2)) {
            s1++;
            s2++;
        }

        if (*s2 == 0xFF) {
            return cp;
        }
        cp++;

        i++;
        if (i>=20) {
            return NULL;
        }
    }

    return NULL;
}

static uint8_t my_strcmp(uint8_t *str1, uint8_t *str2) {
    do {
        if (*str1++ == *str2++) {
            if (*str1 == 0xFF) {
                return 1;
            }
            continue;
        } else {
            return 0;
        }
    }while (1);
}
