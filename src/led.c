/*
 * led.c
 *
 *  Created on: Jun 17, 2016
 *      Author: ChauNM
 */

#include <wiringPi.h>
#include <string.h>
#include <stdio.h>
#include "led.h"
#include "universal.h"
#include <pthread.h>
#ifdef PI_RUNNING
#include "unistd.h"
#endif

pthread_t ledBlinkingThread;
int blinkingFreq = 0;
int ledState = LED_OFF;
char invertRed = FALSE;
char invertGreen = FALSE;
char ledBlinkingStateRunning;

void LedInit()
{
	//wiringPiSetupGpio();
	wiringPiSetupSys();
	pinMode(LED_RED_PIN, OUTPUT);
	pinMode(LED_GREEN_PIN, OUTPUT);
	digitalWrite(LED_RED_PIN, LOW);
	digitalWrite(LED_GREEN_PIN, HIGH);
	ledState = LED_OFF;
	invertRed = 0;
	invertGreen = 0;
}

void LedTurnOff()
{
	digitalWrite(LED_RED_PIN, LOW);
	digitalWrite(LED_GREEN_PIN, LOW);
	invertRed = FALSE;
	invertGreen = FALSE;
	ledState = LED_OFF;
}

int LedTurnOn(const char* color)
{
	if (strcmp(color, "color.red") == 0)
	{
		digitalWrite(LED_GREEN_PIN, LOW);
		digitalWrite(LED_RED_PIN, HIGH);
		invertRed = FALSE;
		invertGreen = FALSE;
		ledState = LED_ON;
		printf("turn on color %s\n", color);
		return 0;
	}
	else if (strcmp(color, "color.green") == 0)
	{
		digitalWrite(LED_RED_PIN, LOW);
		digitalWrite(LED_GREEN_PIN, HIGH);
		invertRed = FALSE;
		invertGreen = FALSE;
		ledState = LED_ON;
		printf("turn on color %s\n", color);
		return 0;
	}
	else if (strcmp(color, "color.orange") == 0)
	{
		digitalWrite(LED_RED_PIN, LOW);
		digitalWrite(LED_GREEN_PIN, HIGH);
		invertRed = FALSE;
		invertGreen = FALSE;
		ledState = LED_ON;
		printf("turn on color %s\n", color);
		return 0;
	}
	else
	{
		digitalWrite(LED_RED_PIN, LOW);
		digitalWrite(LED_GREEN_PIN, LOW);
		invertRed = FALSE;
		invertGreen = FALSE;
		ledState = LED_OFF;
		return -1;
	}
}

void LedBlinkingProcess()
{
	if (ledState != LED_BLINK)
	{
		ledBlinkingStateRunning = FALSE;
		return;
	}
	int timeCount = 1000000 / blinkingFreq / 2;
	while (1)
	{
		if (ledState != LED_BLINK)
		{
			ledBlinkingStateRunning = FALSE;
			return;
		}
		digitalWrite(LED_RED_PIN, (digitalRead(LED_RED_PIN) ^ invertRed));
		digitalWrite(LED_GREEN_PIN, (digitalRead(LED_GREEN_PIN) ^ invertGreen));
		printf("blink\n");
		usleep(timeCount);
	}
}

int LedBlink(const char* color, int freq)
{
	int result;
	if (strcmp(color, "color.red") == 0)
	{
		blinkingFreq = freq;
		ledState = LED_BLINK;
		invertRed = TRUE;
		invertGreen = FALSE;
		printf("blink color %s\n", color);
		result = 0;
	}
	else if (strcmp(color, "color.green") == 0)
	{
		blinkingFreq = freq;
		ledState = LED_BLINK;
		invertRed = FALSE;
		invertGreen = TRUE;
		result = 0;
		printf("blink color %s\n", color);
	}
	else if (strcmp(color, "color.orange") == 0)
	{
		blinkingFreq = freq;
		ledState = LED_BLINK;
		invertRed = TRUE;
		invertGreen = TRUE;
		printf("blink color %s\n", color);
		result = 0;
	}
	else
	{
		digitalWrite(LED_RED_PIN, LOW);
		digitalWrite(LED_GREEN_PIN, LOW);
		ledState = LED_OFF;
		invertRed = FALSE;
		invertGreen = FALSE;
		result = -1;
	}
	if ((result == 0) && (ledBlinkingStateRunning == FALSE))
	{
		// create a blinking thread if necessary
		printf("Start Blinking\n");
		pthread_create(&ledBlinkingThread, NULL,(void*)&LedBlinkingProcess, NULL);
		pthread_detach(ledBlinkingThread);
		ledBlinkingStateRunning = TRUE;
	}
	return result;
}


