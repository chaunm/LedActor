/*
 * led.h
 *
 *  Created on: Jun 17, 2016
 *      Author: ChauNM
 */

#ifndef LED_H_
#define LED_H_

#define LED_RED_PIN 		12
#define LED_GREEN_PIN 		13
#define LED_BLUE_PIN		19

#define LED_CTL_PIN			26

#define LED_OFF				0x01
#define LED_ON				0x02
#define LED_BLINK			0x03

#define PWM_FREQ			1000

void LedInit();
void LedDeinit();
void LedTurnOff();
int LedTurnOn(BYTE red, BYTE green, BYTE blue);
int LedBlink(BYTE red, BYTE green, BYTE blue, int freq);
void LedTransition();
#endif /* LED_H_ */
