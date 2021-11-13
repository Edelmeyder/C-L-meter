#include "seos.h"
#include "rlc.h"
#include "rc.h"
#include "lcd.h"

/* PUBLIC FLAGS */
static volatile uint32_t Flag_measure;

/* PRIVATE */
int seos_init(void);

static volatile uint8_t counterr_measure;

void SysTick_Handler(void) {
	SEOS_Schedule();
}

int seos_init(void)
{
	Flag_measure  = 0;
	//se inicializan los flags y contadores
	counterr_measure = 0;

	//se configura el sistem tick para interrupir una vez cada 100 ms
	if (SysTick_Config(SystemCoreClock / 10)){
		//error handling
	}

	return 0;
}

int SEOS_Boot(void) {
	//inicializa todos los módulos
	LCD_init();
	RLC_init();
	RC_init();
	seos_init();
	return 0;
}

int SEOS_Schedule(void)
{
	//el planificador levanta el flag de las tareas que se tengan que despachar
	if(++counterr_measure == OVERF_MEASURE)
	{
		Flag_measure   = 1; 
		counterr_measure = 0;
	}
	return 0;
}

int SEOS_Dispatch(void)
{
	//el despachador ejecuta las tareas que estén pendientes y baja los flags
	if(Flag_measure) {
		RLC_measure();
		RC_measure();
		Flag_measure = 0;
	}
	return 0;
}

int SEOS_Sleep(void)
{
	//sleep no se implementa en la simulación
	return 0;
}

