#include "p33FJ256GP710.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#define ValPR4	39062
#define ValPR6	5000 //1 ms

volatile unsigned char hours;
volatile unsigned char minutes;
volatile unsigned char seconds;
extern int counterTog;

//#define MAX 240
unsigned int contadorEjes = 0; //Contador de ejes general
//unsigned int masDeDosEjes; //Contador para autos con mas de dos ejes
unsigned int velocidad;
//unsigned int nAuto;

unsigned int quantum;
/*
typedef struct{
    unsigned int hora;
    unsigned int minutos;
    unsigned int segundos;
    double velocidad;
    unsigned int cantEjes;       
} dato;
dato dataLogger[MAX];
*/

void prenderTimmer(){
    TMR6 = 0;
    quantum = 1;
	T6CONbits.TON=1;
}

void detenerTimmer(){
    T6CONbits.TON=0;
}

/*---------------------------------------------------------------------
  Function Name: llenarArreglo
  Description:   llena un arreglo
  Precondiciones: --
  Poscondiciones: --
-----------------------------------------------------------------------*/
/*void llenarArreglo()
{
    unsigned int i=0;
    while(i<2){
        i++;
        dataLogger[nAuto].velocidad= 0x002F;
        dataLogger[nAuto].hora=21;
        dataLogger[nAuto].minutos=30;
        dataLogger[nAuto].segundos=10*i;
        dataLogger[nAuto].cantEjes= 1+i;
        nAuto++; //dos vehiculos
        masDeDosEjes = 1; //uno tiene mas de dos ejes
    }
}

void agregarDatos(){
    dataLogger[nAuto].velocidad= velocidad;
    dataLogger[nAuto].hora=hours;
    dataLogger[nAuto].minutos=minutes;
    dataLogger[nAuto].segundos=seconds;
    dataLogger[nAuto].cantEjes= contadorEjes;
    nAuto++;
    if (contadorEjes > 2){
        masDeDosEjes++;
    }   
}
*/
/*---------------------------------------------------------------------
  Function Name: _CNInterrupt
  Description:   CN Interrupt Handler
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _CNInterrupt( void ) {

	IFS1bits.CNIF=0;
    if (( PORTDbits.RD13)&& (PORTDbits.RD6) && (!PORTDbits.RD7)&&(contadorEjes < 1)){
        prenderTimmer();
    }
    if((PORTDbits.RD13)&& (!PORTDbits.RD6)&&(PORTDbits.RD7)&&(contadorEjes == 1)){
            detenerTimmer();
            velocidad = 36 * (30/quantum);
            if (quantum < 18){
                unsigned int k = 0;
                PORTAbits.RA0 = 1;
                while (k<5000){
                    k++; //tarda un milisegundo 
                }
                PORTAbits.RA0 = 0;
            }
    }          
    if((PORTDbits.RD13)&&(PORTDbits.RD6)&&(!PORTDbits.RD7)){
             contadorEjes++;
    }
    if((!PORTDbits.RD13)&&(contadorEjes > 0)){
        agregarDatos();
        contadorEjes = 0;//listo para recibir a un nuevo vehiculo
    }       
}

/*---------------------------------------------------------------------
  Function Name: _T4Interrupt
  Description:   Timer4 Interrupt Handler

Comments: 62500
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _T4Interrupt( void )
{
	/* reset Timer 4 interrupt flag */
    IFS1bits.T4IF = 0;
    counterTog ++;
    /*if (counterTog > 3){
        IFS1bits.U2TXIF = 1; // Interrupt request has occurred
        T4CONbits.TON = 0; // Deshabilito timer
    }*/
}

/*---------------------------------------------------------------------
  Function Name: Init_Timer4
  Description:   Initialize Timer4

-----------------------------------------------------------------------*/
void Init_Timer4( void )
{
	/* ensure Timer 4 is in reset state */
	T4CON = 0;
	T4CONbits.TCKPS = 3; //Prescaler 256

	/* reset Timer 4 interrupt flag */
 	IFS1bits.T4IF = 0;
 	
 	/* set Timer interrupt priority level */
	IPC6bits.T4IP = 5;

	/* enable Timer interrupt */
 	IEC1bits.T4IE = 1;
 	  	
	/* set Timer period register */
	PR4 = ValPR4;
	T4CONbits.TON = 1; 	//habilito Timer

}


/*---------------------------------------------------------------------
  Function Name: _T6Interrupt
  Description:   Timer6 Interrupt Handler

-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _T6Interrupt( void )
{
	/* reset Timer 6 interrupt flag */
 	IFS2bits.T6IF = 0;
	//T6CONbits.TON = 0;	//Deshabilito Timer6
    quantum++;
}

/*---------------------------------------------------------------------
  Function Name: Init_Timer6
  Description:   Initialize Timer6

-----------------------------------------------------------------------*/
void Init_Timer6( void )
{
	
	/* ensure Timer 6 is in reset state */
	T6CON = 0;
	T6CONbits.TCKPS = 1; //Prescaler 8

	/* reset Timer 6 interrupt flag */
 	IFS2bits.T6IF = 0;
 	
 	/* set Timer interrupt priority level */
	IPC11bits.T6IP = 5;

	/* enable Timer interrupt */
 	IEC2bits.T6IE = 1;
 	  	
	/* set Timer period register */
	PR6 = ValPR6;
	T6CONbits.TON = 0; 	//deshabilito Timer

}
