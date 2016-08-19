/**
  ******************************************************************************
  * @file    usbd_customhid_if_template.c
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   USB Device Custom HID interface file.
  *		     This template should be copied to the user folder, renamed and customized
  *          following user needs.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid_if.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE]  __ALIGN_END =
{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID(1)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0x85, 0x02,                    //   REPORT_ID(2)
    0x06, 0x01, 0x00,              //   USAGE_PAGE (Generic Desktop)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x3F,                    //   REPORT_COUNT (63)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x3F,                    //   REPORT_COUNT (63)
    0x91, 0x00,                    //   OUTPUT (Data,Var,Abs)
    0xc0,                          // END_COLLECTION
};
/* USB handler declaration */
/* Handle for USB Full Speed IP */
extern USBD_HandleTypeDef USBD_Device;

static int8_t CUSTOM_HID_Init     (void);
static int8_t CUSTOM_HID_DeInit   (void);
static int8_t CUSTOM_HID_OutEvent (uint8_t *pBuf);
/* Private variables ---------------------------------------------------------*/
USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops =
{
  CUSTOM_HID_ReportDesc,
  CUSTOM_HID_Init,
  CUSTOM_HID_DeInit,
  CUSTOM_HID_OutEvent,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CUSTOM_HID_Init
  *         Initializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init(void)
{

  return (0);
}

/**
  * @brief  CUSTOM_HID_DeInit
  *         DeInitializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit(void)
{
  /*
     Add your deinitialization code here
  */
  return (0);
}


/**
  * @brief  CUSTOM_HID_Control
  *         Manage the CUSTOM HID class events
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent  (uint8_t *pBuf)
{
  __ALIGN_BEGIN static uint8_t buf[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] __ALIGN_END;

  memcpy(buf, pBuf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
  buf[0] = 2; // report id
  USBD_HID_SendReport(&USBD_Device, buf, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
  return (0);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
