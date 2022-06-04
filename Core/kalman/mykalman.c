/*
 * mykalman.c
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

uint32_t timer;
float acceleration_mg[3];
float gyro_mdps[3];

Kalman_t KalmanX = { .Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f };

Kalman_t KalmanY = { .Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f };


void getReadings(sensors_t *sensorStruct, uint32_t t) {
  accel_read(acceleration_mg);
  BSP_GYRO_GetXYZ(gyro_mdps);

  sensorStruct->Ax = acceleration_mg[0]/1000;
  sensorStruct->Ay = acceleration_mg[1]/1000;
  sensorStruct->Az = acceleration_mg[2]/1000;
  sensorStruct->Gx = gyro_mdps[0]/1000;
  sensorStruct->Gy = gyro_mdps[1]/1000;
  sensorStruct->Gz = gyro_mdps[2]/1000;

  // Kalman angle solve
  float dt = (float) (t) / 1000;

  float roll_radians = 0.0;
  float accel_norm = sqrtf(
      sensorStruct->Ax * sensorStruct->Ax
          + sensorStruct->Ay * sensorStruct->Ay
          + sensorStruct->Az * sensorStruct->Az);
  if (accel_norm != 0.0) {
    roll_radians = atanf(sensorStruct->Ay / accel_norm);
  }

  float pitch_radians = atan2f(sensorStruct->Ax, sensorStruct->Az);
  if ((pitch_radians < -HALFPI && sensorStruct->KalmanAngleY > 90)
      || (pitch_radians > HALFPI && sensorStruct->KalmanAngleY)) {
    KalmanY.angle = pitch_radians;
    sensorStruct->KalmanAngleY = pitch_radians;
  } else {
    sensorStruct->KalmanAngleY = Kalman_getAngle(&KalmanY, pitch_radians,
        sensorStruct->Gy, dt);
  }
  if (fabsf(sensorStruct->KalmanAngleY) > 90) {
    sensorStruct->Gx = -sensorStruct->Gx;
  }
  sensorStruct->KalmanAngleX = Kalman_getAngle(&KalmanX, roll_radians,
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
