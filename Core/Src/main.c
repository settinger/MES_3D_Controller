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
#include "stm32f429xx.h"
#include <stdlib.h>
#include "string.h"
#include "stdio.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "stm32f429i_discovery_gyroscope.h"

#include "app.h"
#include "console.h"
#include "touchscreen.h"
#include "accel.h"
#include "mykalman.h"
#include "color.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FRAME_DELAY      25      // Time to wait (in milliseconds) between updating frames
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
structAppState appState;                   // Program state in state machine
uint16_t touchStateTransition = 0;    // Counter for touch state event detection
void (*checkTouch)(void);               // Function pointer for touch states
clientColor currentColor = COLOR_DEFAULT; // The color of the cursor
uint16_t currentSize = 20; // The current radius (in UV Map pixels) of the cursor

sensors_t boardSensors; // The struct that holds accelerometer data, gyro data, and Kalman filtered estimates of board's Euler angle

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void handleTouchBegin(void);
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
static void handleTouchBegin(void) {
  int16_t x = TS_State.X;
  int16_t y = TS_State.Y;
  y = 320 - y;
  char string[60];
  sprintf(string, "Touch X coordinate: %d\r\nTouch Y coordinate: %d", x, y);
  ConsoleSendLine(string);
  if (appState == APP_NORMAL) {
    appState = mainScreenTouchHandler(x, y);
  } else if (appState == APP_COLORPICKER) {
    appState = colorPickerTouchHandler(x, y);
  } else if (appState == APP_SIZEPICKER) {
    appState = sizePickerTouchHandler(x, y);
  }
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
      handleTouchBegin();
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

/*
 * External-interrupt callback to turn on LD4 when a button is pressed
 * (and off when button is released).
 *
 * This WAS used with a debouncing algorithm to handle button presses
 * outside of the normal 40 FPS loop, but it felt cleaner to handle it
 * without using interrupts during normal loop operation. But the
 * interrupt remains, toggling an LED to demonstrate that I have the
 * capacity to use a button interrupt if so needed.
 *
 * For an interrupt that actually does something useful, see the
 * ConsoleIoReceive() method in "console_io.c" in the Projective Set
 * repository. That uses the non-blocking HAL_UART_Receive_IT() HAL
 * method to detect and process received serial data. It exists in this
 * repo as well but isn't used, since the UART here only transmits.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  HAL_GPIO_WritePin(LED4_GPIO_PORT, LD4_Pin, BSP_PB_GetState(BUTTON_KEY));
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
  // Axes: +X to board right, +Y to board top, +Z out of board
  if (GYRO_OK == BSP_GYRO_Init()) {
    ConsoleSendLine("Gyro initialized.");
  } else {
    ConsoleSendLine("ERROR! Unable to initialize gyro.");
    __disable_irq();
    while (1) {
    }
  }

  // Enable accelerometer
  // LIS2DH connected to SPI5
  // PF7 - SCK
  // PF8 - Sensor data out
  // PF9 - Sensor data in
  // PF6 - Chip select
  // Axes: +X to board left, +Y to board top, +Z into board
  accel_init();

  // Set up double-tap on accelerometer
  //accel_tap_init();

  // Set the initial values for the Kalman filter to work from
  HAL_Delay(100); // Wait for sensors to stabilize

  // Enable USB HID operations

  // Set initial state
  lastFrameTick = HAL_GetTick();
  lastSecondTick = lastFrameTick;
  appStart = lastSecondTick;
  appState = APP_INIT;
  appInit();
  checkTouch = &clearIdle;
  appState = APP_NORMAL;

  ConsoleMoveEuler(MOVE_CURSOR, 0.0, 0.0); // This will reset the orientation of the cursor on the web client
  ConsoleChangeColor(currentColor); // This ensures the web client color matches the initial board color
  ConsoleResizeCursor(currentSize); // This ensures the web client cursor size matches the board's

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    nextTick = HAL_GetTick();
    // Every 25 milliseconds, update sensors and instruct the web client to update
    if ((nextTick - lastFrameTick) >= FRAME_DELAY) {
      // Update gyro reading, accel reading, and Kalman euler angles
      getReadings(&boardSensors, (nextTick - lastFrameTick));

      // Check if a double-tap event has occurred
      //accel_check_tap();

      // If button is pressed, update cursor location and drawing; otherwise, just update cursor location
      if (GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY)) {
        ConsoleMoveEuler(MOVE_AND_DRAW, boardSensors.KalmanStateTheta,
            boardSensors.KalmanStatePhi);
      } else {
        ConsoleMoveEuler(MOVE_CURSOR, boardSensors.KalmanStateTheta,
            boardSensors.KalmanStatePhi);
      }

      // If touch screen is pressed, handle it here
      // (Touch screen used to change cursor color and size)
      BSP_TS_GetState(&TS_State);
      checkTouch();

      lastFrameTick = nextTick;
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
