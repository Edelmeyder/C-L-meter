#include <stm32f103x6.h>
#include <stdint.h>
#include "seos.h"

int main (void){
	SEOS_Boot(); 	
	while(1) {
		SEOS_Dispatch();
		SEOS_Sleep();
	}
}
