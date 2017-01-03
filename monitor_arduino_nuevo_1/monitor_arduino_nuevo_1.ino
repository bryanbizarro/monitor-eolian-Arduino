/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR PRINCIPAL
 *  PERMITE LEER DATOS DE CAN BUS
 *  LECTURA EN TIEMPO REAL DE:
 *  BMS - KELLYs - MPPT
 */

//Vecotres de envío y variables auxiliares
int BMS_TEMP[20];
int BMS_TEMP_ID_anterior;
int BMS_TEMP_VL_anterior;
int aux_temp=0;

int BMS_VOLT[20];
int BMS_VOLT_ID_anterior;
int BMS_VOLT_VL_anterior;
int aux_volt=0;

int BMS[13];
int BMS_anterior[13];

int KELLY_VI_D[6];
int KELLY_VI_I[6];
int KELLY_RPM_D[2];
int KELLY_RPM_D_anterior[1];
int KELLY_RPM_I[2];
int KELLY_RPM_I_anterior[1];
int KELLY_TEMP_D[4];
int KELLY_TEMP_I[4];
int KELLY_THR_D[1];
int KELLY_THR_I[1];
int KELLY_REV_D[1];
int KELLY_REV_I[1];

int MPPT1[8];
int MPPT2[8];
int MPPT3[8];

//True para enviar| False para ignorar
bool revMode = true;
bool motores = true;
bool serialEngine = true;
bool serialBms = true;

//Librerías
#include <mcp_can.h>
#include <SPI.h>
#include <stdlib.h>

//Boud rate de RED CAN
#define baudRate 115200

//Tiempos de resfresh
#define bmstimi 0
#define timi 256
#define maxTimi 1200

//Receive/Send ID de ambos Kelly
#define recvIdKelly1 0xC8
#define sendIdKelly1 0xD2
#define recvIdKelly2 0xC9
#define sendIdKelly2 0xD3

//Pines Xbee
#define pinRX 10
#define pinTX 11

//Variables conocidas
#define radio 0.550
#define pi 3.141592653

//Tiempo de envío
#define tiempoEnvio 1500


//IDs de request para kellys
unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};  // {Ia,Ib,Ic,Va,Vb,Vc}
unsigned char CPP_MONITOR1[1] = {0x33};         // {PWM,EnableMotorRotation,EngTemp,KellyTemp,highSideFETMOSTemp,lowSideFETMOSTemp}
unsigned char CPP_MONITOR2[1] = {0x37};         // {MSB RPM, LSB RPM, someValue, MSB ERROR CODE, LSB ERROR CODE}
unsigned char COM_SW_ACC[2] = {0x42, 0};
unsigned char COM_SW_BRK[2] = {0x43, 0};
unsigned char COM_SW_REV[2] = {0x44, 0};

//Variable aux de envío/respuesta Kelly
unsigned char engData[2] = {B10000001,B10000001}; 

//Pin de señal del Cna Shield hacia el Arduino |D10 para v0.9b y v1.0 | D9 para v1.1+
const int SPI_CS_PIN = 9;

//Variables para lectura CAN
unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned long canId;
unsigned char buff[8];

/*
// Read2Serial vars
int MPPTId;
char inChar;
int index=0;
char inData[13];
int charsRead = 0;
*/


//Variables de delay
long lastKelly1Time = 0;
long lastKelly2Time = 0;

long lastMpptTime = 0;

long lastEnvioTime_BMS_A = 0;
long lastEnvioTime_BMS_V = 0;
long lastEnvioTime_BMS_T = 0;

long lastEnvioTime_KELLY_a = 0;
long lastEnvioTime_KELLY_b = 0;
long lastEnvioTime_KELLY_c = 0;
long lastEnvioTime_KELLY_d = 0;
long lastEnvioTime_KELLY_e = 0;
long lastEnvioTime_KELLY_f = 0;
long lastEnvioTime_KELLY_g = 0;
long lastEnvioTime_KELLY_h = 0;
long lastEnvioTime_KELLY_i = 0;
long lastEnvioTime_KELLY_j = 0;

