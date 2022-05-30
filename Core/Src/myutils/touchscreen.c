/*
 * touchscreen.c
 *
 *  Created on: Apr 23, 2022
 *      Author: Sam
 */

#include "console.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "touchscreen.h"
#include "stdio.h"
#include "eeprom.h"

extern TS_StateTypeDef TS_State;
static uint8_t Calibration_Done = 0;
static int16_t A1, A2, B1, B2;
static int16_t aPhysX[2], aPhysY[2], aLogX[2], aLogY[2];

/**
 * @brief  Performs the TS calibration and tries saving results to EEPROM
 * @param  None
 * @retval None
 */
void Touchscreen_Calibration(void) {
  uint8_t status = 0;
  uint8_t i = 0;

  TouchscreenCalibration_SetHint();

  status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());

  if (status != TS_OK) {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 95, (uint8_t*) "ERROR",
        CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80,
        (uint8_t*) "Touchscreen cannot be initialized", CENTER_MODE);
  }

  while (1) {
    if (status == TS_OK) {
      aLogX[0] = 15;
      aLogY[0] = 15;
      aLogX[1] = BSP_LCD_GetXSize() - 15;
      aLogY[1] = BSP_LCD_GetYSize() - 15;

      for (i = 0; i < 2; i++) {
        GetPhysValues(aLogX[i], aLogY[i], &aPhysX[i], &aPhysY[i]);
      }
      A1 = (1000 * (aLogX[1] - aLogX[0])) / (aPhysX[1] - aPhysX[0]);
      B1 = (1000 * aLogX[0]) - A1 * aPhysX[0];

      A2 = (1000 * (aLogY[1] - aLogY[0])) / (aPhysY[1] - aPhysY[0]);
      B2 = (1000 * aLogY[0]) - A2 * aPhysY[0];

      Calibration_Done = 1;

      // Store values in EEPROM
      ConsoleSendLine("EEPROM initializing...");
      HAL_FLASH_Unlock();
      HAL_Delay(10);
      if (EE_Init() != EE_OK) {
        return; // Error handler?
      }
      ConsoleSendLine("EEPROM initialized");

//      if ((EE_WriteVariable(EEPROM_LCD_A1_ADDRESS, A1)) != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: A1");
//        return;
//      }
//      if ((EE_WriteVariable(EEPROM_LCD_A2_ADDRESS, A2)) != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: A2");
//        return;
//      }
//      if ((EE_WriteVariable(EEPROM_LCD_B1_ADDRESS, B1)) != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: B1");
//        return;
//      }
//      if ((EE_WriteVariable(EEPROM_LCD_B2_ADDRESS, B2)) != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: B2");
//        return;
//      }
//
//      if ((EE_WriteVariable(EEPROM_CHECK_0_ADDRESS, EEPROM_CHECK_0))
//          != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: Check 0");
//        return;
//      }
//      if ((EE_WriteVariable(EEPROM_CHECK_1_ADDRESS, EEPROM_CHECK_1))
//          != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: Check 1");
//        return;
//      }
//      if ((EE_WriteVariable(EEPROM_CHECK_2_ADDRESS, EEPROM_CHECK_2))
//          != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: Check 2");
//        return;
//      }
//      if ((EE_WriteVariable(EEPROM_CHECK_3_ADDRESS, EEPROM_CHECK_3))
//          != HAL_OK) {
//        ConsoleSendLine("Error writing to EEPROM: Check 3");
//        return;
//      }
      HAL_FLASH_Lock();
      ConsoleSendLine("EEPROM written");

      return;
    }

    HAL_Delay(5);
  }
}

/**
 * @brief  Display calibration hint
 * @param  None
 * @retval None
 */
void TouchscreenCalibration_SetHint(void) {
  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  /* Set Touchscreen Demo description */
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 - 27,
      (uint8_t*) "Before using the Touchscreen", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 - 12,
      (uint8_t*) "you need to calibrate it.", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 + 3,
      (uint8_t*) "Press on the black circles", CENTER_MODE);
}

/**
 * @brief  Get Physical position
 * @param  LogX : logical X position
 * @param  LogY : logical Y position
 * @param  pPhysX : Physical X position
 * @param  pPhysY : Physical Y position
 * @retval None
 */
