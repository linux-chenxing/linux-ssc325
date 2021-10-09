/*
* mdrv_pwm.h Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#ifndef __MDRV_PWM_H__
#define __MDRV_PWM_H__

int CamPwmConfig(int pwm, int duty_ns,int period_ns);
//struct pwm_device *CamPwmRequest(int pwm, const char *label);
int CamPwmEnable(int pwm);
void CamPwmDisable(int pwm);

#endif // #ifndef __MDRV_PWM_H__
