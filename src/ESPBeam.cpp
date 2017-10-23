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

/* Includes */
#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ITM_write.h"
#include "user_vcom.h"
#include "GCodeParser.h"
#include "StepperDriver.h"
#include "Semaphore.h"
#include "Servo.h"
#include "string.h"

/* Macros */
#define LIMIT_X 250
#define LIMIT_Y 350

/* Global Variables */
struct commandEvent{
	char input[30];
};

QueueHandle_t xQueue = xQueueCreate(10, sizeof(commandEvent));
StepperDriver *stepperDriver;
Servo *servo;
bool isOffLimits = false;
bool isCalibrating = false;
char penUp[5] = "160";
char penDown[5] = "90";


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


/* Public Functions */
/* Check command validity */
bool checkCommand(GCommand &cmd) {

	if(cmd.point.getPointX() <= LIMIT_X &&
			cmd.point.getPointY() <= LIMIT_Y &&
			cmd.point.getPointX() >= 0 &&
			cmd.point.getPointY() >= 0){
		return true;
	}
	return false;
}

/* Execute command */
void executeCommand(GCommand &cmd) {
	char ok[] = "OK\n";


	switch(cmd.gCodeCommand) {

	// Servo
	case M1:
	{
		if(!isOffLimits) {
			servo->rotate(cmd.penState);
		}
		USB_send((uint8_t *)ok, sizeof(ok));
		vTaskDelay(300);
		break;
	}

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
		// Check if given coordinates are within limits
		if(checkCommand(cmd)) {

			//Run the stepper
			stepperDriver->plot(cmd.point);

			// Check flag
			if(isOffLimits) {
				isOffLimits = false;	// Reset flag
				servo->rotate(penDown); // Drive pen down
				vTaskDelay(300);		// Delay
			}
		}
		else{
			servo->rotate(penUp);		// Drive pen up
			isOffLimits = true;			// Set flag
		}

		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}

	// Some other G-command...
	case G28:
	{
		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}

	// Laser *NOT COMPLETE*
	case M4:
	{
		//		servo->rotate("160");
		//		stepperDriver->reset();
		//		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}

	// Default case
	default:
		USB_send((uint8_t *)ok, sizeof(ok));
		break;
	}
}
/********************/


/* Read Command -Task*/
static void read_task(void *pvParameters) {

	/* Calibrate stepper */
	isCalibrating = true;
	servo->rotate(penUp);		// Drive pen up
	stepperDriver->calibrate(); // Calibrate
	isCalibrating = false;

	/* Initialise variables */
	char buffer[30] = {0};
	char input[30] = {0};
	commandEvent e;

	uint32_t len = 0;
	uint32_t x = 0;
	uint32_t y = 0;

	bool breakFlag = false;

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

			// Check if line break received
			for(x = 0; input[x] != 0; x++) {
				if(input[x] == '\n') {
					breakFlag = true;
				}
			}

			// Break if line break received
			if(breakFlag) break;
		}

		// Copy given command to the queue object
		strcpy(e.input, input);

		// Send queue object to queue
		xQueueSendToBack(xQueue, &e, portMAX_DELAY);

		// Reset values
		memset(input, 0, sizeof(input));
		memset(buffer, 0, sizeof(buffer));
		memset(e.input, 0, sizeof(e.input));
		y = 0;
		breakFlag = false;
	}
}

/* Execute Command -Task */
static void execute_task(void *pvParameters) {

	/* Initialise */
	commandEvent e;
	GCodeParser parser;
	GCommand command;

	/* Loop */
	while(1) {

		// Try to receive item from queue for a second
		if(xQueueReceive(xQueue, &e, configTICK_RATE_HZ * 3)) {

			// Command execution
			command = *(parser.parseGCode(e.input));	// Parse given command into a Command object
			executeCommand(command);					// Execute given command
		}

		else {
			// Reset plotter position if no commands received for a second
			if(!isCalibrating){
				stepperDriver->reset();
			}
		}

		// Delay
		vTaskDelay(1);
	}
}

/* Main */
int main(void) {

	/* Initialisation */
	prvSetupHardware();
	ITM_init();
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
	stepperDriver = new StepperDriver();
	servo = new Servo();

	/* Read Command -Task*/
	xTaskCreate(read_task, "read_task",
			configMINIMAL_STACK_SIZE * 3, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);

	/* Execute Command -Task */
	xTaskCreate(execute_task, "execute_task",
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
