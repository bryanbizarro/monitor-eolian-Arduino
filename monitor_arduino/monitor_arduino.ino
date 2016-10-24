// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>


//  SOFTWARE SERIAL ALLOWS TO READ A SECOND BUFFER SERIAL IN tx & rx PINS

int rx = 10;
int tx = 11;

SoftwareSerial mySerial(rx, tx); 

//  END SOFTWARE SERIAL


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
////////////////////////////////////////////////////////////////////
/// Tx
unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};

int del = 100;
int i   = 0; 

/// Rx
unsigned char flagRecv = 0;
unsigned char len = 0;
char str[20];

unsigned char buff[7];
unsigned char buff_BMS[7];


/// Excel
const int serialBufferSize = 32;      // buffer size for input
char  serialBuffer[serialBufferSize]; // buffer for input
const int serialMaxArgs = 4;          // max CSV message args
char* serialArgs[serialMaxArgs];      // args pointers
int   idxM1 = 0;                        // index
int   idxM2 = 0;
int   idxBMS  = 0;
int   outputTiming = 1000;            // packet sending timing in ms      important: this dermines the output timing
float input1;                         // received value
//float inputArray[6];                  // received values

// Read2Serial vars
int MPPTId;
char inChar;
int index=0;
char inData[13];



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
  while(mySerial.available()){  //Revisar si es necesario un timeout
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
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_OVT,");Serial.print(OVT);Serial.print("\n");
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_NOC,");Serial.print(NOC);Serial.print("\n");
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UNDV,");Serial.print(UNDV);Serial.print("\n");
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_TEMP,");Serial.print(MPPT_TEMP);Serial.print("\n");
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UIN,");Serial.print(Uin);Serial.print("\n");
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_IIN,");Serial.print(Iin);Serial.print("\n");
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UOUT");Serial.print(Uout);Serial.print("\n");
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
  }
  
  /*SERIAL2_START:
  if(mySerial.available()){
      c = mySerial.read();
      if(c == 255){
        c = mySerial.read();
        if(c == 255){
          SERIAL2_STEP2:
          c = mySerial.read();
          if(c == 0){
            MPPTId = mySerial.read()-48;
            c = mySerial.read();
            if(c == 255){
              mySerial.readBytes(buff,7);
              c = mySerial.read();
              if(c == 255){
                
                int MPPT_TEMP  = buff[6];
                int  Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
                int  Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
                int Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
                int BVLR = (bitRead(buff[0],7));
                int OVT  = (bitRead(buff[0],6));
                int NOC  = (bitRead(buff[0],5));
                int UNDV = (bitRead(buff[0],4));
  
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_BVLR,");Serial.print(BVLR);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_OVT,");Serial.print(OVT);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_NOC,");Serial.print(NOC);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UNDV,");Serial.print(UNDV);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_TEMP,");Serial.print(MPPT_TEMP);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UIN,");Serial.print(Uin);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_IIN,");Serial.print(Iin);Serial.print("\n");
                Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UOUT");Serial.print(Uout);Serial.print("\n");
  
                
                return true;
              }
            }
            Serial.print("MISSING_MSG");
            return false;
            
          } else if(c == 2){
            c = mySerial.read();
            if(c == 255){
              Serial.print("ARDUINO2_CANERR");
              return true;
            } else {
              Serial.print("MISSING_MSG");
              return false;
            }
          } else if(c == 1) {
            c = mySerial.read();
            if(c == 255){
              Serial.print("ARDUINO2_READY4YOU");
              return true;
            } else {
              Serial.print("MISSING_MSG");
              return false;
            }
          } else if(c == 255){
            goto SERIAL2_STEP2;
          }
        } else {
          goto SERIAL2_START;
        }

            
      } else {
        goto SERIAL2_START;
      }
  }
      
      
      
      
      /*
      
      else if(c == 1) {
        c = mySerial.read();      
        if(c == 1) {
          Serial.print("ARDUINO2_ERR");
          return true;
        } else if(== 0)){
          Serial.print("ARDUINO2_READY4YOU");
          return true;
      }
    }
    return false;*/
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop(){

  
  CAN.readMsgBuf(&len, buff_BMS);

////////////////////  BMS  ////////////////////
  unsigned long canId_BMS = CAN.getCanId();

//  if(canId_BMS == 1570){
//   Serial.print("XLS,write,DatosMB,G4,");Serial.print(bitRead(buff[0],0));Serial.print("\n");
//
//   Serial.print("XLS,write,BMS,A");Serial.print(idxBMS + 3);Serial.print(",");Serial.print("%date%");Serial.print("\n");
//   Serial.print("XLS,write,BMS,B");Serial.print(idxBMS + 3);Serial.print(",");Serial.print("%time%");Serial.print("\n");
//   Serial.print("XLS,write,BMS,C");Serial.print(idxBMS + 3);Serial.print(",");Serial.print(idxBMS);Serial.print("\n");
//   Serial.print("XLS,write,BMS,D");Serial.print(idxBMS + 3);Serial.print(",");Serial.print(bitRead(buff[0],0));Serial.print("\n");
//   
//  }

  if( canId_BMS == 0x100)
  {
      int packSOC         = buff_BMS[0];
      int packCurrent     = (buff_BMS[1]<<8)|buff_BMS[2];
      int packInstVolt    = (buff_BMS[3]<<8)|buff_BMS[4];
      int packOpenVolt    = (buff_BMS[5]<<8)|buff_BMS[6];
      
      
//      int relayState      = buff_BMS[0];
//      int maxCellNumber   = buff_BMS[1];
//      int populatedCells  = buff_BMS[2];
//      int rollingCounter  = buff_BMS[3];
//      int packCCL         = buff_BMS[4];
//      int packDCL         = buff_BMS[5];
//      
//      int packAmphours    = buff_BMS[7];

      Serial.print("PACK_SOC,");Serial.print(packSOC);Serial.print("\n");
      Serial.print("PACK_CURRENT,");Serial.print(packCurrent);Serial.print("\n");
      Serial.print("PACK_INST_VTG,");Serial.print(packInstVolt);Serial.print("\n");
      Serial.print("PACK_OPEN_VTG,");Serial.print(packOpenVolt);Serial.print("\n");

//      Serial.print("REL_STATE,");Serial.print(relayState);Serial.print("\n");
//      Serial.print("MAX_CELL_NUM,");Serial.print(maxCellNumber);Serial.print("\n");
//      Serial.print("POP_CELLS,");Serial.print(populatedCells);Serial.print("\n");
//      Serial.print("ROLLING_COUNT,");Serial.print(rollingCounter);Serial.print("\n");
//      Serial.print("PACK_CCL,");Serial.print(packCCL);Serial.print("\n");
//      Serial.print("PACK_DCL,");Serial.print(packDCL);Serial.print("\n");
//      Serial.print("PACK_AMPH,");Serial.print(packAmphours);Serial.print("\n");
  }
  
  if( canId_BMS == 0x101)
  {
      int packAbsCurrent  = (buff_BMS[0]<<8)|buff_BMS[1];
      int maximumPackVolt = (buff_BMS[4]<<8)|buff_BMS[5];
      int minimumPackVolt = (buff_BMS[6]<<8)|buff_BMS[7];
      
      
//      int packResistance  = (buff_BMS[6]<<8)|buff_BMS[7];

      Serial.print("PACK_ABSCURRENT,");Serial.print(packAbsCurrent);Serial.print("\n");
      Serial.print("MAXIM_PACK_VTG,");Serial.print(maximumPackVolt);Serial.print("\n");
      Serial.print("MINIM_PACK_VTG,");Serial.print(minimumPackVolt);Serial.print("\n");
      
//      Serial.print("PACK_RESIST,");Serial.print(packResistance);Serial.print("\n");
  }

//  if( canId_BMS == 0x102)
//  {
//      int packDOD         = buff_BMS[0];
//      int packHealth      = buff_BMS[1];
//      int packSummedVolt  = (buff_BMS[2]<<8)|buff_BMS[3];
//      
//  
//      Serial.print("PACK_DOD,");Serial.print(packDOD);Serial.print("\n");
//      Serial.print("PACK_HEALTH,");Serial.print(packHealth);Serial.print("\n");
//      Serial.print("PACK_SUMMED_VOLT,");Serial.print(packSummedVolt);Serial.print("\n");
//      
//  }

//  if( canId_BMS == 0x103)
//  {
//      
//      int totalPackCycles     = buff_BMS[2];
//      int currentLimitStatus  = buff_BMS[3];
//      int packCCLKW           = (buff_BMS[4]<<8)|buff_BMS[5];
//      int packDCLKW           = (buff_BMS[6]<<8)|buff_BMS[7];
//  
//      
//      Serial.print("TOTAL_PACKCYCLES,");Serial.print(totalPackCycles);Serial.print("\n");
//      Serial.print("CUR_LIM_STAT,");Serial.print(currentLimitStatus);Serial.print("\n");
//      Serial.print("PACK_CCLKW,");Serial.print(packCCLKW);Serial.print("\n");
//      Serial.print("PACK_DCLKW,");Serial.print(packDCLKW);Serial.print("\n");
//  }

//  if( canId_BMS == 0x104)
//  {
//      int maximumPackDCL    = buff_BMS[0];
//      int maximumPackCCL    = buff_BMS[1];
//      int simulatedSOC      = buff_BMS[2];
//      int simulatedMode     = buff_BMS[3];
//      int simulatedReqMode  = buff_BMS[4];
//      int fanSpeed          = buff_BMS[5];
//      int reqFanSpeed       = buff_BMS[6];
//      
//      Serial.print("MAX_PACK_DCL,");Serial.print(maximumPackDCL);Serial.print("\n");
//      Serial.print("MAX_PACK_CCL,");Serial.print(maximumPackCCL);Serial.print("\n");
//      Serial.print("SIM_SOC,");Serial.print(simulatedSOC);Serial.print("\n");
//      Serial.print("SIM_MODE,");Serial.print(simulatedMode);Serial.print("\n");
//      Serial.print("SIM_REQ_MODE,");Serial.print(simulatedReqMode);Serial.print("\n");
//      Serial.print("FAN_SPEED,");Serial.print(fanSpeed);Serial.print("\n");
//      Serial.print("REQFAN_SPEED,");Serial.print(reqFanSpeed);Serial.print("\n");
//  }

  if( canId_BMS == 0x102)
  {
      int highTemperature   = buff_BMS[0];
      int highThermistorID  = buff_BMS[1];
      int lowTemperature    = buff_BMS[2];
      int lowThermistorID   = buff_BMS[3];
      int avgTemp           = buff_BMS[4];
      int internalTemp      = buff_BMS[5];
      
      Serial.print("HIGH_TEMP,");Serial.print(highTemperature);Serial.print("\n");
      Serial.print("LOW_TEMP,");Serial.print(lowTemperature);Serial.print("\n");
      Serial.print("HIGH_TID,");Serial.print(highThermistorID);Serial.print("\n");
      Serial.print("LOW_TID,");Serial.print(lowThermistorID);Serial.print("\n");
      Serial.print("AVG_TEMP,");Serial.print(avgTemp);Serial.print("\n");
      Serial.print("INT_TEMP,");Serial.print(internalTemp);Serial.print("\n");
  }

//  if( canId_BMS == 0x106)
//  {
//      int highCellVolt  = (buff_BMS[0]<<8)|buff_BMS[1];
//      int highCellVID   = buff_BMS[2];
//      int lowCellVolt   = (buff_BMS[3]<<8)|buff_BMS[4];
//      int lowCellVID    = buff_BMS[5];
//      int avgCellVolt   = (buff_BMS[6]<<8)|buff_BMS[7];
//      
//      Serial.print("HIGH_CELLVOLT,");Serial.print(highCellVolt);Serial.print("\n");
//      Serial.print("LOW_CELLVOLT,");Serial.print(lowCellVolt);Serial.print("\n");
//      Serial.print("HIGH_CELLVID,");Serial.print(highCellVID);Serial.print("\n");
//      Serial.print("LOW_CELLVID,");Serial.print(lowCellVID);Serial.print("\n");
//      Serial.print("AVG_CELLVOLT,");Serial.print(avgCellVolt);Serial.print("\n");
//  }

//  if( canId_BMS == 0x107)
//  {
//      int highOpCellVolt  = (buff_BMS[0]<<8)|buff_BMS[1];
//      int highOpCellVID   = buff_BMS[2];
//      int lowOpCellVolt   = (buff_BMS[3]<<8)|buff_BMS[4];
//      int lowOpCellVID    = buff_BMS[5];
//      int avgOpCellVolt   = (buff_BMS[6]<<8)|buff_BMS[7];
//      
//      Serial.print("HIGH_CELLVOLT,");Serial.print(highOpCellVolt);Serial.print("\n");
//      Serial.print("LOW_CELLVOLT,");Serial.print(lowOpCellVolt);Serial.print("\n");
//      Serial.print("HIGH_CELLVID,");Serial.print(highOpCellVID);Serial.print("\n");
//      Serial.print("LOW_CELLVID,");Serial.print(lowOpCellVID);Serial.print("\n");
//      Serial.print("AVG_CELLVOLT,");Serial.print(avgOpCellVolt);Serial.print("\n");
//  }



//  if (canId_BMS == 0x006) {
//    int averageTemp = (buff_BMS[0]);
//
//    Serial.print("AV_TEMP,");Serial.print(averageTemp);Serial.print("\n");
//    
//  }

    
  if (canId_BMS == 0x036) 
  {
      int cellID = (buff_BMS[0]);
      int byte1 = buff_BMS[1];
      int byte2 = buff_BMS[2];
      unsigned int instVolt = (buff_BMS[1]<<8)|buff_BMS[2];
      int internalResist =  (buff_BMS[3]<<8)|buff_BMS[4];
      int openVolt =  (buff_BMS[5]<<8)|buff_BMS[6];

//      Serial.print("cellID,");Serial.print(cellID);Serial.print("\n");
//      Serial.print("Inst_Volt,");Serial.print(instVolt);Serial.print("\n");
//      Serial.print("Inst_Resist,");Serial.print(internalResist);Serial.print("\n");
//      Serial.print("Open_Volt,");Serial.print(openVolt);Serial.print("\n");

      Serial.print("CELL_INSTVTG,");Serial.print(cellID);Serial.print(",");Serial.print(instVolt);Serial.print("\n");
//      Serial.print("CELL_CONTINUE_BYTE1,");Serial.print(byte1);Serial.print("\n");
//      Serial.print("CELL_CONTINUE_BYTE2,");Serial.print(byte2);Serial.print("\n");
      
  }

//  if (canId_BMS == 0x080)
//  {
//     
//      int thermistorID = (buff_BMS[0]);
//      int temperature = (buff_BMS[1]);
////      int WHAT_80 = (buff_BMS[3]);
//      
//
//        Serial.print("TEMP,");Serial.print(thermistorID);Serial.print(",");Serial.print(temperature);Serial.print("\n");
////        Serial.print("WHAT_BYTE_3,");Serial.print(WHAT_80);Serial.print("\n");
//  }

  if (canId_BMS == 0x081)
  {
     
      int thermistorID = (buff_BMS[0]);
      int temperature = (buff_BMS[1]);
     // int WHAT = (buff_BMS[3]);
      
      Serial.print("TEMP,");Serial.print(thermistorID);Serial.print(",");Serial.print(temperature);Serial.print("\n");
     
      //Serial.print("ID,");Serial.print(ID);Serial.print("\n");
      //Serial.print("TEMP,");Serial.print(TEMP);Serial.print("\n");
      //Serial.print("WHAT,");Serial.print(WHAT);Serial.print("\n");
    
  }

//  if (canId_BMS == 0x082)
//  {
//   
//    int ID_2 = (buff_BMS[1]);
//    int TEMP_2 = (buff_BMS[2]);
//    int WHAT_2 = (buff_BMS[4]);
//    int WHAT_3 = (buff_BMS[5]);
//    int WHAT_4 = (buff_BMS[7]);
//    
//
//   
//    Serial.print("ID_2,");Serial.print(ID_2);Serial.print("\n");
//    Serial.print("TEMP_2,");Serial.print(TEMP_2);Serial.print("\n");
//    Serial.print("BYTE_4,");Serial.print(WHAT_2);Serial.print("\n");
//    Serial.print("BYTE_5,");Serial.print(WHAT_3);Serial.print("\n");
//    Serial.print("BYTE_7,");Serial.print(WHAT_4);Serial.print("\n");
//  
//  }
    
    ////////////////                     Fin BMS  ///////////////////////////////////////////////////

  Serial.flush();
    
    ////////////////////////////////////////////// MPPT 2.0 ////////////////////////////////////////////////////////////

  read2Serial();
   
  Serial.flush();

    /////// Fin Loop ///////
} 

