/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"

// TODO: insert other definitions and declarations here

/* Read from serial and write to serial */
static void vReadWriteUART(void *pvParameters) {

	while(1) {
		char character = 255;

		// While loop for reading characters from serial
		character = Board_UARTGetChar(character);
		if (character != 255) {
			Board_UARTPutChar(character);
		}

		vTaskDelay(configTICK_RATE_HZ / 1000);
	}
}

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

int main(void) {

	#if defined (__USE_LPCOPEN)
		// Read clock settings and update SystemCoreClock variable
		SystemCoreClockUpdate();
	#if !defined(NO_BOARD_LIB)
		// Set up and initialize all required blocks and
		// functions related to the board hardware
		Board_Init();
		// Set the LED to the state of "On"
		Board_LED_Set(0, true);
	#endif
	#endif

    xTaskCreate(vReadWriteUART, "vReadWriteUART",
    				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

    return 0 ;
}
