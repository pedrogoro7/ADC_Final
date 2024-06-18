
/**********************************************************************
 FileName:        main.c
 Dependencies:    p33FJ256GP710.h
 Processor:       dsPIC33F
 Compiler:        MPLAB® C30 v2.01 or higher

 Ejemplo de funcionamiento de:
 	Timer1
 	LCD
 	PORT A Output
 	PORT D Input

 REVISION HISTORY:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Author            Date      Comments on this revision
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Richard Fischer   07/14/05  Initial Release
 Priyabrata Sinha  01/27/06  Ported to non-prototype devices 
 Ricardo López     05/24/10  Adaptación para uso general
 Sebastián Wahler  08/05/13  Nuevas adaptaciones
 Marcelo Gómez     09/03/14  Portado a MPLAB.X
 Sebastián Wahler  04/06/19  Simplificado sin Placa

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**********************************************************************/
#include "common.h"
#include <xc.h>
#include "confbits.h"
#include "p33FJ256GP710.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//libreria para sacar hora, minuto y segundos aleatorio

#define TRUE	1
#define MAX 256

extern int contadorEjes; //Contador de ejes general
unsigned int masDosEjes; //Contador para autos con mas de dos ejes
extern int velocidad;
unsigned int nAuto;

typedef struct{
    unsigned int hora;
    unsigned int minutos;
    unsigned int segundos;
    unsigned int velocidad;
    unsigned int cantEjes;       
} dato;
dato dataLogger[MAX];

int counterTog = 0;
int terminoRecepcion;
unsigned int bufferRX[MAX];
unsigned int bufferTX[MAX];
int msEnviar;


void UpdateClock (void)
{
    counterTog = 0;
    /* Actualizar Hora */
    seconds=seconds+1;
    if (seconds > 59)
    {
        seconds=0;
        minutes=minutes+1;
    }
    if (minutes > 59)
    {
        minutes=0;
        hours=hours+1;
    }
    if (hours == 24)
    {
        hours=0;
    }
    //Actualizar Hora Sistema en Display
}

void llenarArreglo()
{
    srand(time(NULL));
    unsigned int i=0;
    while(i<10){
        i++;
        dataLogger[nAuto].velocidad= 20 + rand() % (110 - 20 + 1);//15*i;
        //El operador % (110 - 20 + 1) limita este número al rango de 0 a 90, ya que (110 - 20 + 1) es igual a 91.
        //Al sumar 20 al resultado, ajustamos el rango para que esté entre 20 y 110.
        dataLogger[nAuto].hora = rand() % 25;//21;
        dataLogger[nAuto].minutos = rand() % 61;//30;
        dataLogger[nAuto].segundos = rand() % 61;//10*i;
        dataLogger[nAuto].cantEjes =  rand() % 7;//1+i;
        nAuto++; //dos vehiculos
        masDosEjes = 1; //uno tiene mas de dos ejes
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
        masDosEjes++;
    }   
}

/*---------------------------------------------------------------------
  Function Name: calcularChecksum
  Description:   calcula el checksum
  Precondiciones: cant = C; mensaje[0..C]
  Poscondiciones: checksum = acumulador = A; return A;
-----------------------------------------------------------------------*/
unsigned int calcularChecksum(unsigned int Qty, unsigned int mensaje[]){
    unsigned int checksum =0;
    unsigned int var ;
    unsigned int i= 0;
    
    while (i < (Qty-3)){
        var = mensaje[i];//Depositamos lo que esta en la posicion[0,2,4..n] del arreglo
        var = var <<8; //Dezplazamos hacia izquierda; ejemplo: 0x00FE quedaria 0xFE00
        var = var + mensaje[i+1];//sumamos; ejemplo: lo posicionado en [0]+[1] = 0xFE00 + 0x0080 = 0xFE08
        i+=2;
        checksum+= var;//sumamos y guardamos en acumulador; ejemplo: 0x0000 + 0xFE08 = 0xFE08
    }
    
    if(Qty%2 == 1){// Verifico si la cantidad de elementos del mensaje es impar.
        var = mensaje[Qty - 3];//tomo ultimo caracter util del mensaje descartando valores del Checsum
        var = var << 8;//Desplazo 8 bits a la Izquierda
        checksum += var;//lo sumo al acumulador
    }
    return checksum;
}
/*---------------------------------------------------------------------
  Function Name: verificarMensaje
  Description:   verifica el mensaje recibido
  msEnviar = MS; return MS; MS puede ser(41,42,43,44,45,46 o 47 si hay algun tipo de error)
-----------------------------------------------------------------------*/
void verificarMensaje(){
    unsigned int check,valor,Qty;
    
    check = calcularChecksum(bufferRX[1],bufferRX);
    Qty = bufferRX[1];
    valor = bufferRX[Qty - 2];//Parte alta del checksum recibido
    valor = valor << 8;//Desplaza 8 hacia izquierda; ejemplo:0x0081 quedaria 0x8100
    valor = valor + bufferRX[Qty - 1]; //sumamos parte alta mas parte baja; ejemplo: 0x8000 + 0x004B = 0x084B
    if (check == valor){
        msEnviar =  bufferRX[5]; //Si es correcto enviamos el argumento
    } else {
        msEnviar = 0x0047;//si no enviamos NACK MENSAJE ERRONEO
    }
}

