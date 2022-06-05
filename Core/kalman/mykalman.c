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

Kalman_t KalmanTheta = { .Q_angle = 0.001f, .Q_bias = 0.003f,
    .R_measure = 0.03f, angle = 0.0 };

Kalman_t KalmanPhi = { .Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f,
    angle = 0.0 };

void getReadings(sensors_t *sensorStruct, uint32_t t) {
  accel_read(acceleration_mg);
  BSP_GYRO_GetXYZ(gyro_mdps);

  /*
   * This is obnoxious but I may need to compensate for the fact that
   * three.js camera only supports +Z out of screen, +Y up, +X right.
   * This should make the axes match when the board is being held with
   * the USB-mini cable toward the ceiling.
   */
  sensorStruct->Ax = acceleration_mg[0] / 1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Ay = acceleration_mg[1] / 1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Az = acceleration_mg[2] / 1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Gx = gyro_mdps[0] / 1000; // Unit: degrees per second
  sensorStruct->Gy = gyro_mdps[1] / 1000; // Unit: degrees per second
  sensorStruct->Gz = gyro_mdps[2] / 1000; // Unit: degrees per second

  float dt = (float) (t) / 1000; // Unit: seconds

  /*
   * Begin Kalman filter
   * State estimation: use accelerometer data to get naive
   * values for theta and phi
   */

  /*
   * Compute phi from accelerometers, return result in degrees
   * When holding the board upright with USB-mini pointed toward the
   * ceiling and screen toward the user, phi is measured as the angle
   * between the board's Y axis and gravity.
   * This assumes the force of gravity is much greater than the force
   * being applied by the user to the controller.
   *
   */
  float phi = 0.0;
  float force_magnitude = sqrtf(
      sensorStruct->Ax * sensorStruct->Ax + sensorStruct->Ay * sensorStruct->Ay
          + sensorStruct->Az * sensorStruct->Az);
  if (force_magnitude != 0.0) {
    phi = acosf(sensorStruct->Ay / force_magnitude) * RAD_TO_DEG;
  }

  /*
   * Compute theta from accelerometers, return result in degrees
   * Theta is measured as the angle made by the X-axis acceleration
   * component and the Z-axis acceleration component.
   */
  float theta = atan2f(sensorStruct->Ax, sensorStruct->Az) * RAD_TO_DEG;

  // Apply corrections to theta/phi estimates (TODO: why here? shouldn't this be done to the newly calculated states instead?)
   if ((theta < -90 && sensorStruct->KalmanEstimatedPhi > 90)
   || (theta > 90 && sensorStruct->KalmanEstimatedPhi < -90)) {
   KalmanTheta.angle = theta;
   sensorStruct->KalmanEstimatedTheta = theta;
   } else {
   sensorStruct->KalmanEstimatedTheta = Kalman_getAngle(&KalmanTheta, theta,
   sensorStruct->Gx, dt);
   }

   // Correct for Gx overflow (TODO: why?)
   if (fabsf(sensorStruct->KalmanEstimatedPhi) > 90) {
   sensorStruct->Gx = -sensorStruct->Gx;
   }


  /*
   * Update steps performed here and commented within the function
   */
  sensorStruct->KalmanEstimatedPhi = kalmanUpdate(&KalmanPhi, phi,
      FILLTHISINLATER);
  //sensorStruct->KalmanEstimatedTheta = kalmanUpdate(&KalmanTheta, theta, sensorStruct->Gx, dt);
}

/*
 * The update steps of the Kalman filter process
 */
float kalmanUpdate(Kalman_t *Kalman, float newAngle, float newRate, float dt) {
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
  Kalman->angle += K[0] * y;
  Kalman->bias += K[1] * y;

  float P00_temp = Kalman->P[0][0];
  float P01_temp = Kalman->P[0][1];

  Kalman->P[0][0] -= K[0] * P00_temp;
  Kalman->P[0][1] -= K[0] * P01_temp;
  Kalman->P[1][0] -= K[1] * P00_temp;
  Kalman->P[1][1] -= K[1] * P01_temp;

  return Kalman->angle;
}
