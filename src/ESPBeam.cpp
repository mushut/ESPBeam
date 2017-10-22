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

using namespace std;

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"
#include "ITM_write.h"
#include "user_vcom.h"
#include "GCodeParser.h"
#include "StepperDriver.h"
#include "Semaphore.h"
#include "Servo.h"

#define LIMIT_X 250
#define LIMIT_Y 350

// TODO: insert other definitions and declarations here
struct commandEvent{
	char command[30];
};

QueueHandle_t xQueue = xQueueCreate(10, sizeof(commandEvent));
StepperDriver *stepperDriver;
Servo *servo;

bool isOffLimits = false;

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

}

bool checkCommand(GCommand &cmd) {

	// Check if given command is within the limits
	if(cmd.point.getPointX() <= LIMIT_X &&
			cmd.point.getPointY() <= LIMIT_Y &&
			cmd.point.getPointX() >= 0 &&
			cmd.point.getPointY() >= 0){
		return true;
	}
	return false;
}

/* Public Functions */
void executeCommand(GCommand &cmd) {
	char ok[] = "OK\n";


	switch(cmd.gCodeCommand) {

	// Servo
	case M1:
		if(!isOffLimits) {
			servo->rotate(cmd.penState);
		}
		USB_send((uint8_t *)ok, sizeof(ok));
		vTaskDelay(300);
		break;

		// Initialisation
	case M10:
	{
		char m10[] = "M10 XY 250 350 0.00 0.00 A1 B1 H0 S80 U160 D90\n";
		USB_send((uint8_t *)m10, sizeof(m10));
		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}

	// Stepper
	case G1:
	{

		if(checkCommand(cmd)) {

			//Run the stepper
			stepperDriver->plot(cmd.point);

			if(isOffLimits) {
				isOffLimits = false;
				servo->rotate("90");
				vTaskDelay(300);
			}
		}
		else{
			servo->rotate("160");
			isOffLimits = true;
		}

		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}

	// Some other G-command...
	case G28:
		USB_send((uint8_t *)ok, sizeof(ok));
		break;

		// Reset
	case M4:
		servo->rotate("160");		// Drive pen up
		stepperDriver->reset();
		USB_send((uint8_t *)ok, sizeof(ok));
		break;

		// Default case
	default:
		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}
}
/********************/

/* USB Read -thread */
static void usb_read(void *pvParameters) {

	/* Calibrate stepper */
	servo->rotate("160");		// Drive pen up
	stepperDriver->calibrate(); // Calibrate

	/* Initialise variables */
	char buffer[30] = {0};
	char input[30] = {0};
	commandEvent e;

	uint32_t len = 0;
	uint32_t x = 0;
	uint32_t y = 0;

	/* Infinite loop */
	while(1) {

		/* Get input */
		while(1) {
			len = USB_receive((uint8_t *)buffer, 29);
			buffer[len] = 0;

			// Concatenate buffer to input
			for(x = 0; buffer[x] != 0; x++) {
				if(y < 29){
					input[y++] = buffer[x];
				}
			}

			// Stop reading when '\n' received
			if(buffer[len - 1] == '\n'){
				break;
			}
		}

		// Copy given command to the queue object
		strcpy(e.command, input);

		// Send queue object to queue
		xQueueSendToBack(xQueue, &e, portMAX_DELAY);

		// Reset values
		memset(input, 0, sizeof(input));
		memset(buffer, 0, sizeof(buffer));
		memset(e.command, 0, sizeof(e.command));
		y = 0;
	}
}

/* Stepper driver -thread */
static void stepper_driver(void *pvParameters) {

	/* Initialise */
	commandEvent e;
	GCodeParser parser;
	GCommand command;

	/* Loop */
	while(1) {

		// Try to receive item from queue
		if(xQueueReceive(xQueue, &e, configTICK_RATE_HZ * 3)) {

			// Command execution
			command = *(parser.parseGCode(e.command));	// Parse given command into a Command object
			executeCommand(command);					// Execute given command
		}
		else {
		}

		// Delay
		vTaskDelay(1);
	}
}


int main(void) {

	prvSetupHardware();

	ITM_init();
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	stepperDriver = new StepperDriver();
	servo = new Servo();

	/* Read USB -thread */
	xTaskCreate(usb_read, "usb_read",
			configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);

	/* Stepper driver -thread */
	xTaskCreate(stepper_driver, "stepper_driver",
			configMINIMAL_STACK_SIZE * 7, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);

	/* CDC Task */
	xTaskCreate(cdc_task, "CDC",
			configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
