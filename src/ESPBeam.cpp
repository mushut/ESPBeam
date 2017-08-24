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
		// While loop for reading characters from serial
		while ((character = Board_UARTGetChar()) != 255) {
					Board_UARTPutChar(character);
		}
	}
}

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