long lastEnvioTime_MPPT1 = 0;
long lastEnvioTime_MPPT2 = 0;
long lastEnvioTime_MPPT3 = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{ 
  Serial.begin(baudRate);
  Serial1.begin(baudRate);

  while (Serial1.available() > 0) {
    byte c = Serial1.read();
  }

START_INIT:

  if (CAN_OK == CAN.begin(CAN_1000KBPS)){
    
    Serial.println("CAN BUS Shield BMS-KKELLY iniciado!");
    Serial1.println("CAN BUS Shield BMS-KKELLY iniciado!");
  }else{
    Serial.println("Falla de inicio CAN BUS Shield BMS-KELLY ... Reiniciando");
    Serial1.println("Falla de inicio CAN BUS Shield BMS-KELLY ... Reiniciando");
    delay(100);
    goto START_INIT;
  }
  attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt  
}


void MCP2515_ISR()
{
  flagRecv = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////

//Indica si en un vector de tamaño 20, desde el indice 'i' hasta el final hay ceros
//Se usa para poder rellenar el vector si existen espacios disponibles
bool zeros(int vector[20], int desde){
  int largo =20;

  if (desde>20){
    return false;
  }
  
  while(desde<20){
    if (vector[desde]!=0){
      return false;
    }else{
      desde+=1;     
    }
  }
  return true;
}

//Iguala valor por valor dos vectores de tamaño 13
//Se usa para comparar dos vectores
bool igualdadvectores_13(int vector1[13], int vector2[13]){
    int i=0;
    while(i<13){
      if(vector1[i]==vector2[i]){
        i+=1;
      }else{
        return false;
      }
    }
    return true;
}

//Idica si son iguales dos valores a la vez
//Se usa para filtrar posibles envios de datos repetidos
bool esigual(int id, int valor, int id_a, int valor_a){
    if((id==id_a)&&(valor==valor_a)){
      return true;
    }else{
      return false;
    }
}

//Envia los datos del MPPT en un array de 8
void Send_MPPT1(){
if((millis() - lastEnvioTime_MPPT1) > tiempoEnvio){
  Serial.print("M");
  Serial.print(",");
for(int i=0;i<7;i++){
  Serial.print(MPPT1[i]);
  Serial.print(",");
}
  Serial.print(MPPT1[7]);
  Serial.print("\n");

//Xbee
  Serial1.print("M");
  Serial1.print(",");
for(int i=0;i<7;i++){
  Serial1.print(MPPT1[i]);
  Serial1.print(",");
}
  Serial1.print(MPPT1[7]);
  Serial1.print("\n");

  lastEnvioTime_MPPT1 = millis();  
}
}

//Envia los datos del MPPT en un array de 8
void Send_MPPT2(){
if((millis() - lastEnvioTime_MPPT2) > tiempoEnvio){
  Serial.print("N");
  Serial.print(",");
for(int i=0;i<7;i++){
  Serial.print(MPPT2[i]);
  Serial.print(",");
}
  Serial.print(MPPT2[7]);
  Serial.print("\n");

//Xbee
  Serial1.print("N");
  Serial1.print(",");
for(int i=0;i<7;i++){
  Serial1.print(MPPT2[i]);
  Serial1.print(",");
}
  Serial1.print(MPPT2[7]);
  Serial1.print("\n");

  lastEnvioTime_MPPT2 = millis();  
}
}

//Envia los datos del MPPT en un array de 8
void Send_MPPT3(){
if((millis() - lastEnvioTime_MPPT3) > tiempoEnvio){
  Serial.print("O");
  Serial.print(",");
for(int i=0;i<7;i++){
  Serial.print(MPPT3[i]);
  Serial.print(",");
}
  Serial.print(MPPT3[7]);
  Serial.print("\n");

//Xbee
  Serial1.print("O");
  Serial1.print(",");
for(int i=0;i<7;i++){
  Serial1.print(MPPT3[i]);
  Serial1.print(",");
}
  Serial1.print(MPPT3[7]);
  Serial1.print("\n");

  lastEnvioTime_MPPT3 = millis();  
}
}

//Envía por Serial y Serial1 los datos del BMS
void SendBMS(){
if((millis() - lastEnvioTime_BMS_A) > tiempoEnvio){
if (not(igualdadvectores_13(BMS,BMS_anterior))){
  Serial.print("A");
  Serial.print(",");
for(int i=0;i<12;i++){
  Serial.print(BMS[i]);
  Serial.print(",");
}
  Serial.print(BMS[12]);
  Serial.print("\n");

//Xbee
  Serial1.print("A");
  Serial1.print(",");
for(int i=0;i<12;i++){
  Serial1.print(BMS[i]);
  Serial1.print(",");
}
  Serial1.print(BMS[12]);
  Serial1.print("\n");

  for(int i=0;i<13;i++){
    BMS_anterior[i]=BMS[i];
  }
  lastEnvioTime_BMS_A=millis();
}
}
}

//Envía por Serial y Serial1 las temperaturas del BMS
void Send_BMS_TEMP(){
if((millis() - lastEnvioTime_BMS_T) > tiempoEnvio){
  Serial.print("T");
  Serial.print(",");
for(int i=0;i<19;i++){
  Serial.print(BMS_TEMP[i]);
  Serial.print(",");
}
  Serial.print(BMS_TEMP[19]);
  Serial.print("\n");

//Xbee
  Serial1.print("T");
  Serial1.print(",");
for(int i=0;i<19;i++){
  Serial1.print(BMS_TEMP[i]);
  Serial1.print(",");
}
  Serial1.print(BMS_TEMP[19]);
  Serial1.print("\n");

  lastEnvioTime_BMS_T=millis();
}
}

//Envía por Serial y Serial1 los voltajes del BMS
void Send_BMS_VOLT(){
if((millis() - lastEnvioTime_BMS_V) > tiempoEnvio){
  Serial.print("V");
  Serial.print(",");
for(int i=0;i<19;i++){
  Serial.print(BMS_VOLT[i]);
  Serial.print(",");
}
  Serial.print(BMS_VOLT[19]);
  Serial.print("\n");

//Xbee
  Serial1.print("V");
  Serial1.print(",");
for(int i=0;i<19;i++){
  Serial1.print(BMS_VOLT[i]);
  Serial1.print(",");
}
  Serial1.print(BMS_VOLT[19]);
  Serial1.print("\n");

  lastEnvioTime_BMS_V=millis();  
}
}

//Envía por Serial y Serial1 los voltajes/corrientes del KELLY DERECHO
void SendKELLY_VI_D(){
if((millis() - lastEnvioTime_KELLY_a) > tiempoEnvio){
  Serial.print("a");
  Serial.print(",");
for(int i=0;i<5;i++){
  Serial.print(KELLY_VI_D[i]);
  Serial.print(",");
}
  Serial.print(KELLY_VI_D[5]);
  Serial.print("\n");
  
//Xbee
  Serial1.print("a");
  Serial1.print(",");
for(int i=0;i<5;i++){
  Serial1.print(KELLY_VI_D[i]);
  Serial1.print(",");
}
  Serial1.print(KELLY_VI_D[5]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_a=millis();
}
}

//Envía por Serial y Serial1 los voltajes/corrientes del KELLY IZQUIERDO
void SendKELLY_VI_I(){
if((millis() - lastEnvioTime_KELLY_b) > tiempoEnvio){
  Serial.print("b");
  Serial.print(",");
for(int i=0;i<5;i++){
  Serial.print(KELLY_VI_I[i]);
  Serial.print(",");
}
  Serial.print(KELLY_VI_I[5]);
  Serial.print("\n");

//Xbee
  Serial1.print("b");
  Serial1.print(",");
for(int i=0;i<5;i++){
  Serial1.print(KELLY_VI_I[i]);
  Serial1.print(",");
}
  Serial1.print(KELLY_VI_I[5]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_b=millis();
}
}

//Envía por Serial y Serial1 las RPM/ErrorCode del KELLY DERECHO
void SendKELLY_RPM_D(){
if((millis() - lastEnvioTime_KELLY_c) > tiempoEnvio){
  Serial.print("c");
  Serial.print(",");
  Serial.print(KELLY_RPM_D[0]);
  Serial.print(",");
  Serial.print(KELLY_RPM_D[1]);
  Serial.print("\n");

//Xbee
  Serial1.print("c");
  Serial1.print(",");
  Serial1.print(KELLY_RPM_D[0]);
  Serial1.print(",");
  Serial1.print(KELLY_RPM_D[1]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_c=millis();
}
}

//Envía por Serial y Serial1 las RPM/ErrorCode del KELLY IZQUIERDO
void SendKELLY_RPM_I(){
if((millis() - lastEnvioTime_KELLY_d) > tiempoEnvio){
  Serial.print("d");
  Serial.print(",");
  Serial.print(KELLY_RPM_I[0]);
  Serial.print(",");
  Serial.print(KELLY_RPM_I[1]);
  Serial.print("\n");

//Xbee
  Serial1.print("d");
  Serial1.print(",");
  Serial1.print(KELLY_RPM_I[0]);
  Serial1.print(",");
  Serial1.print(KELLY_RPM_I[1]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_d=millis();
}
}

//Envía por Serial y Serial1 las temperaturas del KELLY DERECHO
void SendKELLY_TEMP_D(){
if((millis() - lastEnvioTime_KELLY_e) > tiempoEnvio){
  Serial.print("e");
  Serial.print(",");
for(int i=0;i<3;i++){
  Serial.print(KELLY_TEMP_D[i]);
  Serial.print(",");
}
  Serial.print(KELLY_TEMP_D[3]);
  Serial.print("\n");

//Xbee
  Serial1.print("e");
  Serial1.print(",");
for(int i=0;i<3;i++){
  Serial1.print(KELLY_TEMP_D[i]);
  Serial1.print(",");
}
  Serial1.print(KELLY_TEMP_D[3]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_e=millis();
}
}

//Envía por Serial y Serial1 las temperaturas del KELLY IZQUIERDO
void SendKELLY_TEMP_I(){
if((millis() - lastEnvioTime_KELLY_f) > tiempoEnvio){
  Serial.print("f");
  Serial.print(",");
for(int i=0;i<3;i++){
  Serial.print(KELLY_TEMP_I[i]);
  Serial.print(",");
}
  Serial.print(KELLY_TEMP_I[3]);
  Serial.print("\n");

//Xbee
  Serial1.print("f");
  Serial1.print(",");
for(int i=0;i<3;i++){
  Serial1.print(KELLY_TEMP_I[i]);
  Serial1.print(",");
}
  Serial1.print(KELLY_TEMP_I[3]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_f=millis();
}
}

//Envía por Serial y Serial1 el Throttle Status del KELLY DERECHO
void SendKELLY_THR_D(){
if((millis() - lastEnvioTime_KELLY_g) > tiempoEnvio){
  Serial.print("g");
  Serial.print(",");
  Serial.print(KELLY_THR_D[0]);
  Serial.print("\n");

//Xbee
  Serial1.print("g");
  Serial1.print(",");
  Serial1.print(KELLY_THR_D[0]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_g=millis();
}
}

//Envía por Serial y Serial1 el Throttle Status del KELLY IZQUIERDO
void SendKELLY_THR_I(){
if((millis() - lastEnvioTime_KELLY_h) > tiempoEnvio){
  Serial.print("h");
  Serial.print(",");
  Serial.print(KELLY_THR_I[0]);
  Serial.print("\n");

//Xbee
  Serial1.print("h");
  Serial1.print(",");
  Serial1.print(KELLY_THR_I[0]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_h=millis();
}
}

//Envía por Serial y Serial1 el Reverse Status del KELLY DERECHO
void SendKELLY_REV_D(){
if((millis() - lastEnvioTime_KELLY_i) > tiempoEnvio){
  Serial.print("i");
  Serial.print(",");
  Serial.print(KELLY_REV_D[0]);
  Serial.print("\n");

//Xbee
  Serial1.print("i");
  Serial1.print(",");
  Serial1.print(KELLY_REV_D[0]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_i=millis();
}
}

//Envía por Serial y Serial1 el Reverse Status del KELLY IZQUIERDO
void SendKELLY_REV_I(){
if((millis() - lastEnvioTime_KELLY_j) > tiempoEnvio){
  Serial.print("j");
  Serial.print(",");
  Serial.print(KELLY_REV_I[0]);
  Serial.print("\n");

//Xbee
  Serial1.print("j");
  Serial1.print(",");
  Serial1.print(KELLY_REV_I[0]);
  Serial1.print("\n");

  lastEnvioTime_KELLY_j=millis();
}
}

void loop()
{

/*////////////////////   BLOQUE DE REQUEST KELLYS   ////////////////////*/
  
  if(motores){
    if(millis() - lastKelly1Time > maxTimi){
      engData[0] = B10000001;
      lastKelly1Time = millis();
    }
    if(millis() - lastKelly2Time > maxTimi){
      engData[1] = B10000001; 
      lastKelly2Time = millis();
    }
    //Serial1.print("kelly1 ");Serial1.print(millis() - lastKelly1Time);Serial1.print("  kelly2 ");Serial1.print(millis() - lastKelly2Time);Serial1.print("  engineData ");Serial1.println(engineData);
    
    if(((millis() - lastKelly1Time) > timi) && bitRead(engData[0],7)){
      if((B1111111&engData[0]) == 1){
        CAN.sendMsgBuf(recvIdKelly1, 0, 1, CCP_A2D_BATCH_READ2);
      } else if ((B1111111&engData[0]) == 2){
        CAN.sendMsgBuf(recvIdKelly1, 0, 1, CPP_MONITOR2);
      } else if ((B1111111&engData[0]) == 3){
        CAN.sendMsgBuf(recvIdKelly1, 0, 1, CPP_MONITOR1);
      } else if (((B1111111&engData[0]) == 4) && (revMode == true)){
        CAN.sendMsgBuf(recvIdKelly1, 0, 2, COM_SW_ACC);
      } else if (((B1111111&engData[0]) == 5) && (revMode == true)){
        CAN.sendMsgBuf(recvIdKelly1, 0, 2, COM_SW_REV);
      }
      bitWrite(engData[0],7,0);
      lastKelly1Time = millis();
    }
  
    if(((millis() - lastKelly2Time) > timi) && bitRead(engData[1],7)){
      if((B1111111&engData[1]) == 1){
        CAN.sendMsgBuf(recvIdKelly2, 0, 1, CCP_A2D_BATCH_READ2);
      } else if ((B1111111&engData[1]) == 2){
        CAN.sendMsgBuf(recvIdKelly2, 0, 1, CPP_MONITOR2);
      } else if ((B1111111&engData[1]) == 3){
        CAN.sendMsgBuf(recvIdKelly2, 0, 1, CPP_MONITOR1);
      } else if (((B1111111&engData[1]) == 4) && (revMode == true)){
        CAN.sendMsgBuf(recvIdKelly2, 0, 2, COM_SW_ACC);
      } else if (((B1111111&engData[1]) == 5) && (revMode == true)){
        CAN.sendMsgBuf(recvIdKelly2, 0, 2, COM_SW_REV);
      }
      bitWrite(engData[1],7,0);
      lastKelly2Time = millis();
    }
  }

/*////////////////////   FIN REQUEST KELLYS   ////////////////////*/

  if(CAN.checkReceive()){
    CAN.readMsgBuf(&len, buff);
    unsigned long canId = CAN.getCanId();

/*////////////////////   LECTURA DE DATOS DE BMS   ////////////////////*/
  
    if (canId == 0x036){
      
        int cellID = (buff[0]);
        int instVolt = (buff[1]<<8)|buff[2];
//        int internalResist =  (buff[3]<<8)|buff[4];
//        int openVolt =  (buff[5]<<8)|buff[6];

        if (not(esigual(cellID,instVolt,BMS_VOLT_ID_anterior,BMS_VOLT_VL_anterior))){
          if (zeros(BMS_VOLT,aux_volt)){
            BMS_VOLT[aux_volt] = cellID;
            BMS_VOLT[aux_volt+1] = abs(instVolt);
            aux_volt+=2;
            BMS_VOLT_ID_anterior = cellID;
            BMS_VOLT_VL_anterior = abs(instVolt);
          
          }else{
            Send_BMS_VOLT();
            memset(BMS_VOLT, 0, sizeof(BMS_VOLT));
            aux_volt=0;
          }
        }
        
    }else if (canId == 0x081){ // Lectura de temperaturas
      int thermistorID = buff[0];
      int temperature = buff[1];

      if (not(esigual(thermistorID,temperature,BMS_TEMP_ID_anterior,BMS_TEMP_VL_anterior))){
        if (zeros(BMS_TEMP,aux_temp)){
          BMS_TEMP[aux_temp] = thermistorID;
          BMS_TEMP[aux_temp+1] = temperature;
          aux_temp+=2;
          BMS_TEMP_ID_anterior = thermistorID;
          BMS_TEMP_VL_anterior = temperature;
        
        }else{
          Send_BMS_TEMP();
          memset(BMS_TEMP, 0, sizeof(BMS_TEMP));
          aux_temp=0;
        }
      }
    }else if( canId == 0x100){
      
      int packSOC         = buff[0];
      int packCurrent     = (buff[1]<<8)|buff[2];
      int packInstVolt    = (buff[3]<<8)|buff[4];
      int packOpenVolt    = (buff[5]<<8)|buff[6];

        BMS[0]= packSOC;
        BMS[1]= packCurrent;
        BMS[2]= packInstVolt;
        BMS[3]= packOpenVolt;
        SendBMS();
        
    }else if( canId == 0x101){
      
      unsigned int packAbsCurrent  = (buff[0]<<8)|buff[1];
      unsigned int maximumPackVolt = (buff[2]<<8)|buff[3];
      unsigned int minimumPackVolt = (buff[4]<<8)|buff[5];

      BMS[4]= packAbsCurrent;
      BMS[5]= maximumPackVolt;
      BMS[6]= minimumPackVolt;
      SendBMS(); 
    }else if( canId == 0x102){
      
      int highTemperature   = buff[0];
      int highThermistorID  = buff[1];
      int lowTemperature    = buff[2];
      int lowThermistorID   = buff[3];
      int avgTemp           = buff[4];
      int internalTemp      = buff[5];

      BMS[7]= highTemperature;
      BMS[8]= highThermistorID;
      BMS[9]= lowTemperature;
      BMS[10]= lowThermistorID ;
      BMS[11]= avgTemp;
      BMS[12]= internalTemp;
      SendBMS();
        
    }
/*////////////////////   FIN BMS | LECTURA DE DATOS KELLYS   ////////////////////*/
    else if((canId == sendIdKelly1) && !bitRead(engData[0],7)){

      if((B1111111&engData[0]) == 1){ 

        KELLY_VI_D[0]= (buff[0],DEC);     // IA DER
        KELLY_VI_D[1]= (buff[1],DEC);     // IB DER
        KELLY_VI_D[2]= (buff[2],DEC);     // IC DER
        KELLY_VI_D[3]= (buff[3]/1.84);    // VA DER
        KELLY_VI_D[4]= (buff[4]/1.84);    // VB DER
        KELLY_VI_D[5]= (buff[5]/1.84);    // VC DER

        SendKELLY_VI_D(); 
        engData[0] += 1;
        
      }else if((B1111111&engData[0]) == 2){  // Si el 1er digito de engineData es '2' se procede a leer RPM.

        KELLY_RPM_D[0]= ((buff[0])<<8|buff[1]);   // RPM DER
        KELLY_RPM_D[1]= ((buff[3])<<8|buff[4]);   // ERROR CODE DER

        if (not((KELLY_RPM_D[0] > 80)&&(abs(KELLY_RPM_D[0]-KELLY_RPM_D_anterior[0])>60))){
        SendKELLY_RPM_D();
        KELLY_RPM_D_anterior[0] = KELLY_RPM_D[0];
        }
        engData[0] += 1;
        
      }else if((B1111111&engData[0]) == 3){                        // Si el 1er digito de engineData es '3' se procede a leer la temperatura.

        KELLY_TEMP_D[0]= buff[0];     //PWM DER
        KELLY_TEMP_D[1]= buff[1];     //EMR DER
        KELLY_TEMP_D[2]= buff[2];     //MOTOR TEMP DER
        KELLY_TEMP_D[3]= buff[3];     //KELLY TEMP DER
        SendKELLY_TEMP_D();
        
        if(revMode){ 
          engData[0] += 1;
        }else{
          engData[0] = 1;
        }
      
      }else if((B1111111&engData[0]) == 4){ 

        KELLY_THR_D[0]=buff[0];   //THROTLE SWITCH STATUS DER
        SendKELLY_THR_D();
        
        engData[0] += 1;
        
      }else if((B1111111&engData[0]) == 5){     

        KELLY_REV_D[0]=buff[0];   //REVERSE SWITCH STATUS DER
        SendKELLY_REV_D();
       
        engData[0] = 1;
        
      }
      bitWrite(engData[0],7,1);
      
    }else if((canId == sendIdKelly2) && !bitRead(engData[1],7)){

      if((B1111111&engData[1]) == 1){         

        KELLY_VI_I[0]= (buff[0],DEC);   //IA IZ
        KELLY_VI_I[1]= (buff[1],DEC);   //IB IZ
        KELLY_VI_I[2]= (buff[2],DEC);   //IC IZ
        KELLY_VI_I[3]= (buff[3]/1.84);  //VA IZ
        KELLY_VI_I[4]= (buff[4]/1.84);  //VB IZ
        KELLY_VI_I[5]= (buff[5]/1.84);  //VC IZ

        SendKELLY_VI_I();

        engData[1] += 1;
        
      }else if((B1111111&engData[1]) == 2){  // 2do digito de engineData es 2, lee RPM.

        KELLY_RPM_I[0]= ((buff[0])<<8|buff[1]); //RPM IZ
        KELLY_RPM_I[1]= ((buff[3])<<8|buff[4]); //ERROR CODE IZ
        
        if (not((KELLY_RPM_I[0] > 80)&&(abs(KELLY_RPM_I[0]-KELLY_RPM_I_anterior[0])>60))){
        SendKELLY_RPM_I();
        KELLY_RPM_I_anterior[0] = KELLY_RPM_I[0];
        }
        
        engData[1] += 1;
        
      }else if((B1111111&engData[1]) == 3){                        // 2do digito de engineData es 3, lee temperatura.
        
        KELLY_TEMP_I[0]= buff[0];   //PWM IZ
        KELLY_TEMP_I[1]= buff[1];   //EMR IZ
        KELLY_TEMP_I[2]= buff[2];   //MOTOR TEMP IZ
        KELLY_TEMP_I[3]= buff[3];   //KELLY TEMP IZ
        SendKELLY_TEMP_I();  
      
        if(revMode) { 
          engData[1] += 1;
        } else {
          engData[1] = 1;
        }
        
      }else if((B1111111&engData[1]) == 4){  
  
        KELLY_THR_I[0]=buff[0];   //THROTLE SWITCH STATUS IZ
        SendKELLY_THR_I();
        
        engData[1] += 1;
        
      }else if((B1111111&engData[1]) == 5){     

        KELLY_REV_I[0]=buff[0]; //REVERSE SWITCH STATUS IZ
        SendKELLY_REV_I();
        
        engData[1] = 1;
        
      } 
      bitWrite(engData[1],7,1);
    }
  }

/*////////////////////   FIN KELLYS   ////////////////////*/

/*////////////////////   LECTURA DE DATOS MPPTs   ////////////////////*/
  
  else if(canId = 0x771){
    
    MPPT1[0] = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];   //U_IN_1
    MPPT1[1] = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];   //I_IN_1
    MPPT1[2] = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];   //U_OUT_1
    MPPT1[3] = bitRead(buff[0],7);                                        //BVLR_1
    MPPT1[4] = bitRead(buff[0],6);                                        //OVT_1
    MPPT1[5] = bitRead(buff[0],5);                                        //NOC_1
    MPPT1[6] = bitRead(buff[0],5);                                        //UNDV_1
    MPPT1[7] = buff[6];                                                   //TEMP_1

    Send_MPPT1();
  }
  else if(canId = 0x772){
    
    MPPT2[0] = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];   //U_IN_2
    MPPT2[1] = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];   //I_IN_2
    MPPT2[2] = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];   //U_OUT_2
    MPPT2[3] = bitRead(buff[0],7);                                        //BVLR_2
    MPPT2[4] = bitRead(buff[0],6);                                        //OVT_2
    MPPT2[5] = bitRead(buff[0],5);                                        //NOC_2
    MPPT2[6] = bitRead(buff[0],5);                                        //UNDV_2
    MPPT2[7] = buff[6];                                                   //TEMP_2

    Send_MPPT2();
  }
  else if (canId = 0x773){
    
    MPPT1[0] = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];   //U_IN_3
    MPPT1[1] = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];   //I_IN_3
    MPPT1[2] = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];   //U_OUT_3
    MPPT1[3] = bitRead(buff[0],7);                                        //BVLR_3
    MPPT1[4] = bitRead(buff[0],6);                                        //OVT_3
    MPPT1[5] = bitRead(buff[0],5);                                        //NOC_3
    MPPT1[6] = bitRead(buff[0],5);                                        //UNDV_3
    MPPT1[7] = buff[6];                                                   //TEMP_3

    Send_MPPT3();
  }

/*////////////////////   BLOQUE DE REQUEST MPPTs   ////////////////////*/
/*
  if((millis() - lastMpptTime) > 1024){
    CAN.sendMsgBuf(0x711, 0, 0, 0);
    CAN.sendMsgBuf(0x712, 0, 0, 0);
    lastMpptTime = millis();
  }

/*////////////////////   FIN REQUEST MPPTs   ////////////////////*/

//Limpieza de Puertos Seriales
  Serial.flush();
  Serial1.flush();

//Uso para lectura MPPT
/*
  if(charsRead > 12){
    charsRead = 0;
  }
*/

//Fin Loop
} 


