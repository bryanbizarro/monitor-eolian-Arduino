// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>


//  SOFTWARE SERIAL ALLOWS TO READ A SECOND BUFFER SERIAL IN tx & rx PINS

int rx = 10;
int tx = 11;

SoftwareSerial mySerial(rx, tx); 

//  END SOFTWARE SERIAL

/////// KELLY //////////

unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};
unsigned char CPP_MONITOR1[1] = {0x33};
unsigned char CPP_MONITOR2[1] = {0x37};
unsigned char COM_SW_ACC[2] = {0x42, 0};
unsigned char COM_SW_BRK[2] = {0x43, 0};
unsigned char COM_SW_REV[2] = {0x44, 0};

////// END KELLY ///////


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
////////////////////////////////////////////////////////////////////
/// Tx

int del = 5;
int i   = 0; 

/// Rx
unsigned char flagRecv = 0;
unsigned char len = 0;
char str[20];

unsigned char buff[7];
unsigned char buff_BMS[7];

// Read2Serial vars
int MPPTId;
char inChar;
int index=0;
char inData[13];
int charsRead = 0;
int timi = 5;


long lastTime = 0;
unsigned long canId_Kelly;



///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
  mySerial.begin(9600);
  Serial.begin(57600);

START_INIT:

  if (CAN_OK == CAN.begin(CAN_1000KBPS))                  // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield esta ready papi!");
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
    goto START_INIT;
  }
  attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
  // rx buffer clearing
  while (Serial.available() > 0) {
    byte c = Serial.read();
  }
}


void MCP2515_ISR()
{
  flagRecv = 1;
}


