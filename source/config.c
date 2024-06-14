/**********************************************************************
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Ricardo López     03/11/2011  1ra. versión
* Sebastián Wahler  08/05/2013  2da. versión
* Marcelo Gómez     09/03/2014  Portado a MPLAB.X
* Sebastián Wahler  04/06/2019  Simplificado
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
*
**********************************************************************/
#include "p33FJ256GP710.h"
#include "common.h"
#include "config.h"

void config( void )
{
	//Variables para clock del sistema
	hours, minutes, seconds = 0;

	//Inicialización de PORTS I/O
	/* (RA0-RA7) como salidas - LEDs aa*/
	TRISA = 0xFF00; 
		
	/* Initializar UART2*/
	InitUART2();

	/* Inicializar Timers necesarios */
	Init_Timer4();
    Init_Timer6();
    
    /*Todos los pines de todos los puertos como digitales*/
    AD1PCFGH = 0XFFFF;
    AD1PCFGL = 0XFFFF;
    AD2PCFGL = 0XFFFF;
    
    IFS1bits.CNIF = 0; // Reset change notice interrupt flag 
    IPC4bits.CNIP = 6; //Set Change Notice interrupt priority to 6
    CNEN1bits.CN15IE = 1; // pulsador para RD6
    CNEN2bits.CN19IE = 1; // pulsador para RD13
    CNEN2bits.CN16IE = 1; // pulsador para RD7
    IEC1bits.CNIE = 1; //Enable CN interrupts
    TRISD = 0xFFFF; // PORTD como pulsadores
    TRISA = 0x0000; // PORTA como salida
}
