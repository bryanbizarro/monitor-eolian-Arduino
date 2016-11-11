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

///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
  Serial.begin(115200);

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


void loop()
{

  ////////////////////////// CONTROLLER 1 //////////////////////////////////////////

  
  // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ1);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //cheque si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Brake A/D,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("TPS A/D,");Serial.print(buff[1]);Serial.print("\n");
    Serial.print("Operation voltage A/D,");Serial.print(buff[2]);Serial.print("\n");
    Serial.print("Vs A/D,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("B+ A/D,");Serial.print(buff[4]);Serial.print("\n");
  }

  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Ia A/D,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("Ib A/D,");Serial.print(buff[1]);Serial.print("\n");
    Serial.print("Ic A/D,");Serial.print(buff[2]);Serial.print("\n");
    Serial.print("Va A/D,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("Vb A/D,");Serial.print(buff[4]);Serial.print("\n");
    Serial.print("Vc A/D,");Serial.print(buff[5]);Serial.print("\n");
  }

  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("PWM,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("Enable motor rotation,");Serial.print(buff[1]);Serial.print("\n");  // 1 enable 0 disable
    Serial.print("Motor Temperature,");Serial.print(buff[2]);Serial.print("\n");      // Celcius
    Serial.print("Controller's temperature,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("Temp of HIGH side FETMOS heat sink,");Serial.print(buff[4]);Serial.print("\n");   //Unaccurate below 30C
    Serial.print("Temo of LOW side FETMOS heat sink,");Serial.print(buff[5]);Serial.print("\n");
  }
  
  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("MSB of mechanical speed in RPM,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("LSB of mechanical speed in RPM,");Serial.print(buff[1]);Serial.print("\n");
    Serial.print("Mechanical speed calculation,");Serial.print((buff[0]<<8)|buff[1]);Serial.print("\n");
    Serial.print("Present current accounts for percent of the rated current of controller,");Serial.print(buff[2]);Serial.print("\n");
    Serial.print("MSB of error code,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("LSB of error code,");Serial.print(buff[4]);Serial.print("\n");
    Serial.print("Controller error status,");Serial.print((buff[3]<<8)|buff[4]);Serial.print("\n");   // If = 0x4008 Error code is 0x43 (Bit 6 of [3]) and 0x14(Bit 3 of [4])                    

//    for(int i=1; i<15;i++){
//      int error=(buff[3]<<8)|buff[4];
//      Serial.print("XLS,write,Datos Kelly,B");
//      Serial.print(42+i);
//      Serial.print(",");
//      Serial.print(bitRead(error,i));
//      Serial.print("\n");
//    }
//    
   }

  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_ACC);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Current throttle switch status,");Serial.print(buff[0]);Serial.print("\n"); // 1 active 0 inactive
   }
   
  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_BRK);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Current Brake switch status,");Serial.print(buff[0]);Serial.print("\n");  //1 active 0 inactive
   }

  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_REV);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Current Reverse switch status,");Serial.print(buff[0]);Serial.print("\n");  // 1 active 0 inactive
   }

   ////////////////////////// CONTROLLER 2 //////////////////////////////////////////

  // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ1);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //cheque si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Brake A/D,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("TPS A/D,");Serial.print(buff[1]);Serial.print("\n");
    Serial.print("Operation voltage A/D,");Serial.print(buff[2]);Serial.print("\n");
    Serial.print("Vs A/D,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("B+ A/D,");Serial.print(buff[4]);Serial.print("\n");
  }

  CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Ia A/D,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("Ib A/D,");Serial.print(buff[1]);Serial.print("\n");
    Serial.print("Ic A/D,");Serial.print(buff[2]);Serial.print("\n");
    Serial.print("Va A/D,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("Vb A/D,");Serial.print(buff[4]);Serial.print("\n");
    Serial.print("Vc A/D,");Serial.print(buff[5]);Serial.print("\n");
  }

  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
  //  flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("PWM,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("Enable motor rotation,");Serial.print(buff[1]);Serial.print("\n");  // 1 enable 0 disable
    Serial.print("Motor Temperature,");Serial.print(buff[2]);Serial.print("\n");      // Celcius
    Serial.print("Controller's temperature,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("Temp of HIGH side FETMOS heat sink,");Serial.print(buff[4]);Serial.print("\n");   //Unaccurate below 30C
    Serial.print("Temo of LOW side FETMOS heat sink,");Serial.print(buff[5]);Serial.print("\n");
  }
  
  CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("MSB of mechanical speed in RPM,");Serial.print(buff[0]);Serial.print("\n");
    Serial.print("LSB of mechanical speed in RPM,");Serial.print(buff[1]);Serial.print("\n");
    Serial.print("Mechanical speed calculation,");Serial.print((buff[0]<<8)|buff[1]);Serial.print("\n");
    Serial.print("Present current accounts for percent of the rated current of controller,");Serial.print(buff[2]);Serial.print("\n");
    Serial.print("MSB of error code,");Serial.print(buff[3]);Serial.print("\n");
    Serial.print("LSB of error code,");Serial.print(buff[4]);Serial.print("\n");
    Serial.print("Controller error status,");Serial.print((buff[3]<<8)|buff[4]);Serial.print("\n");   // If = 0x4008 Error code is 0x43 (Bit 6 of [3]) and 0x14(Bit 3 of [4])                    

//    for(int i=1; i<15;i++){
//      int error=(buff[3]<<8)|buff[4];
//      Serial.print("XLS,write,Datos Kelly,B");
//      Serial.print(42+i);
//      Serial.print(",");
//      Serial.print(bitRead(error,i));
//      Serial.print("\n");
//    }
//    
   }

  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_ACC);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Current throttle switch status,");Serial.print(buff[0]);Serial.print("\n"); // 1 active 0 inactive
   }
   
  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_BRK);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Current Brake switch status,");Serial.print(buff[0]);Serial.print("\n");  //1 active 0 inactive
   }

  CAN.sendMsgBuf(0x6B, 0, 2, COM_SW_REV);
  delay(del);                       // send data per 100ms
  if (flagRecv) { //chequea si recibe datos
    //flagRecv = 0; //borrar flag
    CAN.readMsgBuf(&len, buff);
    Serial.print("Current Reverse switch status,");Serial.print(buff[0]);Serial.print("\n");  // 1 active 0 inactive
   }
}
