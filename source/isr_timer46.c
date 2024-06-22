#include "p33FJ256GP710.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#define ValPR1	39062
#define ValPR4	39062
#define ValPR6	5000 //1 ms
#define MAXK	5000

volatile unsigned char hours;
volatile unsigned char minutes;
volatile unsigned char seconds;
extern int counterTog;

unsigned int contadorEjes = 0; //Contador de ejes general
unsigned int velocidad, quantum;

void prenderTimmer(){
    TMR6 = 0;
    quantum = 1;
	T6CONbits.TON=1;
}

void detenerTimmer(){
    T6CONbits.TON=0;
}

/*---------------------------------------------------------------------
  Function Name: _CNInterrupt
  Description:   CN Interrupt Handler
-----------------------------------------------------------------------*/
void __attribute__((interrupt, auto_psv)) _CNInterrupt( void ) {

	IFS1bits.CNIF=0;
    if (( PORTDbits.RD13)&& (PORTDbits.RD6) && (!PORTDbits.RD7)&&(contadorEjes < 1)){
        prenderTimmer();
        //contadorEjes++;
    }
    if((PORTDbits.RD13)&& (!PORTDbits.RD6)&&(PORTDbits.RD7)&&(contadorEjes == 1)){
            detenerTimmer();
            velocidad = 36 * (40/quantum);// 1m/s = 36km/h se multiplica por 36 para convertir de ms a km
            if (velocidad > 60){ //Excede 60KM/H?
                PORTAbits.RA0 = 1;
                for (unsigned int k = 0; k < MAXK; k++);
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
void Init_Timer1( void )
{
	/* ensure Timer 1 is in reset state */
	T1CON = 0;
	T1CONbits.TCKPS = 3; //Prescaler 256
	/* reset Timer 4 interrupt flag */
 	IFS0bits.T1IF = 0;
 	/* set Timer interrupt priority level */
	IPC0bits.T1IP = 4;
	/* enable Timer interrupt */
 	IEC0bits.T1IE = 1;
	/* set Timer period register */
	PR1 = ValPR1;
}
void __attribute__((interrupt, auto_psv)) _T1Interrupt( void )
{
	/* Timer interrumpe cada 0.25 seg */
 	IFS0bits.T1IF = 0;
	T1CONbits.TON = 0; 	//Apago Timer
    TMR1 = 0;
    //flag_delay = 0;
}