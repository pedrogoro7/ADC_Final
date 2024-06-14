
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

#define TRUE	1
#define MAX 256

extern int contadorEjes; //Contador de ejes general
unsigned int masDeDosEjes; //Contador para autos con mas de dos ejes
extern int velocidad;
unsigned int nAuto;

typedef struct{
    unsigned int hora;
    unsigned int minutos;
    unsigned int segundos;
} time;
unsigned int time;

typedef struct{
    struct time timeStamp ;
    unsigned int velocidad;
    unsigned int cantEjes;       
} dato;
dato dataLogger[MAX];

/*typedef struct{
    unsigned int hora;
    unsigned int minutos;
    unsigned int segundos;
    unsigned int velocidad;
    unsigned int cantEjes;       
} dato;
dato dataLogger[MAX];*/

int counterTog = 0;
int terminoRecepcion;
unsigned int mensajeRecepcion[MAX];
unsigned int mensajeTransmicion[MAX];
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

}

void llenarArreglo()
{
    unsigned int i=0;
    while(i<2){
        i++;
        dataLogger[nAuto].velocidad= 50;
        dataLogger[nAuto].timeStamp.hora=21;
        dataLogger[nAuto].timeStamp.minutos=30;
        dataLogger[nAuto].timeStamp.segundos=10*i;
        dataLogger[nAuto].cantEjes= 1+i;
        nAuto++; //dos vehiculos
        masDeDosEjes = 1; //uno tiene mas de dos ejes
    }
}

void agregarDatos(){
    dataLogger[nAuto].velocidad= velocidad;
    dataLogger[nAuto].timeStamp.hora=hours;
    dataLogger[nAuto].timeStamp.minutos=minutes;
    dataLogger[nAuto].timeStamp.segundos=seconds;
    dataLogger[nAuto].cantEjes= contadorEjes;
    nAuto++;
    if (contadorEjes > 2){
        masDeDosEjes++;
    }   
}

/*---------------------------------------------------------------------
  Function Name: calcularChecksum
  Description:   calcula el checksum
  Precondiciones: cant = C; mensaje[0..C]
  Poscondiciones: checksum = acumulador = A; return A;
-----------------------------------------------------------------------*/
unsigned int calcularChecksum(unsigned int cant, unsigned int mensaje[]){
    unsigned int acumulador =0;
    unsigned int var ;
    unsigned int i= 0;
    
    while (i < (cant-3)){
        var = mensaje[i];//Depositamos lo que esta en la posicion[0,2,4..n] del arreglo
        var = var <<8; //Dezplazamos hacia izquierda; ejemplo: 0x00FE quedaria 0xFE00
        var = var + mensaje[i+1];//sumamos; ejemplo: lo posicionado en [0]+[1] = 0xFE00 + 0x0080 = 0xFE08
        i+=2;
        acumulador+= var;//sumamos y guardamos en acumulador; ejemplo: 0x0000 + 0xFE08 = 0xFE08
    }
    
    if(cant%2 == 1){// Verifico si la cantidad de elementos del mensaje es impar.
        var = mensaje[cant - 3];//tomo el ultimo caracter util del mensaje descartando valosres correspondientes al Checsum
        var = var << 8;//Desplazo 8 bits a la Izquierda
        acumulador += var;//lo sumo al acumulador
    }
    return acumulador;
}
/*---------------------------------------------------------------------
  Function Name: verificarMensaje
  Description:   verifica el mensaje recibido
  Precondiciones: --
  Poscondiciones: msEnviar = MS; return MS; MS puede ser(41,42,43,44,45,46 o 47 si hay algun tipo de error)
-----------------------------------------------------------------------*/
void verificarMensaje(){
    unsigned int check,valor,Qty;
    
    check = calcularChecksum(mensajeRecepcion[1],mensajeRecepcion);
    Qty = mensajeRecepcion[1];
    valor = mensajeRecepcion[Qty - 2];//Parte alta del checksum recibido
    valor = valor << 8;//Desplaza 8 hacia izquierda; ejemplo:0x0081 quedaria 0x8100
    valor = valor + mensajeRecepcion[Qty - 1]; //sumamos parte alta mas parte baja; ejemplo: 0x8000 + 0x004B = 0x084B
    if (check == valor){
        msEnviar =  mensajeRecepcion[5]; //Si es correcto enviamos el argumento VER DE HACERLO CON UNA ESTRUCTURA DE DATOS
    }
    /*if (mensajeRecepcion[1] == Qty){
    
    }
    else{
        valor = mensajeRecepcion[6];//Parte alta del checksum recibido
        valor = valor << 8;//Desplaza 8 hacia izquierda; ejemplo:0x0081 quedaria 0x8100
        valor = valor + mensajeRecepcion[7]; //sumamos parte alta mas parte baja; ejemplo: 0x8000 + 0x004B = 0x084B
    }
    if (check == valor){
        msEnviar =  mensajeRecepcion[5]; //Si es correcto enviamos el argumento
    }
    else{
        msEnviar = 0x0047;//si no enviamos NACK
    }*/

}

