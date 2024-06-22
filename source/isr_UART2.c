/* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Ricardo López    11/6/10  UART2C module interrupt handler
* Marcelo Gómez    09/03/14 Portado a MPLAB.X
* Marcelo Gómez    20/05/15 Portado a MPLAB.X
* Sebastián Wahler 04/06/19 Simplificado
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
*
*
**********************************************************************/
#include "p33FJ256GP710.h"

//UART
#define FCY 40000000
#define BAUDRATE 19200
#define BRGVAL ((FCY / BAUDRATE) / 16) -1

volatile unsigned char caracter;
int cant , i, Qty, contador  = 0;   //Cantidad de caracteres a enviar
int esPrimero = 1;
int estado = 0;
unsigned int valor;
volatile unsigned char uart_update;
extern int bufferRX[];
extern int bufferTX[];
extern int terminoRecepcion;

/*---------------------------------------------------------------------
  Function Name: UART2Interrupt
  Description:   UART2 Interrupt Handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
//Interrupciones de la UART2

void __attribute__((interrupt, auto_psv)) _U2RXInterrupt( void )
{
    IFS1bits.U2RXIF = 0;
    
    if (!estado){
        bufferRX[i] = U2RXREG;
        if (bufferRX[i] == 0x00FE){
            estado = 1;
        }
    }
    if (estado){
        //if ((esPrimero) && (bufferRX[i] != 0x00FE)){
        if ((i == 1)){
            bufferRX[i] = U2RXREG;    //Cargo Qty al registro
            //esPrimero = 0;                  //bajo bandera
            Qty = bufferRX[i];       //tomo la cantidad de datos total
        } else {
            if (i < (Qty)){
                bufferRX[i] = U2RXREG;
                }
        }
        i++;
        if(i == Qty){
            //bufferRX[i] = U2RXREG;//
            i = 0;   //resetea valores
            //esPrimero = 1;  //para el proximo
            estado = 0;     //mensaje
            terminoRecepcion = 1; //Bandera Fin Recepcion
        }
    }
}

//Rutina de INT para transmisión (no hace nada)
void __attribute__((interrupt, auto_psv)) _U2TXInterrupt(void)
{
    IFS1bits.U2TXIF = 0;
    if (i == 0){
        Qty = bufferTX[1];//Cantidad de datos a enviar
    }
    if(i < Qty)  // de 0 hasta cantidad de datos
    {
        U2TXREG = bufferTX[i];	//Simple ECO
        i++;
    }
    else{
        //U2TXREG = bufferTX[i];
        i = 0;
        Qty = 0;
    }
}

/*---------------------------------------------------------------------
  Function Name: InitUART2
  Description:   Inicializar UART2
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void InitUART2(void)
{
	// The HPC16 board has a DB9 connector wired to UART2, 
	// so we will be configuring this port only
	// configure U2MODE
	U2MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
	U2MODEbits.RTSMD = 1;	// Bit11 Simplex Mode
    U2MODEbits.PDSEL = 0;   // 8 bits no parity
    
	// Load a value into Baud Rate Generator.  Example is for 19200.
	U2BRG = BRGVAL;	// 40Mhz osc, 19200 Baud

	IPC7 = 0x4400;	// Mid Range Interrupt Priority level, no urgent reason

	IFS1bits.U2RXIF = 0;	// Clear the Recieve Interrupt Flag
	IEC1bits.U2RXIE = 1;	// Enable Recieve Interrupts

	U2MODEbits.UARTEN = 1;	// And turn the peripheral on
	U2STAbits.UTXEN = 1;	// Empieza a transmitir. Se dispara el Flag TXIF

	IFS1bits.U2TXIF = 0;	// Clear the Transmit Interrupt Flag
	IEC1bits.U2TXIE = 1;	// Enable Transmit Interrupts
}