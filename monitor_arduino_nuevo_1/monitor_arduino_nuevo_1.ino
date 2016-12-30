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

// MPPTs vars

unsigned int Uin, Iin, Uout;


//Variables de delay
long lastKelly1Time = 0;
long lastKelly2Time = 0;
long last_BMS_VoltageTime = 0;

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

///////////////////////////////////////////////////////////////////

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


//bool read2Serial(){
//  while(mySerial1.available() || (charsRead < 13)){  //Revisar si es necesario un timeout
//    //Serial1.println("SerialIsAvailable");
//    if(index < 12){
//      //Serial1.print(index);Serial1.println(" < 13");
//      inChar = mySerial1.read();
//      inData[index] = inChar;
//      index++;
//      //Serial1.print("inChar: ");Serial1.println(inChar);
//    } else {
//      inChar = mySerial1.read();
//      inData[index] = inChar;
//      index++;
//      //Serial1.print("inChar: ");Serial1.println(inChar);
//      //Serial1.println("buff full");
//      //Serial1.print(inData[0]);Serial1.print(inData[1]);Serial1.print(inData[2]);Serial1.print(inData[3]);
//      //Serial1.print(inData[4]);Serial1.print(inData[5]);Serial1.print(inData[6]);Serial1.print(inData[7]);
//      //Serial1.print(inData[8]);Serial1.print(inData[9]);Serial1.print(inData[10]);Serial1.print(inData[11]);
//      //Serial1.println(inData[12]);
//      if((inData[0] == 255) && (inData[1] == 255) && (inData[4] == 255) && (inData[13] == 255)){
//        //Serial1.println("data ok");
//        if(inData[2] == 0){  //REVISAR que igualdad se cumpla!!!
//          MPPTId = inData[3];
//          index = 0;
//          for(int j = 5;j<13;j++){
//            buff[index] = inData[j];
//          }
//          int MPPT_TEMP  = buff[6];
//          int  Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
//          int  Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
//          int Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
//          int BVLR = (bitRead(buff[0],7));
//          int OVT  = (bitRead(buff[0],6));
//          int NOC  = (bitRead(buff[0],5));
//          int UNDV = (bitRead(buff[0],4));
//  
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_BVLR,");Serial1.print(BVLR);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_OVT,");Serial1.print(OVT);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_NOC,");Serial1.print(NOC);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_UNDV,");Serial1.print(UNDV);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_TEMP,");Serial1.print(MPPT_TEMP);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_UIN,");Serial1.print(Uin);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_IIN,");Serial1.print(Iin);Serial1.print("\n");
//          delay(bmstimi);
//          Serial1.print("MPPT");Serial1.print(MPPTId);Serial1.print("_UOUT");Serial1.print(Uout);Serial1.print("\n");
//          delay(bmstimi);
//        } else if (inData[2] == 2){
//          Serial1.print("ARDUINO2_ERROR");
//        } else if (inData[2] == 1){
//          Serial1.print("ARDUINO_ReadyPapi!");
//        } else {
//          Serial1.print("BIT_ERROR_READERROR");
//        }
//      } else {
//        for(int j = 1; j<13;j++){
//          inData[j-1] = inData[j];
//        }
//        index -= 1;
//      }
//    }
//    charsRead++;
//  }
//}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//Indica si en un vector de tamaño 20, desde el indice i hasta el final hay ceros
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
  Serial.print(BMS[5]);
  Serial.print("\n");
  
//Xbee
  Serial1.print("a");
  Serial1.print(",");
for(int i=0;i<5;i++){
  Serial1.print(KELLY_VI_D[i]);
  Serial1.print(",");
}
  Serial1.print(BMS[5]);
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
  Serial.print(KELLY_VI_D[i]);
  Serial.print(",");
}
  Serial.print(BMS[5]);
  Serial.print("\n");

//Xbee
  Serial1.print("b");
  Serial1.print(",");
