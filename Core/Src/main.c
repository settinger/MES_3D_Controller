/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "stm32f429i_discovery_gyroscope.h"

#include "app.h"
#include "button.h"
#include "console.h"
#include "touchscreen.h"
#include "accel.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FRAME_DELAY      20      // Time to wait (in milliseconds) between updating frames
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint32_t button0_counter = 0; // How many frames the button has been held for
uint32_t appStart;          // When (in ms since boot) the drawing app started
uint32_t lastFrameTick = 0;             // Counter to track when to update frame
uint32_t lastSecondTick = 0;   // Counter to track when to update time indicator
uint32_t nextTick = 0;                  // Counter for measuring time
TS_StateTypeDef TS_State;               // Touchscreen struct
structAppState appState;                   // State of game
uint16_t touchStateTransition = 0;    // Counter for touch state event detection
void (*checkTouch)(void);               // Function pointer for touch states

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static structAppState handleTouchBegin(void);
static void handleTouchEnd(void);
static void clearIdle(void);
static void touchMaybe(void);
static void touchIdle(void);
static void clearMaybe(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
 * Nonblocking(?) Touch detection with debounce based on WaitForPressedState() method
 */
// Run this when a touch has been confirmed to be occurring
static structAppState handleTouchBegin(void) {
  int16_t x = TS_State.X;
  int16_t y = TS_State.Y;
  y = 320 - y;
  char string[60];
  sprintf(string, "Touch X coordinate: %d\r\nTouch Y coordinate: %d", x, y);
  ConsoleSendLine(string);
  return APP_NORMAL;
}

// Run this when a touch has been confirmed to have ended
static void handleTouchEnd(void) {
}

// Run this function to determine how to parse a touch while in CLEAR_IDLE state (i.e. no touch is occurring)
// If a touch is detected, enter TOUCH_MAYBE state
static void clearIdle(void) {
  if (TS_State.TouchDetected) {
    checkTouch = &touchMaybe;
    touchStateTransition = 1;
  }
}

// Run this function to determine how to parse a touch while in TOUCH_MAYBE state (i.e. a touch might be occurring)
// If four consecutive frames register touches (60 ms), enter TOUCH_IDLE state
static void touchMaybe(void) {
  if (TS_State.TouchDetected) {
    touchStateTransition++;
    if (4 == touchStateTransition) {
      checkTouch = &touchIdle;
      touchStateTransition = 0;
      appState = handleTouchBegin();
    }
  } else {
    checkTouch = &clearIdle;
    touchStateTransition = 0;
  }
}

// Run this function to determine how to parse a touch while in TOUCH_IDLE state (i.e. finger is currently touching screen)
// If no touch is detected, enter CLEAR_MAYBE state
static void touchIdle() {
  if (!TS_State.TouchDetected) {
    checkTouch = &clearMaybe;
    touchStateTransition = 1;
  }
}

// Run this function to determine how to parse a touch while in CLEAR_MAYBE state (i.e. a touch might have ended)
// If four consecutive frames register no touch (60 ms), enter CLEAR_IDLE state
static void clearMaybe() {
  if (TS_State.TouchDetected) {
    checkTouch = &touchIdle;
    touchStateTransition = 0;
  } else {
    touchStateTransition++;
    if (4 == touchStateTransition) {
      checkTouch = &clearIdle;
      touchStateTransition = 0;
      handleTouchEnd();
    }
  }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CRC_Init();
  MX_FMC_Init();
  MX_I2C3_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_UART5_Init();
  MX_TIM7_Init();
  MX_DMA2D_Init();
  MX_LTDC_Init();
  MX_RNG_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  appState = SYSTEM_BOOT;

  ConsoleInit();

  // If using EEPROM, do that here

  // Enable LCD and touchscreen, don't turn them on yet
  BSP_LCD_Init();
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

  // Make sure "USE_STM32F429I_DISCOVERY_REVD" is defined!
  // Enable gyroscope
  // Currently: full scale is +/- 500 DPS
  // Each digit is nominally 17.50 millidegrees per second
  // I *think* it's got a nominal sampling rate of 100 Hz?
  // Results are 16 bit
  if (GYRO_OK == BSP_GYRO_Init()) {
    ConsoleSendLine("Gyro initialized.");
  } else {
    ConsoleSendLine("ERROR! Unable to initialize gyro.");
  }

  // Enable accelerometer
  // LIS2DH connected to SPI5
  // PF7 - SCK
  // PF8 - Sensor data out
  // PF9 - Sensor data in
  // PF6 - Chip select
  accel_init();

  // Set up double-tap on accelerometer
  accel_tap_init();

  // Enable sensor-fusion somethingorother

  // Enable USB HID operations

  // Set initial state
  lastFrameTick = HAL_GetTick();
  lastSecondTick = lastFrameTick;
  appStart = lastSecondTick;
  appState = APP_INIT;
  appInit();
  checkTouch = &clearIdle;
  appState = APP_NORMAL;
  float gyro[3] = { 0 };

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    nextTick = HAL_GetTick();

    // If two seconds have elapsed, update gyro
    if ((APP_NORMAL == appState) && ((nextTick - lastSecondTick) > 2000)) {
      // Update clock time
      BSP_GYRO_GetXYZ(gyro);
      // Todo: figure out what XYZ correspond to
      accel_getValues();
      char gyrotext[200];
      sprintf(gyrotext, "Gyro [DPS]:%4.2f\t%4.2f\t%4.2f",
          //gyro[0], gyro[1], gyro[2]);
          gyro[0]*0.001, gyro[1]*0.001, gyro[2]*0.001);
      ConsoleSendLine(gyrotext);
      lastSecondTick += 2000;
    }

    // Idle so screen is drawn at (at most) 50 FPS
    if ((nextTick - lastFrameTick) > FRAME_DELAY) {
      lastFrameTick = nextTick;

      // If app is in transition state (APP_INIT),
      // do things needed to move to next state
      if (APP_INIT == appState) {
        // TODO: things
        appState = APP_NORMAL;
      }

      // If app is in NORMAL state, check for screen touch
      // This may cause it to enter a transition state
      if (APP_NORMAL == appState) {
        BSP_TS_GetState(&TS_State);
        checkTouch();
      }

      // If app is in NORMAL state, check for console keypresses
      // This may cause it to enter a transition state
      if (APP_NORMAL == appState) {
        appState = ConsoleProcess(appState);
      }

      // If app is in NORMAL state, check for user button press
      // Double-click resets cursor origin
      // Press and hold draws a stroke
      if (APP_NORMAL == appState) {
        appState = buttonProcess(appState);
      }
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
      | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  ConsoleSendLine("ERROR");
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
