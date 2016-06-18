/*
 * led.h
 *
 *  Created on: Jun 17, 2016
 *      Author: ChauNM
 */

#ifndef LED_H_
#define LED_H_

#define LED_RED_PIN 		23
#define LED_GREEN_PIN 		24

#define LED_OFF				0x01
#define LED_ON				0x02
#define LED_BLINK			0x03

void LedInit();
void LedTurnOff();
int LedTurnOn(const char* color);
int LedBlink(const char* color, int freq);
#endif /* LED_H_ */