for(int i=0;i<5;i++){
  Serial1.print(KELLY_VI_D[i]);
  Serial1.print(",");
}
  Serial1.print(BMS[5]);
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
  
    if (canId == 0x036 && (millis() - last_BMS_VoltageTime > 17)){ // Lectura de Voltajes en tiempo real (solo con veloc de CAN BUS mayor a 256)
      
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
                
        last_BMS_VoltageTime = millis();
        
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
      //Serial1.print(sendIdKelly1);
      //mydata.buff[0] = sendIdKelly1;
      if((B1111111&engData[0]) == 1){ 
   /*     
        //mydata.buff[1] = 1;        
        for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        }
        */

        KELLY_VI_D[0]= (buff[0],DEC);     // IA
        KELLY_VI_D[1]= (buff[1],DEC);     // IB
        KELLY_VI_D[2]= (buff[2],DEC);     // IC
        KELLY_VI_D[3]= (buff[3]/1.84);    // VA
        KELLY_VI_D[4]= (buff[4]/1.84);    // VB
        KELLY_VI_D[5]= (buff[5]/1.84);    // VC

        SendKELLY_VI_D(); 
        engData[0] += 1;
      }else if((B1111111&engData[0]) == 2){  // Si el 1er digito de engineData es '2' se procede a leer RPM.
 /*         //mydata.buff[1] = 2;        
          for(int j = 0; j < 5; j++){
            //mydata.buff[j+2] = buff[j];
          }
  */
        KELLY_RPM_D[0]= ((buff[0])<<8|buff[1]);   // RPM
        KELLY_RPM_D[1]= ((buff[3])<<8|buff[4]);   // ERROR CODE

        if (not((KELLY_RPM_D[0] > 80)&&(abs(KELLY_RPM_D[0]-KELLY_RPM_D_anterior[0])>60))){
        SendKELLY_RPM_D();
        KELLY_RPM_D_anterior[0] = KELLY_RPM_D[0];
        }
        engData[0] += 1;
        
      }else if((B1111111&engData[0]) == 3){                        // Si el 1er digito de engineData es '3' se procede a leer la temperatura.
        //mydata.buff[1] = 3;        
/*        for(int j = 0; j < 6; j++){
         //mydata.buff[j+2] = buff[j];
         }
  */

        KELLY_TEMP_D[0]= buff[0];
        KELLY_TEMP_D[1]= buff[1];
        KELLY_TEMP_D[2]= buff[2];
        KELLY_TEMP_D[3]= buff[3];
        SendKELLY_TEMP_D();
        
        if(revMode){ 
          engData[0] += 1;
        }else{
          engData[0] = 1;
        }
      
      }else if((B1111111&engData[0]) == 4){ 
/*      //mydata.buff[1] = 4;        
          for(int j = 0; j < 1; j++){
            //mydata.buff[j+2] = buff[j];
          }  
*/
        KELLY_THR_D[0]=buff[0];
        SendKELLY_THR_D();
        
        engData[0] += 1;
        
      }else if((B1111111&engData[0]) == 5){     
/*      //mydata.buff[1] = 5;        
          for(int j = 0; j < 1; j++){
            //mydata.buff[j+2] = buff[j];
          }
*/
        KELLY_REV_D[0]=buff[0];
        SendKELLY_REV_D();
       
        engData[0] = 1;
        
      } 
      //ET.sendData();
      bitWrite(engData[0],7,1);
    }else if((canId == sendIdKelly2) && !bitRead(engData[1],7)){
      //mydata.buff[0] = sendIdKelly2;
      if((B1111111&engData[1]) == 1){         
/*  
        //mydata.buff[1] = 1;        
        for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        }
  */
        KELLY_VI_I[0]= (buff[0],DEC);
        KELLY_VI_I[1]= (buff[1],DEC);
        KELLY_VI_I[2]= (buff[2],DEC);
        KELLY_VI_I[3]= (buff[3]/1.84);
        KELLY_VI_I[4]= (buff[4]/1.84);
        KELLY_VI_I[5]= (buff[5]/1.84);

        SendKELLY_VI_I();

        engData[1] += 1;
        
      }else if((B1111111&engData[1]) == 2){  // 2do digito de engineData es 2, lee RPM.
 /* 
        //mydata.buff[1] = 2;        
        for(int j = 0; j < 5; j++){
          //mydata.buff[j+2] = buff[j];
        }
*/
        KELLY_RPM_I[0]= ((buff[0])<<8|buff[1]);
        KELLY_RPM_I[1]= ((buff[3])<<8|buff[4]);
        
        if (not((KELLY_RPM_I[0] > 80)&&(abs(KELLY_RPM_I[0]-KELLY_RPM_I_anterior[0])>60))){
        SendKELLY_RPM_I();
        KELLY_RPM_I_anterior[0] = KELLY_RPM_I[0];
        }
        
        engData[1] += 1;
        
      }else if((B1111111&engData[1]) == 3){                        // 2do digito de engineData es 3, lee temperatura.
  
        //mydata.buff[1] = 3;        
        //for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        //}
        
        KELLY_TEMP_I[0]= buff[0];
        KELLY_TEMP_I[1]= buff[1];
        KELLY_TEMP_I[2]= buff[2];
        KELLY_TEMP_I[3]= buff[3];
        SendKELLY_TEMP_I();  
      
        if(revMode) { 
          engData[1] += 1;
        } else {
          engData[1] = 1;
        }
        
      }else if((B1111111&engData[1]) == 4){  
  
 /*     //mydata.buff[1] = 4;        
        for(int j = 0; j < 1; j++){
          //mydata.buff[j+2] = buff[j];
        }
*/
        KELLY_THR_I[0]=buff[0];
        SendKELLY_THR_I();
        
        engData[1] += 1;
        
      }else if((B1111111&engData[1]) == 5){     
  
/*      //mydata.buff[1] = 5;        
        for(int j = 0; j < 1; j++){
          //mydata.buff[j+2] = buff[j];
        }
*/
        KELLY_REV_I[0]=buff[0];
        SendKELLY_REV_I();
        
        engData[1] = 1;
        
      } 
      bitWrite(engData[1],7,1);
    }
  }

