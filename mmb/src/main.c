/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include "stm32f1xx_hal.h"

void led_gpio_init(void) {
    GPIO_InitTypeDef        GPIO_InitStructure;

    __GPIOB_CLK_ENABLE();

    GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
}


int main(void)
{
    HAL_Init();

    led_gpio_init();

    while(1) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
        HAL_Delay(1000);
    }
}
