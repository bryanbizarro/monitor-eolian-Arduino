/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR PRINCIPAL
 *  PERMITE LEER DATOS DE CAN BUS
 *  LECTURA EN TIEMPO REAL DE:
 *  BMS - KELLYs - MPPT
 */

/*  PROTOCOLO DE ENVÍO: 12 bytes
 *  Bytes
 *  ╔  ╗
 *  ║00║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║01║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║02║ Reservados para ID de BMS. Valores:2     ,0 para MPPT, 1 para MOTORES, 2 para BMS
 *  ╠  ╣
 *  ║03║ Reservados para ID de BMS. Valores:0
 *  ╠  ╣
 *  ║04║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║05║ CAN BUFF[0]
 *  ╠  ╣
 *  ║06║ CAN BUFF[1]
 *  ╠  ╣
 *  ║07║ CAN BUFF[2]
 *  ╠  ╣
 *  ║08║ CAN BUFF[3]
 *  ╠  ╣
 *  ║09║ CAN BUFF[4]
 *  ╠  ╣
 *  ║10║ CAN BUFF[5]
 *  ╠  ╣
 *  ║11║ CAN BUFF[6]
 *  ╠  ╣
 *  ║12║ CAN BUFF[7]
 *  ╠  ╣
 *  ║13║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╚  ╝
 */

bool revMode = true;
bool motores = true;

bool serialEngine = true;
bool serialBms = false;

#include <mcp_can.h>
#include <SPI.h>

#define baudRate 115200
#define pint1 A0
#define pint2 A1
#define bmstimi 0
#define timi 256
#define maxTimi 1200
#define tiempoVelocidad 1000
#define recvIdKelly1 0xC8
#define sendIdKelly1 0xD2
#define recvIdKelly2 0xC9
#define sendIdKelly2 0xD3
#define pinRX 10
#define pinTX 11
#define radio 0.550
#define pi 3.141592653


//  END SOFTWARE SERIAL

/////// KELLY //////////

unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};  // {Ia,Ib,Ic,Va,Vb,Vc}
unsigned char CPP_MONITOR1[1] = {0x33};         // {PWM,EnableMotorRotation,EngTemp,KellyTemp,highSideFETMOSTemp,lowSideFETMOSTemp}
unsigned char CPP_MONITOR2[1] = {0x37};         // {MSB RPM, LSB RPM, someValue, MSB ERROR CODE, LSB ERROR CODE}
unsigned char COM_SW_ACC[2] = {0x42, 0};
unsigned char COM_SW_BRK[2] = {0x43, 0};
unsigned char COM_SW_REV[2] = {0x44, 0};

unsigned char engData[2] = {B10000001,B10000001}; 

int RPM[2] = {0,0};

////// END KELLY ///////


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
////////////////////////////////////////////////////////////////////
/// Tx

int del = 5;

/// Rx
unsigned char flagRecv = 0;
unsigned char len = 0;
char str[20];

unsigned long canId;
unsigned char buff[8];

// Read2Serial vars
int MPPTId;
char inChar;
int index=0;
char inData[13];
int charsRead = 0;


// MPPTs vars

unsigned int Uin, Iin, Uout;


/*////////////////// delayTime vars //////////////////*/
long lastKelly1Time = 0;
long lastKelly2Time = 0;
long lastMpptTime = 0;
long lastVoltageTime = 0;
long lastVelocityTime = 0;

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

  if (CAN_OK == CAN.begin(CAN_1000KBPS))                  // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield BMS iniciado!");
    Serial1.println("CAN_SHIELD,1");
  }
  else
  {
    Serial1.println("CAN_SHIELD,0");
    Serial.println("Falla de inicio CAN BUS Shield BMS");
    Serial.println("Reiniciando CAN BUS Shield BMS");
    delay(100);
    goto START_INIT;
  }
  attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
  // rx buffer clearing
  
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

double getVelocidad(){
  double meanRPM = (RPM[0] + RPM[1])/2;
  return 6*pi*radio*meanRPM;
}