/*////////////////////   FIN KELLYS   ////////////////////*/

/*////////////////////   LECTURA DE DATOS MPPTs   ////////////////////*/
  
  else if(canId = 0x771)
  {
    Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
    Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
    Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
    
    Serial1.print("MPPT1_BVLR,");Serial1.print(bitRead(buff[0],7));Serial1.print("\n");
    Serial1.print("MPPT1_OVT,");Serial1.print(bitRead(buff[0],6));Serial1.print("\n");
    Serial1.print("MPPT1_NOC,");Serial1.print(bitRead(buff[0],5));Serial1.print("\n");
    Serial1.print("MPPT1_UNDV,");Serial1.print(bitRead(buff[0],4));Serial1.print("\n");
    Serial1.print("MPPT1_UIN,");Serial1.print(Uin);Serial1.print("\n");
    Serial1.print("MPPT1_IIN,");Serial1.print(Iin);Serial1.print("\n");
    Serial1.print("MPPT1_UOUT,");Serial1.print(Uout);Serial1.print("\n");
    Serial1.print("MPPT1_TAMB,");Serial1.print(buff[6]);Serial1.print("\n");
    
  }
  else if(canId = 0x772)
  {
    Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
    Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
    Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
    
    Serial1.print("MPPT2_BVLR,");Serial1.print(bitRead(buff[0],7));Serial1.print("\n");
    Serial1.print("MPPT2_OVT,");Serial1.print(bitRead(buff[0],6));Serial1.print("\n");
    Serial1.print("MPPT2_NOC,");Serial1.print(bitRead(buff[0],5));Serial1.print("\n");
    Serial1.print("MPPT2_UNDV,");Serial1.print(bitRead(buff[0],4));Serial1.print("\n");
    Serial1.print("MPPT2_UIN,");Serial1.print(Uin);Serial1.print("\n");
    Serial1.print("MPPT2_IIN,");Serial1.print(Iin);Serial1.print("\n");
    Serial1.print("MPPT2_UOUT,");Serial1.print(Uout);Serial1.print("\n");
    Serial1.print("MPPT2_TAMB,");Serial1.print(buff[6]);Serial1.print("\n");
    
  }
  
/*////////////////////   FIN MPPTs   ////////////////////*/



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


