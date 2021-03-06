/*
 * led.c
 *
 *  Created on: Jun 17, 2016
 *      Author: ChauNM
 */

//#include <wiringPi.h>
#include <string.h>
#include <stdio.h>
#include "led.h"
#include "universal.h"
#include "pigpio.h"
#include <pthread.h>
#ifdef PI_RUNNING
#include "unistd.h"
#endif

pthread_t ledBlinkingThread;
int blinkingFreq = 0;
int ledState = LED_OFF;
unsigned char redBlinkValue = 0;
unsigned char greenBlinkValue = 0;
unsigned char blueBlinkValue = 0;
unsigned char redCurrent = 0;
unsigned char blueCurrent = 0;
unsigned char greenCurrent = 0;
unsigned char redSet = 0;
unsigned char greenSet = 0;
unsigned char blueSet = 0;
char ledBlinkingStateRunning;

void LedInit()
{
	//wiringPiSetupGpio();
	//wiringPiSetupSys();
	gpioInitialise();
	//set GPIO 12 and GPIO 13 to use hardware PWM
	gpioSetMode(LED_RED_PIN, PI_ALT0);
	gpioSetMode(LED_GREEN_PIN, PI_ALT0);
	//set GPIO 19 and GPIO 26 to use as output
	gpioSetMode(LED_BLUE_PIN, PI_OUTPUT);
	gpioSetMode(LED_CTL_PIN, PI_OUTPUT);;
	//clear all output
	gpioWrite(LED_CTL_PIN, PI_LOW);
	gpioWrite(LED_BLUE_PIN, PI_LOW);
	// configure pwm channel
	// diviser 16 in mark/space mode give the pwm freq to be nearly 1171Hz
	gpioSetPWMrange(LED_BLUE_PIN, 255);
	gpioSetPWMfrequency(LED_BLUE_PIN, PWM_FREQ);
	gpioPWM(LED_BLUE_PIN, 0);
	gpioWrite(LED_BLUE_PIN, 0);
	gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, 0);
	gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, 0);
	gpioWrite(LED_CTL_PIN, PI_HIGH);
	ledState = LED_OFF;
}

void LedDeinit()
{
	gpioPWM(LED_BLUE_PIN, 0);
	gpioWrite(LED_BLUE_PIN, 0);
	gpioWrite(LED_CTL_PIN, 0);
	gpioHardwarePWM(LED_RED_PIN, 0, 0);
	gpioHardwarePWM(LED_GREEN_PIN, 0, 0);
	gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, 0);
	gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, 0);
	gpioTerminate();
}

void LedTurnOff()
{
	if (ledState == LED_BLINK)
	{
		redCurrent = redBlinkValue;
		greenCurrent = greenBlinkValue;
		blueCurrent = blueBlinkValue;
	}
	redSet = 0;
	greenSet = 0;
	blueSet = 0;
//	gpioWrite(LED_CTL_PIN, PI_LOW);
//	gpioPWM(LED_BLUE_PIN, 0);
//	gpioHardwarePWM(LED_RED_PIN, 0, 0);
//	gpioHardwarePWM(LED_GREEN_PIN, 0, 0);
	ledState = LED_OFF;
}

int LedTurnOn(BYTE red, BYTE green, BYTE blue)
{
	redSet = red;
	greenSet = green;
	blueSet = blue;
	if (ledState == LED_BLINK)
	{
		redCurrent = redBlinkValue;
		greenCurrent = greenBlinkValue;
		blueCurrent = blueBlinkValue;
	}
	gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, redCurrent * 1000000 / 255);
	gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, greenCurrent * 1000000 / 255);
	gpioPWM(LED_BLUE_PIN, blueCurrent);
	gpioWrite(LED_CTL_PIN, PI_HIGH);
	ledState = LED_ON;
	return 0;
}

void LedBlinkingProcess()
{
	if (ledState != LED_BLINK)
	{
		ledBlinkingStateRunning = FALSE;
		return;
	}
	while (1)
	{
		if (ledState != LED_BLINK)
		{
			ledBlinkingStateRunning = FALSE;
			return;
		}
		//gpioWrite(LED_CTL_PIN, !(gpioRead(LED_CTL_PIN)));
		if ((redBlinkValue == 0) && (greenBlinkValue == 0) && (blueBlinkValue == 0))
		{
			redBlinkValue = redCurrent;
			greenBlinkValue = greenCurrent;
			blueBlinkValue = blueCurrent;
		}
		else
		{
			redBlinkValue = 0;
			greenBlinkValue = 0;
			blueBlinkValue = 0;
		}
		gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, redBlinkValue * 1000000 / 255);
		gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, greenBlinkValue * 1000000 / 255);
		gpioPWM(LED_BLUE_PIN, blueBlinkValue);
		usleep(1000 * blinkingFreq / 2);
	}
}

int LedBlink(BYTE red, BYTE green, BYTE blue, int period)
{
	blinkingFreq = period;
	ledState = LED_BLINK;
	redSet = red;
	greenSet = green;
	blueSet = blue;
	redCurrent = red;
	greenCurrent = green;
	blueCurrent = blue;
	redBlinkValue = redCurrent;
	greenBlinkValue = greenCurrent;
	blueBlinkValue = blueCurrent;
	gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, redBlinkValue * 1000000 / 255);
	gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, greenBlinkValue * 1000000 / 255);
	gpioPWM(LED_BLUE_PIN, blueBlinkValue);
	gpioWrite(LED_CTL_PIN, PI_HIGH);
	if (ledBlinkingStateRunning == FALSE)
	{
		// create a blinking thread if necessary
		printf("Start Blinking\n");
		pthread_create(&ledBlinkingThread, NULL, (void*)&LedBlinkingProcess, NULL);
		pthread_detach(ledBlinkingThread);
		ledBlinkingStateRunning = TRUE;
	}
	return 0;
}

void LedTransition()
{
	if ((redCurrent == redSet) && (greenCurrent == greenSet) && (blueCurrent == blueSet))
		return;
	if (redCurrent < redSet)
		redCurrent++;
	if (redCurrent > redSet)
		redCurrent--;
	if (greenCurrent < greenSet)
		greenCurrent++;
	if (greenCurrent > greenSet)
		greenCurrent--;
	if (blueCurrent < blueSet)
		blueCurrent++;
	if (blueCurrent > blueSet)
		blueCurrent--;
	if ((ledState == LED_ON) || (ledState == LED_OFF))
	{
		gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, redCurrent * 1000000 / 255);
		gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, greenCurrent * 1000000 / 255);
		gpioPWM(LED_BLUE_PIN, blueCurrent);
	}
}


