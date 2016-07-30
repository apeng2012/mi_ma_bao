#include "usbKB.h"


TIM_HandleTypeDef TimHandle;

void usbKB_init(void) {
    TimHandle.Instance = TIM2;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TimHandle.Init.Period = 1000 - 1;
    TimHandle.Init.Prescaler = 72000 - 1;
    TimHandle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&TimHandle);

	HAL_TIM_Base_Start_IT(&TimHandle);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
}
