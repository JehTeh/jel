/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define DC_MOT_PB_Pin GPIO_PIN_13
#define DC_MOT_PB_GPIO_Port GPIOC
#define ZERO_CROSS_Pin GPIO_PIN_14
#define ZERO_CROSS_GPIO_Port GPIOC
#define ZERO_CROSS_EXTI_IRQn EXTI15_10_IRQn
#define MOT_3_PS_Pin GPIO_PIN_15
#define MOT_3_PS_GPIO_Port GPIOC
#define UL_Pin GPIO_PIN_7
#define UL_GPIO_Port GPIOA
#define VL_Pin GPIO_PIN_0
#define VL_GPIO_Port GPIOB
#define WL_Pin GPIO_PIN_1
#define WL_GPIO_Port GPIOB
#define HALL_C_Pin GPIO_PIN_10
#define HALL_C_GPIO_Port GPIOB
#define HALL_C_EXTI_IRQn EXTI15_10_IRQn
#define DC_MOT_DIR_Pin GPIO_PIN_11
#define DC_MOT_DIR_GPIO_Port GPIOB
#define DC_MOT_INHIBIT_Pin GPIO_PIN_12
#define DC_MOT_INHIBIT_GPIO_Port GPIOB
#define SLAVE_MCU_RESET_Pin GPIO_PIN_13
#define SLAVE_MCU_RESET_GPIO_Port GPIOB
#define PMSM_BLDC_Pin GPIO_PIN_6
#define PMSM_BLDC_GPIO_Port GPIOC
#define LIGHT_CONTROL_Pin GPIO_PIN_7
#define LIGHT_CONTROL_GPIO_Port GPIOC
#define MOT3_4_Pin GPIO_PIN_8
#define MOT3_4_GPIO_Port GPIOC
#define MOT_4_PS_Pin GPIO_PIN_9
#define MOT_4_PS_GPIO_Port GPIOC
#define UH_Pin GPIO_PIN_8
#define UH_GPIO_Port GPIOA
#define VH_Pin GPIO_PIN_9
#define VH_GPIO_Port GPIOA
#define WH_Pin GPIO_PIN_10
#define WH_GPIO_Port GPIOA
#define GATE_KILL_Pin GPIO_PIN_11
#define GATE_KILL_GPIO_Port GPIOA
#define GATE_KILL_EXTI_IRQn EXTI15_10_IRQn
#define PIEZO_RELAY_Pin GPIO_PIN_12
#define PIEZO_RELAY_GPIO_Port GPIOA
#define HALL_A_Pin GPIO_PIN_15
#define HALL_A_GPIO_Port GPIOA
#define HALL_A_EXTI_IRQn EXTI15_10_IRQn
#define REVERSAL_Pin GPIO_PIN_12
#define REVERSAL_GPIO_Port GPIOC
#define PRESETS_Pin GPIO_PIN_2
#define PRESETS_GPIO_Port GPIOD
#define HALL_B_Pin GPIO_PIN_3
#define HALL_B_GPIO_Port GPIOB
#define HALL_B_EXTI_IRQn EXTI3_IRQn
#define PUMP_PB_Pin GPIO_PIN_6
#define PUMP_PB_GPIO_Port GPIOB
#define MOT_2_PS_Pin GPIO_PIN_8
#define MOT_2_PS_GPIO_Port GPIOB
#define MOT_1_PS_Pin GPIO_PIN_9
#define MOT_1_PS_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
 #define USE_FULL_ASSERT    1U 

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