void armarMensajeD() {
    unsigned int bccl,bcch= 0;
    unsigned int checksum;//checksum
    unsigned int i = 0;
    unsigned int j = 0;

    mensajeTransmicion[0] = mensajeRecepcion[0];//SOF; 0x00FE
    mensajeTransmicion[2] = mensajeRecepcion[3];//Dts
    mensajeTransmicion[3] = mensajeRecepcion[2];//Src
    mensajeTransmicion[4]= 0x0080;//Sec
    mensajeTransmicion[5]= mensajeRecepcion[5];//Argumentos

    while(i<nAuto){
    	if(mensajeRecepcion[6] == dataLogger[i].timeStamp.hora){
            mensajeTransmicion[6+j] = dataLogger[i].timeStamp.hora;
            mensajeTransmicion[7+j] = dataLogger[i].timeStamp.minutos;
            mensajeTransmicion[8+j] = dataLogger[i].timeStamp.segundos;
            mensajeTransmicion[9+j] = dataLogger[i].velocidad;
            mensajeTransmicion[10+j] = dataLogger[i].cantEjes;
            j = j + 5;
        }
        i++;      
    }
    mensajeTransmicion[1] = 8 + j;//QTY
    checksum = calcularChecksum(mensajeTransmicion[1],mensajeTransmicion);
    bcch = checksum >>8;
    bccl = checksum << 8;
    bccl = bccl >> 8;
    mensajeTransmicion[mensajeTransmicion[1]-2] = bcch;//BCCH
    mensajeTransmicion[mensajeTransmicion[1]-1] = bccl;//BCCL   
}
/*---------------------------------------------------------------------
  Function Name: armarMensaje
  Description:   arma el mensaje a transmitir
  Precondiciones: cant = C; msEnviar = MS;
  Poscondiciones: mensajeTransmicion = MT; MT[0..C]
-----------------------------------------------------------------------*/
void armarMensaje( unsigned int cant , unsigned int msEnviar) {
    unsigned int bccl,bcch= 0;
    unsigned int checksum;
    
    mensajeTransmicion[0] = mensajeRecepcion[0];//SOF; 0x0FE
    mensajeTransmicion[1] = cant;//QTY; 8 o 9
    mensajeTransmicion[2] = mensajeRecepcion[3];//Dts
    mensajeTransmicion[3] = mensajeRecepcion[2];//Src
    mensajeTransmicion[4]= 0x0080;//Sec
    if(cant == 9){
        mensajeTransmicion[5]= mensajeRecepcion[5];//Argumentos
        mensajeTransmicion[6] = msEnviar;//Datos
        checksum = calcularChecksum(9,mensajeTransmicion);
        bcch = checksum >>8;
        bccl = checksum << 8;
        bccl = bccl >> 8;
        mensajeTransmicion[7] = bcch;//BCCH
        mensajeTransmicion[8] = bccl;//BCCL
    }
    else {
        mensajeTransmicion[5]= msEnviar;//Argumentos
        checksum = calcularChecksum(8,mensajeTransmicion);
        bccl = checksum << 8;
        bccl = bccl >> 8;
        checksum = checksum >>8;
        bcch = checksum ;
        mensajeTransmicion[6] = bcch;//BCCH
        mensajeTransmicion[7] = bccl;//BCCL
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
        dataLogger[i].timeStamp.hora= 0;
        dataLogger[i].timeStamp.minutos = 0;
        dataLogger[i].timeStamp.segundos = 0;
        dataLogger[i].cantEjes = 0;
        i++;
    }
    masDeDosEjes = 0;
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
            armarMensaje(9,nAuto);//Consultar cantidad de vehículos hasta el momento.
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'B': 
            resetear();//Resetear la cantidad de vehículos a 0 y borrar todos los registros.
            armarMensaje(8,46);//ACK
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'C': 
            armarMensaje(9,masDeDosEjes);//Consultar cantidad de vehículos con más de dos ejes.
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'D': 
            armarMensajeD();//Consulta detallada de vehículos que pasaron en una determinada hora.
            IFS1bits.U2TXIF = 1; // Interrupt request has occurred
            break;
        case 'E': 
            encenderCamara();//Accionar la cámara fotográfica.
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