void armarMensajeD() {
    unsigned int chsl,chsh,Qty = 0;//Valor Checksum Alto y Bajo
    unsigned int checksum;//checksum
    unsigned int i = 0;
    unsigned int j = 0;

    bufferTX[0] = bufferRX[0];//SOF; 0x00FE
    bufferTX[2] = bufferRX[3];//Dts
    bufferTX[3] = bufferRX[2];//Src
    bufferTX[4]= 0x0080;//Sec
    bufferTX[5]= bufferRX[5];//Argumentos

    while(i < nAuto){
    	if(bufferRX[6] == dataLogger[i].hora) //Si la hora correponsde a la solicitada cargo bufferTX con los datos a enviar
        {
            bufferTX[6+j] = dataLogger[i].hora;
            bufferTX[7+j] = dataLogger[i].minutos;
            bufferTX[8+j] = dataLogger[i].segundos;
            bufferTX[9+j] = dataLogger[i].velocidad;
            bufferTX[10+j] = dataLogger[i].cantEjes;
            j = j + 5;
        }
        i++;      
    }
    bufferTX[1] = 8 + j;//QTY total del mensaje
    Qty = bufferTX[1];
    checksum = calcularChecksum(bufferTX[1],bufferTX);
    chsh = checksum >> 8;
    chsl = checksum << 8;
    chsl = chsl >> 8;
    bufferTX[Qty-2] = chsh;//Cargo Checsum parte Alta en mensaje
    bufferTX[Qty-1] = chsl;//Cargo Checsum parte Baja en mensaje
}
/*---------------------------------------------------------------------
  Function Name: armarMensaje
  Description:   arma el mensaje a transmitir
  Precondiciones: cant = C; msEnviar = MS;
  Poscondiciones: mensajeTransmicion = MT; MT[0..C]
-----------------------------------------------------------------------*/
void armarMensaje( unsigned int Qty , unsigned int msEnviar) {
    unsigned int chsl,chsh= 0;
    unsigned int checksum;
    
    bufferTX[0] = bufferRX[0];//SOF = 0x0FE
    bufferTX[1] = Qty;//QTY; 8 o 9 depende del comando que se recibio
    bufferTX[2] = bufferRX[3];//Dts
    bufferTX[3] = bufferRX[2];//Src
    bufferTX[4]= 0x0080;//Sec
    if(Qty == 9){
        bufferTX[5]= bufferRX[5];//Argumentos
        bufferTX[6] = msEnviar;//Dato solcitado 
        checksum = calcularChecksum(Qty,bufferTX);
        chsh = checksum >>8;
        chsl = checksum << 8;
        chsl = chsl >> 8;
        bufferTX[7] = chsh;//BCCH
        bufferTX[8] = chsl;//BCCL
    }
    else { //sino no es 9 el Qty tiene que ser 8 para ACK o NACK
        bufferTX[5]= msEnviar;//Argumentos
        checksum = calcularChecksum(Qty,bufferTX);
        chsl = checksum << 8;
        chsl = chsl >> 8;
        checksum = checksum >>8;
        chsh = checksum ;
        bufferTX[6] = chsh;//Cheksum parte Alta
        bufferTX[7] = chsl;//Cheksum parte Baja
    }
}
/*---------------------------------------------------------------------
  Function Name: encenderCamara
  Description:   enciende camara por un milisegundo y la apaga
  Precondiciones: --
  Poscondiciones: --
-----------------------------------------------------------------------*/

void encenderCamara(){
    unsigned int k=0;
    PORTAbits.RA3 = 1;
    while (k<5000){
        k++; //tarda un milisegundo 
    }
    PORTAbits.RA3 = 0;
}
/*---------------------------------------------------------------------
  Function Name: resetear
  Description:   Resetea la cantidad de vehículos a 0 y borrar todos los registros.
  Precondiciones: --
  Poscondiciones: --
-----------------------------------------------------------------------*/
void resetear(){
    unsigned int i = 0;
    while(i < nAuto){
        dataLogger[i].velocidad= 0;
        dataLogger[i].hora= 0;
        dataLogger[i].minutos = 0;
        dataLogger[i].segundos = 0;
        dataLogger[i].cantEjes = 0;
        i++;
    }
    masDosEjes = 0;
    nAuto = 0;
}
/*---------------------------------------------------------------------
  Function Name: procesarMensaje
  Description:   procesa el mensaje y segun el caso ejecuta una funcion
  Precondiciones: msEnviar = MS; MS (41..47)
  Poscondiciones: --
-----------------------------------------------------------------------*/
void procesarMensaje(){
    switch(msEnviar) {
        case 'A': 
            armarMensaje(9,nAuto);//Consultar cantidad de vehÃ­culos hasta el momento.
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'B': 
            resetear();//Resetear la cantidad de vehÃ­culos a 0 y borrar todos los registros.
            armarMensaje(8,46);//ACK
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'C': 
            armarMensaje(9,masDosEjes);//Consultar cantidad de vehÃ­culos con mÃ¡s de dos ejes.
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'D': 
            armarMensajeD();//Consulta detallada de vehÃ­culos que pasaron en una determinada hora.
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'E': 
            encenderCamara();//Accionar la cÃ¡mara fotogrÃ¡fica.
            armarMensaje(8,46);//ACK
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'F': 
            armarMensaje(8,46);//ACK
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'G': 
            armarMensaje(8,msEnviar);//NACK
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
    }
}

int main (void)
{
    config();
    llenarArreglo();
    
    /* Loop infinito */
    while (TRUE)
    {
        if (terminoRecepcion){
            terminoRecepcion = 0;
            verificarMensaje();
            procesarMensaje();
        }
        if(counterTog > 3)
            UpdateClock();  //Actualizar hora del dispositivo
    }
}