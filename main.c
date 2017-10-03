///////////////////////////////////////////////////////////
//   IOTask v1.0                                         //
//   ===========                                         //
//   Copyright© 2017 by Felix Knobl, FH Technikum Wien   //
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "GPIO.h"

//#define DEBUG

#define APPMODES_COUNT		2
#define RUNMODES_COUNT		4
#define LED_ACTION_DELAY	100

#define BUTTON1_PRESSED		(P1_14_read() == 0)
#define BUTTON1_RELEASED	(P1_14_read() == (1 << 14)) // 0x00004000UL

#define BUTTON2_PRESSED		(P1_15_read() == 0)
#define BUTTON2_RELEASED	(P1_15_read() == (1 << 15)) // 0x00008000UL

#define LED1ON 				(PORT1->OMR = 0x00000002UL)
#define LED1OFF 			(PORT1->OMR = 0x00020000UL)

#define LED2ON 				(PORT1->OMR = 0x00000001UL)
#define LED2OFF 			(PORT1->OMR = 0x00010000UL)

typedef enum AppMode
{
	RUN = 0,
	CONTROL
} AppMode;

typedef enum RunMode
{
	LED1BLINK = 0,
	LED2BLINK,
	LED12BLINK,
	LED12ABLINK

} RunMode;

typedef enum ButtonState
{
	RELEASED = 0,
	PRESSED,
	DONE
} ButtonState;

AppMode appMode = RUN;
RunMode runMode = LED1BLINK;

ButtonState button1State = RELEASED;
ButtonState button2State = RELEASED;

uint8_t ledCounter = 0;
bool ledFirstStep = true;

int main(void)
{
	// Configure LED pins
	P1_0_set_mode(OUTPUT_PP_GP);
	P1_0_set_driver_strength(STRONG);
	P1_0_reset();

	P1_1_set_mode(OUTPUT_PP_GP);
	P1_1_set_driver_strength(STRONG);
	P1_1_reset();

	// Configure button pins
	P1_14_set_mode(INPUT);
	P1_15_set_mode(INPUT);

	// Configure System Timer interrupt each 10ms
	SysTick_Config(SystemCoreClock / 100);

	// Endless loop
	while (true)
	{
		if (appMode == RUN)
		{
			// App is in RUN mode
			if (ledCounter < LED_ACTION_DELAY)
			{
				continue;
			}

			// LED counter timeout -> Reset LED counter
			ledCounter = 0;

			// Check current run mode and perform specified animation
			switch (runMode)
			{
				case LED1BLINK:
					if (ledFirstStep)
					{
						LED1ON;
						LED2OFF;
					}
					else
					{
						LED1OFF;
						LED2OFF;
					}

					break;

				case LED2BLINK:
					if (ledFirstStep)
					{
						LED2ON;
						LED1OFF;
					}
					else
					{
						LED2OFF;
						LED1OFF;
					}

					break;

				case LED12BLINK:
					if (ledFirstStep)
					{
						LED1ON;
						LED2ON;
					}
					else
					{
						LED1OFF;
						LED2OFF;
					}

					break;

				case LED12ABLINK:
					if (ledFirstStep)
					{
						LED1ON;
						LED2OFF;
					}
					else
					{
						LED1OFF;
						LED2ON;
					}

					break;

				default:
					break;

			}

			// Change animation step from 0 to 1 or from 1 to 0
			ledFirstStep = !ledFirstStep;

		}
		else if (appMode == CONTROL)
		{
			// App is in CONTROL mode
			LED1OFF;
			LED2OFF;

			// Turn LED1 on if bit 0 of PORT1 is set
			if (runMode & 1)
			{
				LED1ON;
			}

			// Turn LED1 on if bit 1 of PORT1 is set
			if (runMode & 2)
			{
				LED2ON;
			}
		}
	}

	return 0;
}

void processButtons()
{
	// Debouncing and processing BUTTON1
	// (Changes between different RUN-modes)
	if (BUTTON1_PRESSED)
	{
		if (button1State == RELEASED)
		{
			button1State = PRESSED;
		}
		else if (button1State == PRESSED)
		{
			// Action of the button
			if (appMode == CONTROL)
			{
				runMode = (runMode + 1) % RUNMODES_COUNT;
			}

			// Marks that the button action is done,
			// if the user continues holding the button pressed
			button1State = DONE;
		}
	}
	else
	{
		button1State = RELEASED;
	}

	// Debouncing and processing BUTTON2
	// (Changes appMode between RUN-mode and Control-mode)
	if (BUTTON2_PRESSED)
	{
		if (button2State == RELEASED)
		{
			button2State = PRESSED;
		}
		else if (button2State == PRESSED)
		{
			// Action of the button
			appMode = (appMode + 1) % APPMODES_COUNT;

			// Reset animation
			ledCounter = LED_ACTION_DELAY;
			ledFirstStep = true;

			// Marks that the button action is done,
			// if the user continues holding the button pressed
			button2State = DONE;
		}
	}
	else
	{
		button2State = RELEASED;
	}

}

void SysTick_Handler(void)
{
	// Increment the led time out counter
	ledCounter++;

	// Debounce and process the buttons
	processButtons();
}
