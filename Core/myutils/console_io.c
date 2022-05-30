/*
 * console_io.c
 * A wrapper between raw inputs and console operations
 * Expects console inputs to be receved on STM32F429I-DISC1 USART5
 * Ported from Elecia White's code here:
 * https://wokwi.com/projects/324879108372693587
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#include "../../myutils/console_io.h"

#include <stdio.h>
#include <stdbool.h>
#include "string.h"
#include "usart.h"

static uint8_t chReceive;
static bool uart_rx;

consoleError ConsoleIoInit(void) {
  /* Enable UART5 interrupt*/
  HAL_NVIC_SetPriority(UART5_IRQn, 4, 0); // I have no idea what priority to assign this
  HAL_NVIC_EnableIRQ(UART5_IRQn);
  HAL_UART_Receive_IT(&huart5, &chReceive, 1);
  return CONSOLE_SUCCESS;
}

// Receive from console (blocking? non-blocking?)
consoleError ConsoleIoReceive(uint8_t *buffer, const uint32_t bufferLength,
    uint32_t *readLength) {
  uint32_t i = 0;

  if (uart_rx) {
    uart_rx = false;
    HAL_UART_Transmit(&huart5, &chReceive, 1, 250); // Display the received character in the console
    buffer[i] = chReceive;
    i++;
    HAL_UART_Receive_IT(&huart5, &chReceive, 1);
  }

  *readLength = i;

  return CONSOLE_SUCCESS;
}

consoleError ConsoleIoSend(const char *buffer) {
  HAL_UART_Transmit(&huart5, (uint8_t*) buffer, strlen(buffer), 100);
  return CONSOLE_SUCCESS;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
  /* Set transmission flag: transfer complete*/
  uart_rx = true;
}