void loop(){

  //Serial1.print("                               1st engineData = ");Serial1.println(engineData);

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
  
    if (canId == 0x036 && (millis() - lastVoltageTime > 17)) // Lectura de Voltajes en tiempo real (solo con veloc de CAN BUS mayor a 256)
    
    {
        int cellID = (buff[0]);
        int byte1 = buff[1];
        int byte2 = buff[2];
        unsigned int instVolt = (buff[1]<<8)|buff[2];
        int internalResist =  (buff[3]<<8)|buff[4];
        int openVolt =  (buff[5]<<8)|buff[6];
  
        Serial1.print("CELL_INSTVTG,");Serial1.print(cellID);Serial1.print(",");Serial1.print(instVolt);Serial1.print("\n");
        if(serialBms){
          Serial.print("Voltaje modulo ");Serial.print(cellID);Serial.print(":  ");Serial.print(instVolt);Serial.print("\n");
        }
        lastVoltageTime = millis();
    }

    else if (canId == 0x081) // Lectura de temperaturas
    
    {
        int thermistorID = (buff[0]);
        int temperature = (buff[1]);
        
        Serial1.print("TEMP,");Serial1.print(thermistorID);Serial1.print(",");Serial1.print(temperature);Serial1.print("\n");
        if(serialBms){
        Serial.print("Temperatura Nro ");Serial.print(thermistorID);Serial.print(": ");Serial.print(temperature);Serial.print("\n");
        }
    }

  //   else if (canId == 0x082) // Lectura de temperaturas
  //  
  //  {
  //      int thermistorID = (buff[1]);
  //      int temperature = (buff[2]);
  //      
  //      Serial1.print("TEMP,");Serial1.print(thermistorID);Serial1.print(",");Serial1.print(temperature);Serial1.print("  82");Serial1.print("\n");
  //      delay(bmstimi);
  //  }
  
    else if( canId == 0x100)
    {
        //int packSOC         = buff[0];
        //int packCurrent     = (buff[1]<<8)|buff[2];
        //int packInstVolt    = (buff[3]<<8)|buff[4];
        //int packOpenVolt    = (buff[5]<<8)|buff[6];
        
        Serial1.print("PACK_SOC,");Serial1.print(buff[0]);Serial1.print("\n");
        
        Serial1.print("PACK_CURRENT,");Serial1.print((buff[1]<<8)|buff[2]);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("PACK_INST_VTG,");Serial1.print((buff[3]<<8)|buff[4]);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("PACK_OPEN_VTG,");Serial1.print((buff[5]<<8)|buff[6]);Serial1.print("\n");
        delay(bmstimi);
  
    }
    else if( canId == 0x101)
    {
        unsigned int packAbsCurrent  = (buff[0]<<8)|buff[1];
        unsigned int maximumPackVolt = (buff[2]<<8)|buff[3];
        unsigned int minimumPackVolt = (buff[4]<<8)|buff[5];
  
        Serial1.print("PACK_ABSCURRENT,");Serial1.print(packAbsCurrent);Serial1.print("\n");
        Serial1.print("MAXIM_PACK_VTG,");Serial1.print(maximumPackVolt);Serial1.print("\n");
        Serial1.print("MINIM_PACK_VTG,");Serial1.print(minimumPackVolt);Serial1.print("\n");
        
        if(serialBms){
          Serial.print("Corriente (absoluta): ");Serial.print(packAbsCurrent);Serial.print(" | ");
          Serial.print("Voltaje pack maximo: ");Serial.print(maximumPackVolt);Serial.print(" | ");
          Serial.print("Voltaje pack minimo: ");Serial.print(minimumPackVolt);Serial.print("\n");
        }
    }
    else if( canId == 0x102)
    {
        int highTemperature   = buff[0];
        int highThermistorID  = buff[1];
        int lowTemperature    = buff[2];
        int lowThermistorID   = buff[3];
        int avgTemp           = buff[4];
        int internalTemp      = buff[5];
        
        Serial1.print("HIGH_TEMP,");Serial1.print(highTemperature);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("LOW_TEMP,");Serial1.print(lowTemperature);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("HIGH_TID,");Serial1.print(highThermistorID);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("LOW_TID,");Serial1.print(lowThermistorID);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("AVG_TEMP,");Serial1.print(avgTemp);Serial1.print("\n");
        delay(bmstimi);
        Serial1.print("INT_TEMP,");Serial1.print(internalTemp);Serial1.print("\n");
        delay(bmstimi);
    }
    
/*////////////////////   FIN BMS   ////////////////////*/

/*////////////////////   LECTURA DE DATOS KELLYS   ////////////////////*/

    else if((canId == sendIdKelly1) && !bitRead(engData[0],7)){
      //Serial1.print(sendIdKelly1);
      //mydata.buff[0] = sendIdKelly1;
      if((B1111111&engData[0]) == 1){ 
        
        //mydata.buff[1] = 1;        
        for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        }
        Serial1.print("I1_A,");Serial1.print(buff[0]);Serial1.print("\n");
        Serial1.print("I1_B,");Serial1.print(buff[1]);Serial1.print("\n");
        Serial1.print("I1_C,");Serial1.print(buff[2]);Serial1.print("\n");
        Serial1.print("V1_A,");Serial1.print(buff[3]/1.84);Serial1.print("\n");
        Serial1.print("V1_B,");Serial1.print(buff[4]/1.84);Serial1.print("\n");
        Serial1.print("V1_C,");Serial1.print(buff[5]/1.84);Serial1.print("\n");

        if(serialEngine){
          Serial.print("I1_A,");Serial.print(buff[0],DEC);Serial.print(" | ");
          Serial.print("I1_B,");Serial.print(buff[1],DEC);Serial.print(" | ");
          Serial.print("I1_C,");Serial.print(buff[2],DEC);Serial.print(" | ");
          Serial.print("V1_A,");Serial.print(buff[3]/1.84);Serial.print(" | ");
          Serial.print("V1_B,");Serial.print(buff[4]/1.84);Serial.print(" | ");
          Serial.print("V1_C,");Serial.print(buff[5]/1.84);Serial.print("\n");
        }
        
        engData[0] += 1;
      } else if((B1111111&engData[0]) == 2){  // Si el 1er digito de engineData es '2' se procede a leer RPM.
        //mydata.buff[1] = 2;        
        for(int j = 0; j < 5; j++){
          //mydata.buff[j+2] = buff[j];
        }
        
        Serial1.print("ENG1_RPM,");Serial1.print((buff[0])<<8|buff[1]);Serial1.print("\n");
        Serial1.print("ENG1_ERR_CODE,");Serial1.print((buff[3])<<8|buff[4]);Serial1.print("\n");

        if(serialEngine){
          Serial.print("ENG1_RPM,");Serial.print((buff[0])<<8|buff[1]);Serial.print(" | ");
          Serial.print("ENG1_ERR_CODE,");Serial.print((buff[3])<<8|buff[4]);Serial.print("\n");
        }
        
        RPM[0] = (buff[0])<<8|buff[1];
        engData[0] += 1;
      } else if((B1111111&engData[0]) == 3){                        // Si el 1er digito de engineData es '3' se procede a leer la temperatura.
        //mydata.buff[1] = 3;        
        for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        }
        
        Serial1.print("ENG1_PWM,");Serial1.print(buff[0]);Serial1.print("\n");
        Serial1.print("ENG1_EMR,");Serial1.print(buff[1]);Serial1.print("\n");
        Serial1.print("ENG1_TEMP,");Serial1.print(buff[2]);Serial1.print("\n");      // Temperatura motor: Celcius
        Serial1.print("Kelly1_Temp,");Serial1.print(buff[3]);Serial1.print("\n");

        if(serialEngine){
          Serial.print("ENG1_PWM,");Serial.print(buff[0]);Serial.print(" | ");
          Serial.print("ENG1_EMR,");Serial.print(buff[1]);Serial.print(" | ");
          Serial.print("ENG1_TEMP,");Serial.print(buff[2],DEC);Serial.print(" | ");      // Temperatura motor: Celcius
          Serial.print("Kelly1_Temp,");Serial.print(buff[3]);Serial.print("\n");
        }
        
        if(revMode) { 
          engData[0] += 1;
        } else {
          engData[0] = 1;
        }
      } else if((B1111111&engData[0]) == 4){ 
  
        //mydata.buff[1] = 4;        
        for(int j = 0; j < 1; j++){
          //mydata.buff[j+2] = buff[j];
        }                            
        Serial1.print("ENG1_Current_Throttle_Switch_Status,");Serial1.print(buff[0]);Serial1.print("\n");      // Throttle Status

        if(serialEngine){
          Serial.print("ENG1_Current_Throttle_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");
        }
        engData[0] += 1;
        
      } else if((B1111111&engData[0]) == 5){     
  
        //mydata.buff[1] = 5;        
        for(int j = 0; j < 1; j++){
          //mydata.buff[j+2] = buff[j];
        }
        Serial1.print("ENG1_Current_Reserve_Switch_Status,");Serial1.print(buff[0]);Serial1.print("\n");      // Reverse Status
        if(serialEngine){
          Serial.print("ENG1_Current_Reserve_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");
        }
        engData[0] = 1;
        
      } 
      //ET.sendData();
      bitWrite(engData[0],7,1);
    } else if((canId == sendIdKelly2) && !bitRead(engData[1],7)){
      //mydata.buff[0] = sendIdKelly2;
      if((B1111111&engData[1]) == 1){         
  
        //mydata.buff[1] = 1;        
        for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        }
  
        Serial1.print("I2_A,");Serial1.print(buff[0]);Serial1.print("\n");
        Serial1.print("I2_B,");Serial1.print(buff[1]);Serial1.print("\n");
        Serial1.print("I2_C,");Serial1.print(buff[2]);Serial1.print("\n");
        Serial1.print("V2_A,");Serial1.print(buff[3]/1.84);Serial1.print("\n");
        Serial1.print("V2_B,");Serial1.print(buff[4]/1.84);Serial1.print("\n");
        Serial1.print("V2_C,");Serial1.print(buff[5]/1.84);Serial1.print("\n");

        if(serialEngine){
          Serial.print("I2_A,");Serial.print(buff[0],DEC);Serial.print(" | ");
          Serial.print("I2_B,");Serial.print(buff[1],DEC);Serial.print(" | ");
          Serial.print("I2_C,");Serial.print(buff[2],DEC);Serial.print(" | ");
          Serial.print("V2_A,");Serial.print(buff[3]/1.84);Serial.print(" | ");
          Serial.print("V2_B,");Serial.print(buff[4]/1.84);Serial.print(" | ");
          Serial.print("V2_C,");Serial.print(buff[5]/1.84);Serial.print("\n");
        }
        engData[1] += 1;
        
      } else if((B1111111&engData[1]) == 2){  // 2do digito de engineData es 2, lee RPM.
  
        //mydata.buff[1] = 2;        
        for(int j = 0; j < 5; j++){
          //mydata.buff[j+2] = buff[j];
        }
        
        Serial1.print("ENG2_RPM,");Serial1.print((buff[0])<<8|buff[1]);Serial1.print("\n");
        Serial1.print("ENG2_ERR_CODE,");Serial1.print((buff[3])<<8|buff[4]);Serial1.print("\n");
        if(serialEngine){
          Serial.print("ENG2_RPM,");Serial.print((buff[0])<<8|buff[1]);Serial.print(" | ");
          Serial.print("ENG2_ERR_CODE,");Serial.print((buff[3])<<8|buff[4]);Serial.print("\n");
        }
        RPM[1] = (buff[0])<<8|buff[1];
        engData[1] += 1;
        
      } else if((B1111111&engData[1]) == 3){                        // 2do digito de engineData es 3, lee temperatura.
  
        //mydata.buff[1] = 3;        
        //for(int j = 0; j < 6; j++){
          //mydata.buff[j+2] = buff[j];
        //}
        
        Serial1.print("ENG2_PWM,");Serial1.print(buff[0]);Serial1.print("\n");      // PWM
        Serial1.print("ENG2_EMR,");Serial1.print(buff[1]);Serial1.print("\n");      // Enable Motor Rotation
        Serial1.print("ENG2_TEMP,");Serial1.print(buff[2]);Serial1.print("\n");     // Temperatura motor: Celcius
        Serial1.print("Kelly2_Temp,");Serial1.print(buff[3]);Serial1.print("\n");

        if(serialEngine){
          Serial.print("ENG2_PWM,");Serial.print(buff[0]);Serial.print(" | ");      // PWM
          Serial.print("ENG2_EMR,");Serial.print(buff[1]);Serial.print(" | ");      // Enable Motor Rotation
          Serial.print("ENG2_TEMP,");Serial.print(buff[2],DEC);Serial.print(" | ");     // Temperatura motor: Celcius
          Serial.print("Kelly2_Temp,");Serial.print(buff[3]);Serial.print("\n");
        }
        
        
        if(revMode) { 
          engData[1] += 1;
        } else {
          engData[1] = 1;
        }
        
      } else if((B1111111&engData[1]) == 4){  
  
        //mydata.buff[1] = 4;        
        for(int j = 0; j < 1; j++){
          //mydata.buff[j+2] = buff[j];
        }
        Serial1.print("ENG2_Current_Throttle_Switch_Status,");Serial1.print(buff[0]);Serial1.print("\n");      // Throttle Status
        if(serialEngine){
          Serial.print("ENG2_Current_Throttle_Switch_Status,");Serial.print(buff[0]);Serial.print("\n"); 
        }
        engData[1] += 1;
        
      } else if((B1111111&engData[1]) == 5){     
  
        //mydata.buff[1] = 5;        
        for(int j = 0; j < 1; j++){
          //mydata.buff[j+2] = buff[j];
        }
        Serial1.print("ENG2_Current_Reverse_Switch_Status,");Serial1.print(buff[0]);Serial1.print("\n");      // Reverse Status
        if(serialEngine){
          Serial.print("ENG2_Current_Reverse_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");
        }
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

//  if((millis() - lastMpptTime) > 512){
//    CAN.sendMsgBuf(0x711, 0, 0, 0);
//    CAN.sendMsgBuf(0x712, 0, 0, 0);
//    lastMpptTime = millis();
//  }

/*////////////////////   FIN REQUEST MPPTs   ////////////////////*/


  if(millis() - lastVelocityTime > tiempoVelocidad){
    Serial1.print("VELOCIDAD,");Serial1.println(getVelocidad());

    Serial.print("Velocidad Actual: ");Serial.println(getVelocidad());
    lastVelocityTime = millis();
  }
  
  Serial1.flush();

  if(charsRead > 12){
    charsRead = 0;
  }

    /////// Fin Loop ///////
} 

