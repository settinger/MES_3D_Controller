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
    ConsoleSendLine("Error! Accelerometer not recognized.");
    while (1) {
    }  // TODO: ERROR HANDLER
  } else {
    ConsoleSendLine("Accerometer online.");
  }

  // Enable block data update
  lis2dh_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  // Set output data rate to 1Hz (this gets overwritten as soon as tap_config is run)
  lis2dh_data_rate_set(&dev_ctx, LIS2DH_ODR_1Hz);
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
    sprintf((char*) tx_buffer, "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f",
        acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
    ConsoleSendLine((char*) tx_buffer);
  }
}

void accel_tap_init(void) {
  // Based on SparkFun's implementation: https://github.com/sparkfun/SparkFun_LIS2DH12_Arduino_Library/tree/master/examples/Example4_TapDetection

  /*
   * Initialize
   *    lis2dh12_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
   *      I think this means output registers aren't cleared until they are read
   *    set block data update rate ODR 25Hz
   *    Set full scale to +/- 2g
   *    turn on temp sensor
   *    set to continuous 12bit mode
   * Enable tap detection
   *    use lis2dh12_tap_conf_get
   *    set xs, ys, zs to true
   * Set tap threshold to 40
   * Clear any initial events by waiting for accel.isTapped
   *
   * to check for taps:
   *    lis2dh12_tap_source_get
   *    check x, y, z
   *    then wait 10 milliseconds?
   */
}

static void accel_tap_init_BAD(void) {
  // Set output data rate to 400Hz so we can do tap detection
  lis2dh_data_rate_set(&dev_ctx, LIS2DH_ODR_400Hz);

  // Edit the Click Config register
  // Enable doubletap on Z
  lis2dh_click_cfg_t cfgReg;
  lis2dh_tap_conf_get(&dev_ctx, &cfgReg);
  cfgReg.zs = 1; ///////// Todo: set this back to zd (double-click)
  cfgReg.xs = 1;
  cfgReg.ys = 1;
  lis2dh_tap_conf_set(&dev_ctx, &cfgReg);

  // Enable high-pass filter for click interrupt
  // Set HPCLICK bit in CTRL_REG2


  // Set I2_CLICKen in CTRL_REG6 to tie click interrupt to pin 2
  lis2dh_ctrl_reg6_t ctrlReg;
  lis2dh_pin_int2_config_get(&dev_ctx, &ctrlReg);
  ctrlReg.i2_clicken = 1;
  lis2dh_pin_int2_config_set(&dev_ctx, &ctrlReg);

  // Set LIR_INT2 in CTRL_REG5 to latch the pin2 interrupt flag (only clear once read)
  //lis2dh_int2_pin_notification_mode_set(&dev_ctx, LIS2DH_INT2_LATCHED);

  // Set tap threshold to 50 digits and duration to 5 ms
  lis2dh_tap_threshold_set(&dev_ctx, 5);
  lis2dh_int2_gen_duration_set(&dev_ctx, 20);
  lis2dh_shock_dur_set(&dev_ctx, 100);

  lis2dh_hp_t HPReg;
  lis2dh_high_pass_int_conf_get(&dev_ctx, &HPReg);
  HPReg |= LIS2DH_ON_TAP_GEN;
  lis2dh_high_pass_int_conf_set(&dev_ctx, HPReg);


  /////////TODO: datasheet https://www.st.com/resource/en/datasheet/lis2dh.pdf


}

void accel_check_tap(void) {
//  lis2dh_click_src_t srcReg;
//  lis2dh_tap_source_get(&dev_ctx, &srcReg);
//  if (srcReg.ia || srcReg.z) {
//    ConsoleSendLine("Doubletap registered");
//  }
  // Reading CLICK_SRC register directly didn't work, so
  // try reading INT2_SRC instead (after configuring it)

  lis2dh_int2_src_t intReg;
  lis2dh_int2_gen_source_get(&dev_ctx, &intReg);
  if (intReg.ia || intReg.xh || intReg.yh || intReg.zh) {
    ConsoleSendLine("Doubletap registered");
  }

}
