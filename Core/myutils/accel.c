/*
 * accel.c
 * Cribbing from example here: https://github.com/STMicroelectronics/STMems_Standard_C_drivers/blob/master/lis2dh_STdC/examples/lis2dh_read_data_polling.c
 *
 *  Created on: May 30, 2022
 *      Author: Sam
 */

#include "accel.h"
#include "stdio.h"
#include <string.h>
#include "stm32f4xx_hal.h"
#include "lis2dh_reg.h"
#include "console.h"

static int16_t data_raw_acceleration[3];
static int16_t data_raw_temperature;
static float acceleration_mg[3];
static float temperature_degC;
static uint8_t whoamI;
static uint8_t tx_buffer[1000];

stmdev_ctx_t dev_ctx;

// HSPI5 defined initialized elsewhere
extern HAL_SPI_StateTypeDef hspi5;

void accel_init(void) {
  // Initialize MEMS driver interface
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hspi5;
  // After delay to ensure platform is initialized, check device ID
  lis2dh_device_id_get(&dev_ctx, &whoamI);
  if (LIS2DH_ID != whoamI) {
    while (1) {
    }  // TODO: ERROR HANDLER
  }

  // Enable block data update
  lis2dh_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  // Set output data rate to 1Hz
  lis2dh_data_rate_set(&dev_ctx, LIS2DH_ODR_1Hz);
  // Set scale to +/- 2g
  lis2dh_full_scale_set(&dev_ctx, LIS2DH_2g);
  // leave device in continuous mode with 10 bit resolution

}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
    uint16_t len) {
  /* write multiple command */
  reg |= 0x40;
  //HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState) reset pin
  HAL_SPI_Transmit(handle, &reg, 1, 100);
  HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 100);
  //HAL_GPIO_WritePin set pin
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
    uint16_t len) {
  // write multiple command
  reg |= 0x80; // Shouldn't this be 0xC0??
  // GPIO reset pin
  HAL_SPI_Transmit(handle, &reg, 1, 100);
  HAL_SPI_Receive(handle, bufp, len, 100);
  // GPI set pin
  return 0;
}

void accel_getValues(void) {
  lis2dh_reg_t reg;
  // Check if new data is available
  lis2dh_xl_data_ready_get(&dev_ctx, &reg.byte);

  if (reg.byte) {
    // Read accelerometer data
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    lis2dh_acceleration_raw_get(&dev_ctx, data_raw_acceleration);

    acceleration_mg[0] = lis2dh_from_fs2_nm_to_mg(data_raw_acceleration[0]);
    acceleration_mg[1] = lis2dh_from_fs2_nm_to_mg(data_raw_acceleration[1]);
    acceleration_mg[2] = lis2dh_from_fs2_nm_to_mg(data_raw_acceleration[2]);
    sprintf((char*) tx_buffer, "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
        acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
    ConsoleSendLine((char*)tx_buffer);
  }
}
