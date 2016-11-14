/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR MPPT.
 *  PERMITE LEER DATOS MPPT EN CASOS DE:
 *  - FALLA PROGRAMA PRINCIPAL.
 *  - SE REQUIERE SOLO DATOS DE MPPTs.
 */

/*  PROTOCOLO DE ENVÍO: 12 bytes
 *  Bytes
 *  ╔  ╗
 *  ║00║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║01║ 255 - FIJO COMO PROTOCOLO
 *  ╠  ╣
 *  ║02║ Reservados para ID de MPPT. Valores:0
 *  ╠  ╣
 *  ║03║ Reservados para ID de MPPT. Valores:0,1
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
 
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>
int rx = 10;
int tx = 11;

SoftwareSerial mySerial(rx, tx); // RX, TX

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

unsigned char dataToSend[13];

unsigned long canId;
unsigned int Uin, Iin, Uout;


///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
  mySerial.begin(57600);
  Serial.begin(57600);   // Iniciar Serial para debug
  dataToSend[0]   = 255; //Header
  dataToSend[1]   = 255; //Header
  dataToSend[4]   = 255; //Middle
  dataToSend[13]  = 255; //END
    
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
    mySerial.write(charToSend);
  }
}

void loop(){
  
  ////////////////////////////////////////////// MPPT1 2.0 ////////////////////////////////////////////////////////////

    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x711, 0, 0, 0);
    delay(del);
    CAN.readMsgBuf(&len, buff);
    canId = CAN.getCanId();
    if (canId == 0x771){
      
      dataToSend[2] = 0;
      dataToSend[3] = 1;
      for(int j = 5; j < 13; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();

      Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
      Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
      Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
      
      Serial.print("MPPT1_BVLR,");Serial.print(bitRead(buff[0],7));Serial.print("\n");
      Serial.print("MPPT1_OVT,");Serial.print(bitRead(buff[0],6));Serial.print("\n");
      Serial.print("MPPT1_NOC,");Serial.print(bitRead(buff[0],5));Serial.print("\n");
      Serial.print("MPPT1_UNDV,");Serial.print(bitRead(buff[0],4));Serial.print("\n");
      Serial.print("MPPT1_UIN,");Serial.print(Uin);Serial.print("\n");
      Serial.print("MPPT1_IIN,");Serial.print(Iin);Serial.print("\n");
      Serial.print("MPPT1_UOUT,");Serial.print(Uout);Serial.print("\n");
      Serial.print("MPPT1_TAMB,");Serial.print(buff[6]);Serial.print("\n");
    }

    ////////////////////////////////////////////// MPPT2 2.0 ////////////////////////////////////////////////////////////

    // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x712, 0, 0, 0);
    delay(del);
    CAN.readMsgBuf(&len, buff);
    canId = CAN.getCanId();
    if (canId == 0x772){
      dataToSend[2] = 0;
      dataToSend[3] = 2;
      for(int j = 5; j < 13; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();

      Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
      Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
      Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
      
      Serial.print("MPPT2_BVLR,");Serial.print(bitRead(buff[0],7));Serial.print("\n");
      Serial.print("MPPT2_OVT,");Serial.print(bitRead(buff[0],6));Serial.print("\n");
      Serial.print("MPPT2_NOC,");Serial.print(bitRead(buff[0],5));Serial.print("\n");
      Serial.print("MPPT2_UNDV,");Serial.print(bitRead(buff[0],4));Serial.print("\n");
      Serial.print("MPPT2_UIN,");Serial.print(Uin);Serial.print("\n");
      Serial.print("MPPT2_IIN,");Serial.print(Iin);Serial.print("\n");
      Serial.print("MPPT2_UOUT,");Serial.print(Uout);Serial.print("\n");
      Serial.print("MPPT2_TAMB,");Serial.print(buff[6]);Serial.print("\n");
    }

    /////// Fin Loop ///////
} 