void GetPhysValues(int16_t LogX, int16_t LogY, int16_t *pPhysX, int16_t *pPhysY) {
  /* Draw the ring */
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillCircle(LogX, LogY, 5);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillCircle(LogX, LogY, 2);

  /* Wait until touch is pressed */
  WaitForPressedState(1);

  BSP_TS_GetState(&TS_State);
  *pPhysX = TS_State.X;
  *pPhysY = TS_State.Y;

  /* Wait until touch is released */
  WaitForPressedState(0);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillCircle(LogX, LogY, 5);
}

/**
 * @brief  Wait For Pressed State
 * @param  Pressed: Pressed State
 * @retval None
 */
void WaitForPressedState(uint8_t Pressed) {
  TS_StateTypeDef State;

  do {
    BSP_TS_GetState(&State);
    HAL_Delay(10);
    if (State.TouchDetected == Pressed) {
      uint16_t TimeStart = HAL_GetTick();
      do {
        BSP_TS_GetState(&State);
        HAL_Delay(10);
        if (State.TouchDetected != Pressed) {
          break;
        } else if ((HAL_GetTick() - 100) > TimeStart) {
          return;
        }
      } while (1);
    }
  } while (1);
}

/**
 * @brief  Calibrate X position
 * @param  x: X position
 * @retval calibrated x
 */
uint16_t Calibration_GetX(uint16_t x) {
  return (((A1 * x) + B1) / 1000);
}

/**
 * @brief  Calibrate Y position
 * @param  y: Y position
 * @retval calibrated y
 */
uint16_t Calibration_GetY(uint16_t y) {
  return (((A2 * y) + B2) / 1000);
}

/**
 * @brief  Check if the TS is calibrated
 * @param  None
 * @retval calibration state
 */
uint8_t IsCalibrationDone(void) {
  return (Calibration_Done);
}

// Touchscreen parameter loading
// Check for A1, A2, B1, B2 values stored in EEPROM
// If those exist, use those while initializing touchscreen
// If they don't, enter the calibration screen
int TS_Get_Params(void) {
  ConsoleSendLine("EEPROM initializing...");
  HAL_FLASH_Unlock();
  HAL_Delay(10);
  if (EE_Init() != EE_OK) {
    return 2; // MAGIC NUMBER
  }
  ConsoleSendLine("EEPROM initialized");

//  uint16_t readInt;

//  // Check the EE_Check values
//  //   If those match expected, load A1 A2 B1 B2 from memory
//  if ((EE_ReadVariable(EEPROM_CHECK_0_ADDRESS, &readInt)) != HAL_OK) {
//    ConsoleSendLine("Problem loading settings!");
//    return 2; // MAGIC NUMBER
//  }
//  if (readInt != EEPROM_CHECK_0) {
//    ConsoleSendLine("No EEPROM data found.");
//    return 1; // MAGIC NUMBER
//  }
//
//  if ((EE_ReadVariable(EEPROM_LCD_A1_ADDRESS, &readInt)) != HAL_OK) {
//    ConsoleSendLine("Problem loading A1!");
//    return 2; // MAGIC NUMBER
//  }
//  A1 = (int16_t) readInt;
//
//  if ((EE_ReadVariable(EEPROM_LCD_A2_ADDRESS, &readInt)) != HAL_OK) {
//    ConsoleSendLine("Problem loading A2!");
//    return 2; // MAGIC NUMBER
//  }
//  A2 = (int16_t) readInt;
//
//  if ((EE_ReadVariable(EEPROM_LCD_B1_ADDRESS, &readInt)) != HAL_OK) {
//    ConsoleSendLine("Problem loading B1!");
//    return 2; // MAGIC NUMBER
//  }
//  B1 = (int16_t) readInt;
//
//  if ((EE_ReadVariable(EEPROM_LCD_B2_ADDRESS, &readInt)) != HAL_OK) {
//    ConsoleSendLine("Problem loading B2!");
//    return 2; // MAGIC NUMBER
//  }
//  B2 = (int16_t) readInt;

  ConsoleSendLine("LCD configuration loaded.");

//  Serial_Message("A1:");
//  Print_Int(A1);
//  Serial_Message("B1:");
//  Print_Int(B1);
//  Serial_Message("A2:");
//  Print_Int(A2);
//  Serial_Message("B2:");
//  Print_Int(B2);
  HAL_FLASH_Lock();
  return 0; // MAGIC NUMBER
}
