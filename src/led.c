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

	ledState = LED_OFF;
}

void LedDeinit()
{
	gpioPWM(LED_BLUE_PIN, 0);
	gpioWrite(LED_BLUE_PIN, 0);
	gpioWrite(LED_CTL_PIN, 0);
	gpioHardwarePWM(LED_RED_PIN, 0, 0);
	gpioHardwarePWM(LED_GREEN_PIN, 0, 0);

	gpioTerminate();
}

void LedTurnOff()
{
	gpioWrite(LED_CTL_PIN, PI_LOW);
	gpioPWM(LED_BLUE_PIN, 0);
	gpioHardwarePWM(LED_RED_PIN, 0, 0);
	gpioHardwarePWM(LED_GREEN_PIN, 0, 0);
	ledState = LED_OFF;
}

int LedTurnOn(BYTE red, BYTE green, BYTE blue)
{
	gpioHardwarePWM(LED_RED_PIN, PWM_FREQ, red * 1000000 / 255);
	gpioHardwarePWM(LED_GREEN_PIN, PWM_FREQ, green * 1000000 / 255);
	gpioPWM(LED_BLUE_PIN, blue);
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
		gpioWrite(LED_CTL_PIN, !(gpioRead(LED_CTL_PIN)));
		printf("blink\n");
		usleep(1000000 / blinkingFreq / 2);
	}
}

int LedBlink(BYTE red, BYTE green, BYTE blue, int freq)
{
	blinkingFreq = freq;
	ledState = LED_BLINK;
	if (ledBlinkingStateRunning == FALSE)
	{
		// create a blinking thread if necessary
		printf("Start Blinking\n");
		pthread_create(&ledBlinkingThread, NULL,(void*)&LedBlinkingProcess, NULL);
		pthread_detach(ledBlinkingThread);
		ledBlinkingStateRunning = TRUE;
	}
	return 0;
}


