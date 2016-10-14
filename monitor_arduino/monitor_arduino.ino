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
  Serial.begin(9600);

START_INIT:

  if (CAN_OK == CAN.begin(CAN_500KBPS))                  // init can bus : baudrate = 500k
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

  
  if( canId_BMS == 0x101)
  {
      int Voltage_total = (buff_BMS[0]<<8)|buff_BMS[1];
      int Max_Vtg=(buff_BMS[2]<<8)|buff_BMS[3];
      int Min_Vtg=(buff_BMS[4]<<8)|buff_BMS[5];
  
      Serial.print("PACK_VTG,");Serial.print(Voltage_total);Serial.print("\n");
      Serial.print("MAX_VTG,");Serial.print(Max_Vtg*0.1);Serial.print("\n");
      Serial.print("MIN_VTG,");Serial.print(Min_Vtg*0.1);Serial.print("\n");
  }

  if( canId_BMS == 0x102)
  {
      int highTemperature = buff_BMS[0];
      int lowTemperature = buff_BMS[1];
      int highThermistorID = buff_BMS[2];
      int lowThermistorID = buff_BMS[3];
      
      Serial.print("HIGH_TEMP,");Serial.print(highTemperature);Serial.print("\n");
      Serial.print("LOW_TEMP,");Serial.print(lowTemperature);Serial.print("\n");
      Serial.print("HIGH_T_ID,");Serial.print(highThermistorID);Serial.print("\n");
      Serial.print("LOW_T_ID,");Serial.print(lowThermistorID);Serial.print("\n");
  }

  if(canId_BMS == 0x03B)
  {
      signed corriente= (buff_BMS[0]<<8)|buff_BMS[1];      
      
      Serial.print("CURRENT,");Serial.print(corriente);Serial.print("\n");

  } 
    
 if(canId_BMS == 0x3CB){
    int packDCL = buff_BMS[0];
    int packCCL = buff_BMS[1];
    int simulatedSOC = buff_BMS[3];

    Serial.print("PACK_DCL,");Serial.print(packDCL);Serial.print("\n");
    Serial.print("PACK_CCL,");Serial.print(packCCL);Serial.print("\n");
    Serial.print("SIM_SOC,");Serial.print(simulatedSOC);Serial.print("\n");

  } 

  if(canId_BMS == 0x6B2){
    int relayState = buff_BMS[0];
    int packSOC = buff_BMS[1];
    int packResistance = (buff_BMS[2]<<8)|buff_BMS[3];
    int packOpenVtg = (buff_BMS[4]<<8)|buff_BMS[5];
    int packAmphours = buff_BMS[6];

    Serial.print("REL_STATE,");Serial.print(relayState);Serial.print("\n");
    Serial.print("PACK_SOC,");Serial.print(packSOC);Serial.print("\n");
    Serial.print("PACK_RES,");Serial.print(packResistance);Serial.print("\n");
    Serial.print("PACK_OPENVTG,");Serial.print(packOpenVtg);Serial.print("\n");
    Serial.print("PACK_AMPH,");Serial.print(packAmphours);Serial.print("\n");

  }

  if (canId_BMS == 0x02B) {
    int summVolt = (buff_BMS[0]<<8)|buff_BMS[1];
    int averageTemp = (buff_BMS[2]);
    int packcycles = (buff_BMS[3]);
    int high_cell_volt = (buff_BMS[5]<<8)|buff_BMS[6];
    int low_cell_volt = (buff_BMS[7]<<8)|buff_BMS[8];

    Serial.print("SUMM_VOLT,");Serial.print(summVolt);Serial.print("\n");
    Serial.print("AV_TEMP,");Serial.print(averageTemp);Serial.print("\n");
    Serial.print("PACK_CYCLES,");Serial.print(packcycles);Serial.print("\n");
    Serial.print("HIGH_CELL_VOLT,");Serial.print(high_cell_volt);Serial.print("\n");
    Serial.print("LOW_CELL_VOLT,");Serial.print(low_cell_volt);Serial.print("\n");
    
  }

  if (canId_BMS == 0x006) {
    int averageTemp = (buff_BMS[0]);

    Serial.print("AV_TEMP,");Serial.print(averageTemp);Serial.print("\n");
    
  }

    
  if (canId_BMS == 0x036) 
  {
      int cellID = (buff_BMS[0]);
      int instVolt = (buff_BMS[1]<<8)|buff_BMS[2];
      int internalResist =  (buff_BMS[3]<<8)|buff_BMS[4];
      int openVolt =  (buff_BMS[5]<<8)|buff_BMS[6];

      Serial.print("cellID,");Serial.print(cellID);Serial.print("\n");
      Serial.print("Inst_Volt,");Serial.print(instVolt);Serial.print("\n");
      Serial.print("Inst_Resist,");Serial.print(internalResist);Serial.print("\n");
      Serial.print("Open_Volt,");Serial.print(openVolt);Serial.print("\n");
      
  }

  if (canId_BMS == 0x080)
  {
     
      int ID_80 = (buff_BMS[0]);
      int TEMP_80 = (buff_BMS[1]);
      int WHAT_80 = (buff_BMS[3]);
      

     
        Serial.print("ID,");Serial.print(ID_80);Serial.print("\n");
        Serial.print("TEMP,");Serial.print(TEMP_80);Serial.print("\n");
        Serial.print("WHAT_BYTE_3,");Serial.print(WHAT_80);Serial.print("\n");
  }

  if (canId_BMS == 0x081)
  {
     
      int ID = (buff_BMS[0]);
      int TEMP = (buff_BMS[1]);
      int WHAT = (buff_BMS[3]);
      

     
      Serial.print("ID,");Serial.print(ID);Serial.print("\n");
      Serial.print("TEMP,");Serial.print(TEMP);Serial.print("\n");
      Serial.print("WHAT,");Serial.print(WHAT);Serial.print("\n");
    
  }

  if (canId_BMS == 0x082)
  {
   
    int ID_2 = (buff_BMS[1]);
    int TEMP_2 = (buff_BMS[2]);
    int WHAT_2 = (buff_BMS[4]);
    int WHAT_3 = (buff_BMS[5]);
    int WHAT_4 = (buff_BMS[7]);
    

   
    Serial.print("ID_2,");Serial.print(ID_2);Serial.print("\n");
    Serial.print("TEMP_2,");Serial.print(TEMP_2);Serial.print("\n");
    Serial.print("BYTE_4,");Serial.print(WHAT_2);Serial.print("\n");
    Serial.print("BYTE_5,");Serial.print(WHAT_3);Serial.print("\n");
    Serial.print("BYTE_7,");Serial.print(WHAT_4);Serial.print("\n");
  
  }
    
    ////////////////                     Fin BMS  ///////////////////////////////////////////////////

  Serial.flush();
    
    ////////////////////////////////////////////// MPPT 2.0 ////////////////////////////////////////////////////////////

  read2Serial();
   
  Serial.flush();

    /////// Fin Loop ///////
} 

