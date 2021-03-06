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
#include "stm32f4xx_hal_gpio.h"
#include "lis2dh_reg.h"
#include "console.h"

static int16_t data_raw_acceleration[3];
static float acceleration_mg[3];
static uint8_t whoamI;

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
    ConsoleSendLine("Error! Accelerometer not recognized.");
    __disable_irq();
    while (1) {
    }
  } else {
    ConsoleSendLine("Accerometer online.");
  }

  // Enable block data update
  lis2dh_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  // Set output data rate (this gets overwritten as soon as tap_config is run)
  //lis2dh_data_rate_set(&dev_ctx, LIS2DH_ODR_1Hz);
  lis2dh_data_rate_set(&dev_ctx, LIS2DH_ODR_400Hz);
  // Set scale to +/- 2g
  lis2dh_full_scale_set(&dev_ctx, LIS2DH_2g);
  // leave device in continuous mode with 10 bit resolution

}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
    uint16_t len) {
  /* write multiple command */
  reg |= 0x40;
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_SPI_Transmit(handle, &reg, 1, 1000);
  HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 1000);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
    uint16_t len) {
  // write multiple command
  reg |= 0xC0; // Example project set this to 0x80??
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_SPI_Transmit(handle, &reg, 1, 1000);
  HAL_SPI_Receive(handle, bufp, len, 1000);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
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
    char buff[200];
    sprintf(buff, "%1.5f,%1.5f,%1.5f,", acceleration_mg[0] * .001,
        acceleration_mg[1] * .001, acceleration_mg[2] * .001);
    ConsoleSendString(buff);
  }
}

// Match accelerometer's axes to my definition of the board's axes (remember accel is mounted on the back)
// Accelerometer: +X toward board down, +Y toward board left, +Z into board
// Desired: +X toward right of board, +Y toward top, +Z out of board
void accel_read(float *accelStruct) {
  lis2dh_reg_t reg;
  lis2dh_xl_data_ready_get(&dev_ctx, &reg.byte);

  if (reg.byte) {
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    lis2dh_acceleration_raw_get(&dev_ctx, data_raw_acceleration);

    accelStruct[0] = -1*lis2dh_from_fs2_nm_to_mg(data_raw_acceleration[1]);
    accelStruct[1] = -1*lis2dh_from_fs2_nm_to_mg(data_raw_acceleration[0]);
    accelStruct[2] = -1*lis2dh_from_fs2_nm_to_mg(data_raw_acceleration[2]);
  }

}

/*
 * Tap feature not used in this project
 */
void accel_tap_init(void) {
  // Based on SparkFun's implementation: https://github.com/sparkfun/SparkFun_LIS2DH12_Arduino_Library/tree/master/examples/Example4_TapDetection

  // lis2dh_block_data_update_set(&dev_ctx, PROPERTY_ENABLE); // Skipping this one for now because I like the continuous update
  lis2dh_data_rate_set(&dev_ctx, LIS2DH_ODR_400Hz);

  lis2dh_click_cfg_t clickReg;
  lis2dh_tap_conf_get(&dev_ctx, &clickReg);
  clickReg.zd = 1;
  lis2dh_tap_conf_set(&dev_ctx, &clickReg);

  lis2dh_tap_threshold_set(&dev_ctx, 50); // TODO: DIAL IN THRESHOLD

  // Clear any initial events
  lis2dh_click_src_t srcReg;
  lis2dh_tap_source_get(&dev_ctx, &srcReg);
  while (srcReg.x || srcReg.y || srcReg.z) {
    ConsoleSendLine("Doubletap registered on init");
  }
}

void accel_check_tap(void) {
  lis2dh_click_src_t srcReg;
  lis2dh_tap_source_get(&dev_ctx, &srcReg);
  if (srcReg.z) {
    ConsoleSendLine("Doubletap registered");
  }
}
