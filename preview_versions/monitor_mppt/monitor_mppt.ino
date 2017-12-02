/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR MPPT.
 *  PERMITE LEER DATOS MPPT EN CASOS DE:
 *  - FALLA PROGRAMA PRINCIPAL.
 *  - SE REQUIERE SOLO DATOS DE MPPTs.
 */
 
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
////////////////////////////////////////////////////////////////////
/// Tx

int del = 100;
int i   = 0; 

/// Rx
unsigned char flagRecv = 0;
unsigned char len = 0;
char str[20];

unsigned char buff[8];

unsigned char dataToSend[13];

unsigned long canId;
unsigned int Uin, Iin, Uout;

long lastMpptTime = 0;


///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
 // Serial1.begin(115200);
  Serial.begin(9600);   // Iniciar Serial para debug
  dataToSend[0]   = 255; //Header
  dataToSend[1]   = 255; //Header
  dataToSend[4]   = 255; //Middle
  dataToSend[12]  = 255; //END
    
START_INIT:

  if (CAN_OK == CAN.begin(CAN_125KBPS))                
  {
    Serial.println("CAN BUS Shield esta ready papi!");
    dataToSend[2] = 1;
  }
  else
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    dataToSend[2] = 2;
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

void SendMsg(){
  for(unsigned char charToSend:dataToSend){
   // Serial1.write(charToSend);
  }
}

void loop(){
  
  ////////////////////////////////////////////// MPPT1 2.0 ////////////////////////////////////////////////////////////
  CAN.readMsgBuf(&len, buff);
  canId = CAN.getCanId();
  //Serial.print("MPPTId: ANtes ");Serial.println(canId);
  //Serial.print("lastMpptTime: ");Serial.println(lastMpptTime);
  if((millis() - lastMpptTime) > 1024){
    Serial.println("Sending Request to MPPTs");
    CAN.sendMsgBuf(0x711, 0, 0, 0);
    //CAN.sendMsgBuf(0x712, 0, 0, 0);
    lastMpptTime = millis();
  }

  //Serial.print("checkReceive ");Serial.println(CAN.checkReceive());

  if(CAN.checkReceive() != CAN_NOMSG){


    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    CAN.readMsgBuf(&len, buff);
    canId = CAN.getCanId();
    Serial.print("MPPTId: ");Serial.println(canId,HEX);
    if (canId == 0x771){
      
//      dataToSend[2] = 0;
//      dataToSend[3] = 1;
//      for(int j = 5; j < 12; j++){
//        dataToSend[j] = buff[j-5];
//      }
//      SendMsg();
      
      Uin  = (((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1])<<1;
      Iin  = (((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3])<<1;
      Uout  = (((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5])<<1;
      
      Serial.print("MPPT1_BVLR,");Serial.print(bitRead(buff[0],7));Serial.print("\n");
      Serial.print("MPPT1_OVT,");Serial.print(bitRead(buff[0],6));Serial.print("\n");
      Serial.print("MPPT1_NOC,");Serial.print(bitRead(buff[0],5));Serial.print("\n");
      Serial.print("MPPT1_UNDV,");Serial.print(bitRead(buff[0],4));Serial.print("\n");
      Serial.print("MPPT1_UIN,");Serial.print(Uin,DEC);Serial.print(" | ");Serial.print(Uin,BIN);Serial.print(" | ");Serial.print(bitRead(buff[0],1));Serial.print(" ");Serial.print(bitRead(buff[0],0));Serial.print(" ");Serial.print(buff[1],BIN);Serial.print("\n");
      Serial.print("MPPT1_IIN,");Serial.print(Iin,DEC);Serial.print("\n");
      Serial.print("MPPT1_UOUT,");Serial.print(Uout,DEC);Serial.print("\n");
      Serial.print("MPPT1_TAMB,");Serial.print(buff[6]);Serial.print("\n");
    }
  }

    ////////////////////////////////////////////// MPPT2 2.0 ////////////////////////////////////////////////////////////

    /////// Fin Loop ///////
} 



