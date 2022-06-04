/*
 * mykalman.c
 * Largely based on the MPU6050 library by Konstantin Bulanov: https://github.com/leech001/MPU6050
 * That library is released under a GNU GPL v3.0 license.
 *
 *  Created on: Jun 3, 2022
 *      Author: Sam
 */

#include "mykalman.h"
#include "accel.h"
#include "stm32f429i_discovery_gyroscope.h"
#include <math.h>

#define PI 3.141592f
#define HALFPI 1.570796f
#define RAD_TO_DEG 180/PI

uint32_t timer;
float acceleration_mg[3];
float gyro_mdps[3];

Kalman_t KalmanX = { .Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f };

Kalman_t KalmanY = { .Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f };


void getReadings(sensors_t *sensorStruct, uint32_t t) {
  accel_read(acceleration_mg);
  BSP_GYRO_GetXYZ(gyro_mdps);

  sensorStruct->Ax = acceleration_mg[0]/1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Ay = acceleration_mg[1]/1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Az = acceleration_mg[2]/1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Gx = gyro_mdps[0]/1000; // Unit: degrees per second
  sensorStruct->Gy = gyro_mdps[1]/1000; // Unit: degrees per second
  sensorStruct->Gz = gyro_mdps[2]/1000; // Unit: degrees per second

  // Kalman angle solve
  float dt = (float) (t) / 1000; // Unit: seconds

  float roll = 0.0;
  float accel_norm = sqrtf(
      sensorStruct->Ax * sensorStruct->Ax
          + sensorStruct->Ay * sensorStruct->Ay
          + sensorStruct->Az * sensorStruct->Az);
  if (accel_norm != 0.0) {
    roll = atanf(sensorStruct->Ay / accel_norm) * RAD_TO_DEG;
  }

  float pitch = atan2f(sensorStruct->Ax, sensorStruct->Az) * RAD_TO_DEG;
  if ((pitch < -90 && sensorStruct->KalmanAngleY > 90)
      || (pitch > 90 && sensorStruct->KalmanAngleY)) {
    KalmanY.angle = pitch;
    sensorStruct->KalmanAngleY = pitch;
  } else {
    sensorStruct->KalmanAngleY = Kalman_getAngle(&KalmanY, pitch,
        sensorStruct->Gy, dt);
  }
  if (fabsf(sensorStruct->KalmanAngleY) > 90) {
    sensorStruct->Gx = -sensorStruct->Gx;
  }
  sensorStruct->KalmanAngleX = Kalman_getAngle(&KalmanX, roll,
      sensorStruct->Gx, dt);
}

float Kalman_getAngle(Kalman_t *Kalman, float newAngle, float newRate, float dt) {
  float rate = newRate - Kalman->bias;
  Kalman->angle += dt * rate;

  Kalman->P[0][0] += dt
      * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0]
          + Kalman->Q_angle);
  Kalman->P[0][1] -= dt * Kalman->P[1][1];
  Kalman->P[1][0] -= dt * Kalman->P[1][1];
  Kalman->P[1][1] += Kalman->Q_bias * dt;

  float S = Kalman->P[0][0] + Kalman->R_measure;
  float K[2];
  K[0] = Kalman->P[0][0] / S;
  K[1] = Kalman->P[1][0] / S;

  float y = newAngle - Kalman->angle;
  Kalman->angle += K[0]*y;
  Kalman->bias += K[1]*y;

  float P00_temp = Kalman->P[0][0];
  float P01_temp = Kalman->P[0][1];

  Kalman->P[0][0] -= K[0] * P00_temp;
  Kalman->P[0][1] -= K[0] * P01_temp;
  Kalman->P[1][0] -= K[1] * P00_temp;
  Kalman->P[1][1] -= K[1] * P01_temp;

  return Kalman->angle;
}
