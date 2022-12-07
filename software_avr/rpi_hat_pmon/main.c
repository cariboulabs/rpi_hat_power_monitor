#include <atmel_start.h>
#include <app/app.h>

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	/* Replace with your application code */
	app();
	while (1) 
	{
	}
}
