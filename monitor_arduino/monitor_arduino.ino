// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>

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



///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
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

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop(){
///////////////  BMS///////////////////
  CAN.readMsgBuf(&len, buff_BMS);
  unsigned long canId_BMS = CAN.getCanId();

/*
  if(canId_BMS == 1570){
   Serial.print("XLS,write,DatosMB,G4,");Serial.print(bitRead(buff[0],0));Serial.print("\n");

   Serial.print("XLS,write,BMS,A");Serial.print(idxBMS + 3);Serial.print(",");Serial.print("%date%");Serial.print("\n");
   Serial.print("XLS,write,BMS,B");Serial.print(idxBMS + 3);Serial.print(",");Serial.print("%time%");Serial.print("\n");
   Serial.print("XLS,write,BMS,C");Serial.print(idxBMS + 3);Serial.print(",");Serial.print(idxBMS);Serial.print("\n");
   Serial.print("XLS,write,BMS,D");Serial.print(idxBMS + 3);Serial.print(",");Serial.print(bitRead(buff[0],0));Serial.print("\n");
   
  }

  */
  
    /*if( canId_BMS == 0x101){
      int Voltage_total = (buff_BMS[0]<<8)|buff_BMS[1];
      int Max_Vtg=(buff_BMS[2]<<8)|buff_BMS[3];
      int Min_Vtg=(buff_BMS[4]<<8)|buff_BMS[5];
  
      Serial.print("PACK_VTG,");Serial.print(Voltage_total);Serial.print("\n");
      Serial.print("MAX_VTG,");Serial.print(Max_Vtg*0.1);Serial.print("\n");
      Serial.print("MIN_VTG,");Serial.print(Min_Vtg*0.1);Serial.print("\n");
    }

    if( canId_BMS == 0x102){
      int highTemperature = buff_BMS[0];
      int lowTemperature = buff_BMS[1];
      int highThermistorID = buff_BMS[2];
      int lowThermistorID = buff_BMS[3];
      
      Serial.print("HIGH_TEMP,");Serial.print(highTemperature);Serial.print("\n");
      Serial.print("LOW_TEMP,");Serial.print(lowTemperature);Serial.print("\n");
      Serial.print("HIGH_T_ID,");Serial.print(highThermistorID);Serial.print("\n");
      Serial.print("LOW_T_ID,");Serial.print(lowThermistorID);Serial.print("\n");
    }

    if(canId_BMS == 0x03B){
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
      
    }*/
    if (canId_BMS == 0x036) {
      int cellID = (buff_BMS[0]);
      int instVolt = (buff_BMS[1]<<8)|buff_BMS[2];
      int internalResist =  (buff_BMS[3]<<8)|buff_BMS[4];
      int openVolt =  (buff_BMS[5]<<8)|buff_BMS[6];

      Serial.print("cellID,");Serial.print(cellID);Serial.print("\n");
      Serial.print("Inst_Volt,");Serial.print(instVolt);Serial.print("\n");
      Serial.print("Inst_Resist,");Serial.print(internalResist);Serial.print("\n");
      Serial.print("Open_Volt,");Serial.print(openVolt);Serial.print("\n");
      
    }

Serial.print("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
Serial.print("\n");

        if (canId_BMS == 0x080) {
     
      int ID_80 = (buff_BMS[0]);
      int TEMP_80 = (buff_BMS[1]);
      int WHAT_80 = (buff_BMS[3]);
      

     
      Serial.print("ID,");Serial.print(ID_80);Serial.print("\n");
      Serial.print("TEMP,");Serial.print(TEMP_80);Serial.print("\n");
       Serial.print("WHAT_BYTE_3,");Serial.print(WHAT_80);Serial.print("\n");
        }

if (canId_BMS == 0x081) {
     
      int ID = (buff_BMS[0]);
      int TEMP = (buff_BMS[1]);
      int WHAT = (buff_BMS[3]);
      

     
      Serial.print("ID,");Serial.print(ID);Serial.print("\n");
      Serial.print("TEMP,");Serial.print(TEMP);Serial.print("\n");
       Serial.print("WHAT,");Serial.print(WHAT);Serial.print("\n");
    
    }

    
    
    if (canId_BMS == 0x082) {
     
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

/*
  
///////////////////////////// MPPT 1 //////////////////////////////////////////////////////////////////////
    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
  //CAN.sendMsgBuf(0x711, 0, 0, 0);
  ///delay(del);
  unsigned long canId1 = CAN.getCanId();                      // send data per 100ms
  if (canId1 == 0x771) 
  {
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
        
    int  Uin = ((bitRead(buff[0],1)<<1|bitRead(buff[0],1))<<8)|buff[1];
    int  Iin = ((bitRead(buff[2],1)<<1|bitRead(buff[2],1))<<8)|buff[3];
    int Uout = ((bitRead(buff[4],1)<<1|bitRead(buff[4],1))<<8)|buff[5];
    
    Serial.print("XLS,write,DatosMB,C4,");Serial.print(bitRead(buff[0],7));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C5,");Serial.print(bitRead(buff[0],6));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C6,");Serial.print(bitRead(buff[0],5));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C7,");Serial.print(bitRead(buff[0],4));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C8,");Serial.print(buff[6]);Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C10,");Serial.print(Uin);Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C11,");Serial.print(Iin);Serial.print("\n");
    Serial.print("XLS,write,DatosMB,C12,");Serial.print(Uout);Serial.print("\n");

    Serial.print("XLS,write,Ms,A");Serial.print(idxM1 + 3);Serial.print(",");Serial.print("%date%");Serial.print("\n");
    Serial.print("XLS,write,Ms,B");Serial.print(idxM1 + 3);Serial.print(",");Serial.print("%time%");Serial.print("\n");
    Serial.print("XLS,write,Ms,C");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(idxM1);Serial.print("\n");

    Serial.print("XLS,write,Ms,D");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(bitRead(buff[0],7));Serial.print("\n");
    Serial.print("XLS,write,Ms,E");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(bitRead(buff[0],6));Serial.print("\n");
    Serial.print("XLS,write,Ms,F");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(bitRead(buff[0],5));Serial.print("\n");
    Serial.print("XLS,write,Ms,G");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(bitRead(buff[0],4));Serial.print("\n");
    Serial.print("XLS,write,Ms,H");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(buff[6]);Serial.print("\n");
    Serial.print("XLS,write,Ms,I");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(Uin);Serial.print("\n");
    Serial.print("XLS,write,Ms,J");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(Iin);Serial.print("\n");
    Serial.print("XLS,write,Ms,K");Serial.print(idxM1 + 3);Serial.print(",");Serial.print(Uout);Serial.print("\n");

    idxM1++;
    delay(del);   
  } ////  Fin MPPT_1   /////////////////////////////////////////////////////////////////////////////////////////


CAN.sendMsgBuf(0x712, 0, 0, 0);
///delay(del);
  unsigned long canId2 = CAN.getCanId();                       // send data per 100ms
  if (canId2 == 0x772);
  { //cheque si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    int  Uin = ((bitRead(buff[0],1)<<1|bitRead(buff[0],1))<<8)|buff[1];
    int  Iin = ((bitRead(buff[2],1)<<1|bitRead(buff[2],1))<<8)|buff[3];
    int Uout = ((bitRead(buff[4],1)<<1|bitRead(buff[4],1))<<8)|buff[5];
    
    Serial.print("XLS,write,DatosMB,D4,");Serial.print(bitRead(buff[0],7));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D5,");Serial.print(bitRead(buff[0],6));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D6,");Serial.print(bitRead(buff[0],5));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D7,");Serial.print(bitRead(buff[0],4));Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D8,");Serial.print(buff[6]);Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D10,");Serial.print(Uin);Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D11,");Serial.print(Iin);Serial.print("\n");
    Serial.print("XLS,write,DatosMB,D12,");Serial.print(Uout);Serial.print("\n");

    Serial.print("XLS,write,Ms,M");Serial.print(idxM2 + 3);Serial.print(",");Serial.print("%date%");Serial.print("\n");
    Serial.print("XLS,write,Ms,N");Serial.print(idxM2 + 3);Serial.print(",");Serial.print("%time%");Serial.print("\n");
    Serial.print("XLS,write,Ms,O");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(idxM1);Serial.print("\n");

    Serial.print("XLS,write,Ms,P");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(bitRead(buff[0],7));Serial.print("\n");
    Serial.print("XLS,write,Ms,Q");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(bitRead(buff[0],6));Serial.print("\n");
    Serial.print("XLS,write,Ms,R");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(bitRead(buff[0],5));Serial.print("\n");
    Serial.print("XLS,write,Ms,S");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(bitRead(buff[0],4));Serial.print("\n");
    Serial.print("XLS,write,Ms,T");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(buff[6]);Serial.print("\n");
    Serial.print("XLS,write,Ms,U");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(Uin);Serial.print("\n");
    Serial.print("XLS,write,Ms,V");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(Iin);Serial.print("\n");
    Serial.print("XLS,write,Ms,W");Serial.print(idxM2 + 3);Serial.print(",");Serial.print(Uout);Serial.print("\n");

    idxM2++;
    delay(del);
  } //// Fin MPPT_2  ///////////////////////////////////////////////////////////////////////////////////////////////////

*/


  
} ///////   Fin  Loop //////

