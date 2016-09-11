#include "mima.h"
#include "usbd_conf.h"
#include "usbd_hid.h"

G_FLAG gFlag;
MSG_T gMsg;
__ALIGN_BEGIN static uint8_t outBuf[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] __ALIGN_END;

/* USB handler declaration */
/* Handle for USB Full Speed IP */
extern USBD_HandleTypeDef USBD_Device;


static void msg_rsp_status(void);
static void msg_rsp_permit(void);
static void msg_rsp_set_permit(void);

void mima_init(void) {
    gMsg = MSG_NONE;
    gFlag.s.permit = 0;
    gFlag.s.ver = 1;

    if (*(uint8_t *)PERMIT_BASE != 0xFF) {
        gFlag.s.initpermit = 1;
    }
}

void mima_loop(void) {
    switch (gMsg) {
    case MSG_NONE:
        break;
    case MSG_STATUS:
        msg_rsp_status();
        gMsg = MSG_NONE;
        break;
    case MSG_PERMIT:
        msg_rsp_permit();
        gMsg = MSG_NONE;
        break;
    case MSG_SET_PERMIT:
        msg_rsp_set_permit();
        gMsg = MSG_NONE;
        break;
    default:
        gMsg = MSG_NONE;
        break;
    }
}

void msg_rsp_error(void) {
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = MSG_ERROR;
    USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

static void msg_rsp_status(void) {
    outBuf[0] = 2; //report_id
    outBuf[1] = '@'; // response flag
    outBuf[2] = (uint8_t)~MSG_STATUS;
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
        msg_rsp_error();
    } else {
        gFlag.s.permit = 1;
        outBuf[0] = 2; //report_id
        outBuf[1] = '@'; // response flag
        outBuf[2] = (uint8_t)~MSG_PERMIT;
        USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
    }
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

    HAL_Delay(10000);

    if (PAGEError != 0) {
        msg_rsp_error();
    } else {
        gFlag.s.initpermit = 1;
        outBuf[0] = 2; //report_id
        outBuf[1] = '@'; // response flag
        outBuf[2] = (uint8_t)~MSG_SET_PERMIT;
        USBD_HID_SendReport(&USBD_Device, outBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
    }
}
