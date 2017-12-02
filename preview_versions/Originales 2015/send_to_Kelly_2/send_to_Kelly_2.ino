// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
////////////////////////////////////////////////////////////////////
/// Tx
unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};
unsigned char CPP_MONITOR1[1] = {0x33};
unsigned char CPP_MONITOR2[1] = {0x37};
unsigned char COM_SW_ACC[2] = {0x42, 0};
unsigned char COM_SW_BRK[2] = {0x43, 0};
unsigned char COM_SW_REV[2] = {0x44, 0};
int del = 1;
/// Rx
unsigned char flagRecv = 0;
unsigned char len = 0;
char str[20];

unsigned char buff[6];


/// Excel
const int serialBufferSize = 32;      // buffer size for input
char  serialBuffer[serialBufferSize]; // buffer for input
const int serialMaxArgs = 4;          // max CSV message args
char* serialArgs[serialMaxArgs];      // args pointers
int   idx = 0;                        // index
int   outputTiming = 1000;            // packet sending timing in ms      important: this dermines the output timing
float input1;                         // received value
//float inputArray[6];                  // received values
float a0;                             // A0 pin reading
float a1;                             // A1 pin reading
float rnd;                            // random number


///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
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

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop()
{
  // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ1);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //cheque si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B5,");
    Serial.print(buff[0]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B6,");
    Serial.print(buff[1]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B7,");
    Serial.print(buff[2]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B8,");
    Serial.print(buff[3]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B9,");
    Serial.print(buff[4]);
    Serial.print("\n");
  }

  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B12,");
    Serial.print(buff[0]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B13,");
    Serial.print(buff[1]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B14,");
    Serial.print(buff[2]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B15,");
    Serial.print(buff[3]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B16,");
    Serial.print(buff[4]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B17,");
    Serial.print(buff[5]);
    Serial.print("\n");
  }

  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B20,");
    Serial.print(buff[0]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B21,");
    Serial.print(buff[1]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B22,");
    Serial.print(buff[2]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B23,");
    Serial.print(buff[3]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B24,");
    Serial.print(buff[4]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B25,");
    Serial.print(buff[5]);
    Serial.print("\n");
  }
  
  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B28,");
    Serial.print(buff[0]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B29,");
    Serial.print(buff[1]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B30,");
    Serial.print((buff[0]<<8)|buff[1]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B31,");
    Serial.print(buff[2]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B33,");
    Serial.print(buff[3]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B34,");
    Serial.print(buff[4]);
    Serial.print("\n");
    Serial.print("XLS,write,Datos Kelly,B35,");
    Serial.print((buff[3]<<8)|buff[4]);
    Serial.print("\n");

    for(int i=1; i<15;i++){
      int error=(buff[3]<<8)|buff[4];
      Serial.print("XLS,write,Datos Kelly,B");
      Serial.print(42+i);
      Serial.print(",");
      Serial.print(bitRead(error,i));
      Serial.print("\n");
    }
    
   }

  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_ACC);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B37,");
    Serial.print(buff[0]);
    Serial.print("\n");
   }
   
  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_BRK);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B38,");
    Serial.print(buff[0]);
    Serial.print("\n");
   }

  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_REV);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("XLS,write,Datos Kelly,B39,");
    Serial.print(buff[0]);
    Serial.print("\n");
   }

}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