bool read2Serial(){
  while(mySerial.available() || (charsRead < 13)){  //Revisar si es necesario un timeout
    //Serial.println("SerialIsAvailable");
    if(index < 12){
      //Serial.print(index);Serial.println(" < 13");
      inChar = mySerial.read();
      inData[index] = inChar;
      index++;
      //Serial.print("inChar: ");Serial.println(inChar);
    } else {
      inChar = mySerial.read();
      inData[index] = inChar;
      index++;
      //Serial.print("inChar: ");Serial.println(inChar);
      //Serial.println("buff full");
      //Serial.print(inData[0]);Serial.print(inData[1]);Serial.print(inData[2]);Serial.print(inData[3]);
      //Serial.print(inData[4]);Serial.print(inData[5]);Serial.print(inData[6]);Serial.print(inData[7]);
      //Serial.print(inData[8]);Serial.print(inData[9]);Serial.print(inData[10]);Serial.print(inData[11]);
      //Serial.println(inData[12]);
      if((inData[0] == 255) && (inData[1] == 255) && (inData[4] == 255) && (inData[12] == 255)){
        //Serial.println("data ok");
        if(inData[2] == 0){  //REVISAR que igualdad se cumpla!!!
          MPPTId = inData[3];
          index = 0;
          for(int j = 5;j<12;j++){
            buff[index] = inData[j];
          }
          int MPPT_TEMP  = buff[6];
          int  Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
          int  Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
          int Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
          int BVLR = (bitRead(buff[0],7));
          int OVT  = (bitRead(buff[0],6));
          int NOC  = (bitRead(buff[0],5));
          int UNDV = (bitRead(buff[0],4));
  
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_BVLR,");Serial.print(BVLR);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_OVT,");Serial.print(OVT);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_NOC,");Serial.print(NOC);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UNDV,");Serial.print(UNDV);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_TEMP,");Serial.print(MPPT_TEMP);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UIN,");Serial.print(Uin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_IIN,");Serial.print(Iin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UOUT");Serial.print(Uout);Serial.print("\n");
          delay(timi);
        } else if (inData[2] == 2){
          Serial.print("ARDUINO2_ERROR");
        } else if (inData[2] == 1){
          Serial.print("ARDUINO_ReadyPapi!");
        } else {
          Serial.print("BIT_ERROR_READERROR");
        }
      } else {
        for(int j = 1; j<13;j++){
          inData[j-1] = inData[j];
        }
        index -= 1;
      }
    }
    charsRead++;
  }
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop(){

  
  CAN.readMsgBuf(&len, buff_BMS);

////////////////////  BMS  ////////////////////
  unsigned long canId_BMS = CAN.getCanId();

  if( canId_BMS == 0x100)
  {
      //int packSOC         = buff_BMS[0];
      //int packCurrent     = (buff_BMS[1]<<8)|buff_BMS[2];
      //int packInstVolt    = (buff_BMS[3]<<8)|buff_BMS[4];
      //int packOpenVolt    = (buff_BMS[5]<<8)|buff_BMS[6];

      Serial.print("PACK_SOC,");Serial.print(buff_BMS[0]);Serial.print("\n");
      delay(timi);
      Serial.print("PACK_CURRENT,");Serial.print((buff_BMS[1]<<8)|buff_BMS[2]);Serial.print("\n");
      delay(timi);
      Serial.print("PACK_INST_VTG,");Serial.print((buff_BMS[3]<<8)|buff_BMS[4]);Serial.print("\n");
      delay(timi);
      Serial.print("PACK_OPEN_VTG,");Serial.print((buff_BMS[5]<<8)|buff_BMS[6]);Serial.print("\n");
      delay(timi);

  }
  else if( canId_BMS == 0x101)
  {
      unsigned int packAbsCurrent  = (buff_BMS[0]<<8)|buff_BMS[1];
      unsigned int maximumPackVolt = (buff_BMS[4]<<8)|buff_BMS[5];
      unsigned int minimumPackVolt = (buff_BMS[6]<<8)|buff_BMS[7];

      Serial.print("PACK_ABSCURRENT,");Serial.print(packAbsCurrent);Serial.print("\n");
      delay(timi);
      Serial.print("MAXIM_PACK_VTG,");Serial.print(maximumPackVolt);Serial.print("\n");
      delay(timi);
      Serial.print("MINIM_PACK_VTG,");Serial.print(minimumPackVolt);Serial.print("\n");
      delay(timi);
  }
  else if( canId_BMS == 0x102)
  {
      int highTemperature   = buff_BMS[0];
      int highThermistorID  = buff_BMS[1];
      int lowTemperature    = buff_BMS[2];
      int lowThermistorID   = buff_BMS[3];
      int avgTemp           = buff_BMS[4];
      int internalTemp      = buff_BMS[5];
      
      Serial.print("HIGH_TEMP,");Serial.print(highTemperature);Serial.print("\n");
      delay(timi);
      Serial.print("LOW_TEMP,");Serial.print(lowTemperature);Serial.print("\n");
      delay(timi);
      Serial.print("HIGH_TID,");Serial.print(highThermistorID);Serial.print("\n");
      delay(timi);
      Serial.print("LOW_TID,");Serial.print(lowThermistorID);Serial.print("\n");
      delay(timi);
      Serial.print("AVG_TEMP,");Serial.print(avgTemp);Serial.print("\n");
      delay(timi);
      Serial.print("INT_TEMP,");Serial.print(internalTemp);Serial.print("\n");
      delay(timi);
  }
  
  else if (canId_BMS == 0x036) 
  
  {
      int cellID = (buff_BMS[0]);
      int byte1 = buff_BMS[1];
      int byte2 = buff_BMS[2];
      unsigned int instVolt = (buff_BMS[1]<<8)|buff_BMS[2];
      int internalResist =  (buff_BMS[3]<<8)|buff_BMS[4];
      int openVolt =  (buff_BMS[5]<<8)|buff_BMS[6];

      Serial.print("CELL_INSTVTG,");Serial.print(cellID);Serial.print(",");Serial.print(instVolt);Serial.print("\n");
      delay(timi);
  }

//  else if (canId_BMS == 0x081)
//  
//  {
//      int thermistorID = (buff_BMS[0]);
//      int temperature = (buff_BMS[1]);
//      
//      Serial.print("TEMP,");Serial.print(thermistorID);Serial.print(",");Serial.print(temperature);Serial.print("\n");
//      delay(timi);
//  }
    
    ////////////////                     Fin BMS  ///////////////////////////////////////////////////

  Serial.flush();

    ///////////////////  INICIO KELLYs  //////////////////

    /////// KELLY 1 //////

  if((millis() - lastTime)>1000)
  {
    
//    CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ1);
//    delay(del);                       // send data per 100ms
//    if (flagRecv) { //cheque si recibe datos
//    //  flagRecv = 0; //borrar flag
//      CAN.readMsgBuf(&len, buff_BMS);
////      Serial.print("Brake A/D,");Serial.print(buff_BMS[0]);Serial.print("\n");
////      Serial.print("TPS A/D,");Serial.print(buff_BMS[1]);Serial.print("\n");
////      Serial.print("Operation voltage A/D,");Serial.print(buff_BMS[2]);Serial.print("\n");
////      Serial.print("Vs A/D,");Serial.print(buff_BMS[3]);Serial.print("\n");
////      Serial.print("B+ A/D,");Serial.print(buff_BMS[4]);Serial.print("\n");
//    }
  
    CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
    delay(del);  // send data per 100ms
    
    //if (flagRecv) { //chequea si recibe datos
    //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff_BMS);
    canId_Kelly = CAN.getCanId();
    if(canId_Kelly == 0xC8){
      
      Serial.print("I1_A,");Serial.print(buff_BMS[0]);Serial.print("\n");
      delay(timi);
      Serial.print("I1_B,");Serial.print(buff_BMS[1]);Serial.print("\n");
      delay(timi);
      Serial.print("I1_C,");Serial.print(buff_BMS[2]);Serial.print("\n");
      delay(timi);
      Serial.print("V1_A,");Serial.print(buff_BMS[3]);Serial.print("\n");
      delay(timi);
      Serial.print("V1_B,");Serial.print(buff_BMS[4]);Serial.print("\n");
      delay(timi);
      Serial.print("V1_C,");Serial.print(buff_BMS[5]);Serial.print("\n");
      delay(timi);
    }
    if(canId_Kelly == 0x12C){
      
      Serial.print("I2_A,");Serial.print(buff_BMS[0]);Serial.print("\n");
      delay(timi);
      Serial.print("I2_B,");Serial.print(buff_BMS[1]);Serial.print("\n");
      delay(timi);
      Serial.print("I2_C,");Serial.print(buff_BMS[2]);Serial.print("\n");
      delay(timi);
      Serial.print("V2_A,");Serial.print(buff_BMS[3]);Serial.print("\n");
      delay(timi);
      Serial.print("V2_B,");Serial.print(buff_BMS[4]);Serial.print("\n");
      delay(timi);
      Serial.print("V2_C,");Serial.print(buff_BMS[5]);Serial.print("\n");
      delay(timi);
    }
  
    CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
    delay(del);                       // send data per 100ms
    //if (flagRecv) { //chequea si recibe datos
    //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff_BMS);
    canId_Kelly = CAN.getCanId();
    if(canId_Kelly == 0xC8){
//      Serial.print("PWM,");Serial.print(buff_BMS[0]);Serial.print("\n");
//      Serial.print("Enable motor rotation,");Serial.print(buff_BMS[1]);Serial.print("\n");  // 1 enable 0 disable
      Serial.print("ENG_TEMP1,");Serial.print(buff_BMS[2]);Serial.print("\n");      // Temperatura motor: Celcius
      delay(timi);
//      Serial.print("Controller's temperature,");Serial.print(buff_BMS[3]);Serial.print("\n");
//      Serial.print("Temp of HIGH side FETMOS heat sink,");Serial.print(buff_BMS[4]);Serial.print("\n");   //Unaccurate below 30C
//      Serial.print("Temo of LOW side FETMOS heat sink,");Serial.print(buff_BMS[5]);Serial.print("\n");
    }
    if(canId_Kelly == 0x12C) {
      Serial.print("ENG_TEMP2,");Serial.print(buff_BMS[2]);Serial.print("\n");      // Temperatura motor: Celcius
      delay(timi);
    }
    
    CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
    delay(del);                       // send data per 100ms
    //if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff_BMS);
    canId_Kelly = CAN.getCanId();
    if(canId_Kelly == 0xC8) {
        Serial.print("ENGA,");Serial.print((buff_BMS[0])<<8|buff_BMS[1]);Serial.print("\n");
        delay(timi);
//      Serial.print("MSB of mechanical speed in RPM,");Serial.print(buff_BMS[0]);Serial.print("\n");
//      Serial.print("LSB of mechanical speed in RPM,");Serial.print(buff_BMS[1]);Serial.print("\n");
//      Serial.print("Mechanical speed calculation,");Serial.print((buff_BMS[0]<<8)|buff_BMS[1]);Serial.print("\n");
//      Serial.print("Present current accounts for percent of the rated current of controller,");Serial.print(buff_BMS[2]);Serial.print("\n");
//      Serial.print("MSB of error code,");Serial.print(buff_BMS[3]);Serial.print("\n");
//      Serial.print("LSB of error code,");Serial.print(buff_BMS[4]);Serial.print("\n");
//      Serial.print("Controller error status,");Serial.print((buff_BMS[3]<<8)|buff_BMS[4]);Serial.print("\n");   // If = 0x4008 Error code is 0x43 (Bit 6 of [3]) and 0x14(Bit 3 of [4])                    
  
    }
    if(canId_Kelly == 0x12C) {
      Serial.print("ENGB,");Serial.print((buff_BMS[0])<<8|buff_BMS[1]);Serial.print("\n");
      delay(timi);
    }
  
////    CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_ACC);
////    delay(del);                       // send data per 100ms
////    if (flagRecv) { //chequea si recibe datos
////      //flagRecv = 0; //borrar flag
////      CAN.readMsgBuf(&len, buff_BMS);
//////      Serial.print("Current throttle switch status,");Serial.print(buff_BMS[0]);Serial.print("\n"); // 1 active 0 inactive
////     }
//     
////    CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_BRK);
////    delay(del);                       // send data per 100ms
////    if (flagRecv) { //chequea si recibe datos
////      //flagRecv = 0; //borrar flag
////      CAN.readMsgBuf(&len, buff_BMS);
//////      Serial.print("Current Brake switch status,");Serial.print(buff_BMS[0]);Serial.print("\n");  //1 active 0 inactive
////     }
//  
////    CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_REV);
////    delay(del);                       // send data per 100ms
////    if (flagRecv) { //chequea si recibe datos
////      //flagRecv = 0; //borrar flag
////      CAN.readMsgBuf(&len, buff_BMS);
//////      Serial.print("Current Reverse switch status,");Serial.print(buff_BMS[0]);Serial.print("\n");  // 1 active 0 inactive
////     }
//
//
//    ///////// KELLY 2 ///////////////
//
////    CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ1);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //cheque si recibe datos
////  //  flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("Brake A/D,");Serial.print(buff_BMS[0]);Serial.print("\n");
//////    Serial.print("TPS A/D,");Serial.print(buff_BMS[1]);Serial.print("\n");
//////    Serial.print("Operation voltage A/D,");Serial.print(buff_BMS[2]);Serial.print("\n");
//////    Serial.print("Vs A/D,");Serial.print(buff_BMS[3]);Serial.print("\n");
//////    Serial.print("B+ A/D,");Serial.print(buff_BMS[4]);Serial.print("\n");
////  }
//
////  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //chequea si recibe datos
////  //  flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("Ia A/D,");Serial.print(buff_BMS[0]);Serial.print("\n");
//////    Serial.print("Ib A/D,");Serial.print(buff_BMS[1]);Serial.print("\n");
//////    Serial.print("Ic A/D,");Serial.print(buff_BMS[2]);Serial.print("\n");
//////    Serial.print("Va A/D,");Serial.print(buff_BMS[3]);Serial.print("\n");
//////    Serial.print("Vb A/D,");Serial.print(buff_BMS[4]);Serial.print("\n");
//////    Serial.print("Vc A/D,");Serial.print(buff_BMS[5]);Serial.print("\n");
////  }
//
////  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //chequea si recibe datos
////  //  flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("PWM,");Serial.print(buff_BMS[0]);Serial.print("\n");
//////    Serial.print("Enable motor rotation,");Serial.print(buff_BMS[1]);Serial.print("\n");  // 1 enable 0 disable
////      Serial.print("ENG_TEMP,2,");Serial.print(buff_BMS[2]);Serial.print("\n");     // Celcius
//////    Serial.print("Controller's temperature,");Serial.print(buff_BMS[3]);Serial.print("\n");
//////    Serial.print("Temp of HIGH side FETMOS heat sink,");Serial.print(buff_BMS[4]);Serial.print("\n");   //Unaccurate below 30C
//////    Serial.print("Temo of LOW side FETMOS heat sink,");Serial.print(buff_BMS[5]);Serial.print("\n");
////  }
//  
////  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //chequea si recibe datos
////    //flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("MSB of mechanical speed in RPM,");Serial.print(buff_BMS[0]);Serial.print("\n");
//////    Serial.print("LSB of mechanical speed in RPM,");Serial.print(buff_BMS[1]);Serial.print("\n");
////      Serial.print("ENG_RPM,2,");Serial.print((buff_BMS[0]<<8)|buff_BMS[1]);Serial.print("\n");
//////    Serial.print("Present current accounts for percent of the rated current of controller,");Serial.print(buff_BMS[2]);Serial.print("\n");
//////    Serial.print("MSB of error code,");Serial.print(buff_BMS[3]);Serial.print("\n");
//////    Serial.print("LSB of error code,");Serial.print(buff_BMS[4]);Serial.print("\n");
//////    Serial.print("Controller error status,");Serial.print((buff_BMS[3]<<8)|buff_BMS[4]);Serial.print("\n");   // If = 0x4008 Error code is 0x43 (Bit 6 of [3]) and 0x14(Bit 3 of [4])                    
////   }
//
////  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_ACC);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //chequea si recibe datos
////    //flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("Current throttle switch status,");Serial.print(buff_BMS[0]);Serial.print("\n"); // 1 active 0 inactive
////   }
//   
////  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_BRK);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //chequea si recibe datos
////    //flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("Current Brake switch status,");Serial.print(buff_BMS[0]);Serial.print("\n");  //1 active 0 inactive
////   }
//
////  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_REV);
////  delay(del);                       // send data per 100ms
////  if (flagRecv) { //chequea si recibe datos
////    //flagRecv = 0; //borrar flag
////    CAN.readMsgBuf(&len, buff_BMS);
//////    Serial.print("Current Reverse switch status,");Serial.print(buff_BMS[0]);Serial.print("\n");  // 1 active 0 inactive
////   }

   Serial.flush();
   lastTime = millis();
  }
    
    ////////////////////////////////////////////// MPPT 2.0 ////////////////////////////////////////////////////////////

  read2Serial();
   
  Serial.flush();

  if(charsRead > 12){
    charsRead = 0;
  }

    /////// Fin Loop ///////
} 

