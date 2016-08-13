#include "usbKB.h"
#include "usbd_hid.h"
#include "keycode.h"

extern USBD_HandleTypeDef USBD_Device;
TIM_HandleTypeDef TimHandle;

static uint8_t KB_USBBuf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static char KB_strBuf[40];
static char *pKB_str = NULL;
static uint8_t len_KB_str = 0;

static enum KB_STATE {
	BTN_Down,
	BTN_Up,
}KB_state;

void usbKB_init(void) {
    TimHandle.Instance = TIM2;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TimHandle.Init.Period = 1000 - 1;
    TimHandle.Init.Prescaler = 72000 - 1;
    TimHandle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&TimHandle);
}

static void char2KBID(char ch) {
	memset(KB_USBBuf, 0, 9);
	KB_USBBuf[0] = 1; // report id
	KB_USBBuf[3] = 0x2c; // space

	if ((ch>='a') && (ch<='z')) {
		KB_USBBuf[3] = ch - 'a' + 4;
	} else if ((ch>='A') && (ch<='Z')) {
		KB_USBBuf[3] = ch - 'A' + 4;
		KB_USBBuf[1] = 0x02;
	} else if ((ch>='1') && (ch <='9')) {
		KB_USBBuf[3] = ch - '1' + 0x1E;
	} else if (ch == '0') {
		KB_USBBuf[3] = KC_0;
    } else if (ch == '~') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_NONUS_HASH;
    } else if (ch == '!') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_1;
    } else if (ch == '@') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_2;
    } else if (ch == '#') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_3;
    } else if (ch == '$') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_4;
    } else if (ch == '%') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_5;
    } else if (ch == '^') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_6;
    } else if (ch == '&') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_7;
    } else if (ch == '*') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_8;
     } else if (ch == '(') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_9;
    } else if (ch == ')') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_0;
    } else if (ch == '-') {
		KB_USBBuf[3] = KC_MINUS;
    } else if (ch == '+') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = KC_EQUAL;
	} else if (ch == ':') {
		KB_USBBuf[1] = 0x02;//left shift
		KB_USBBuf[3] = 0x33;
	} else if (ch == '/') {
		KB_USBBuf[3] = 0x38;///
	} else if (ch == '.') {
		KB_USBBuf[3] = 0x37;//.
	} else if (ch == '\r') {
		KB_USBBuf[3] = 0x28;//Return
	} else if (ch == '\1') {
		KB_USBBuf[1] = 0x08; // Windows
		KB_USBBuf[3] = 0x15; // r
	}
}

void USB_KB_type(const char *str, uint8_t len) {
	if (pKB_str != NULL) {
		return;
	}
	if (len == 0) {
        return;
	}

	strcpy(KB_strBuf, str);
	KB_state = BTN_Up;
	pKB_str = KB_strBuf;

	char2KBID(*pKB_str++);
	len_KB_str = len - 1;
	USBD_HID_SendReport(&USBD_Device, KB_USBBuf, HID_KB_EPIN_SIZE);

	__HAL_TIM_SET_COUNTER(&TimHandle, 0);
	__HAL_TIM_SET_AUTORELOAD(&TimHandle, 50-1); // 50ms
	HAL_TIM_Base_Start_IT(&TimHandle);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    __HAL_TIM_SET_COUNTER(htim, 0);

    switch (KB_state) {
    case BTN_Down:
        if ((len_KB_str == 0) || (pKB_str == NULL)) {
            HAL_TIM_Base_Stop_IT(htim);
            return;
        }
        KB_state = BTN_Up;

        char2KBID(*pKB_str);
        pKB_str++;
        len_KB_str--;
        USBD_HID_SendReport(&USBD_Device, KB_USBBuf, HID_KB_EPIN_SIZE);

        __HAL_TIM_SET_COUNTER(&TimHandle, 0);
        __HAL_TIM_SET_AUTORELOAD(&TimHandle, 50-1); // 50ms
        HAL_TIM_Base_Start_IT(&TimHandle);
        break;

    case BTN_Up:
        memset(KB_USBBuf, 0, 9);
        KB_USBBuf[0] = 1;
        USBD_HID_SendReport(&USBD_Device, KB_USBBuf, HID_KB_EPIN_SIZE);
        if (len_KB_str == 0) {
            pKB_str = NULL;
            HAL_TIM_Base_Stop_IT(htim);
        } else {
            KB_state = BTN_Down;
            __HAL_TIM_SET_COUNTER(&TimHandle, 0);
            __HAL_TIM_SET_AUTORELOAD(&TimHandle, 200-1); // 200ms
            HAL_TIM_Base_Start_IT(&TimHandle);
        }
        break;
    default:
        pKB_str = NULL;
        HAL_TIM_Base_Stop_IT(htim);
        break;
    }
}
