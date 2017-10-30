/*  EOLIAN FENIX - 2017 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR MPPT.
 *  PERMITE LEER DATOS MPPT
  */

#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 9;
unsigned char len = 0;
unsigned char buff[7];
unsigned long lastMpptTime = 0;
unsigned long actualTime = 0;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  Serial.begin(9600);

  while (CAN_OK != CAN.begin(CAN_125KBPS))
    {
        Serial.println("NO SE HA PODIDO INICIAR CAN BUS");
        Serial.println("INICIANDO NUEVAMENTE");
        delay(500);
    }
  Serial.println("CAN BUS INICIADO");
  Serial.println("PROGRAMA MONITOR MPPT EOLIAN FENIX OK");
}

void loop() {
  actualTime = millis();

  if((actualTime - lastMpptTime) > 500){
    //Serial.println("Sending Request to MPPTs");
    CAN.sendMsgBuf(0x711, 0, 0, 0);
    CAN.sendMsgBuf(0x712, 0, 0, 0);
    CAN.sendMsgBuf(0x713, 0, 0, 0);
    CAN.sendMsgBuf(0x714, 0, 0, 0);
    lastMpptTime = millis();
  }
  
  if(CAN_MSGAVAIL == CAN.checkReceive())
  {
    CAN.readMsgBuf(&len, buff); 

    unsigned int canId = CAN.getCanId();
    Serial.print("ID:");
    Serial.print("\t");
    Serial.print(canId,HEX);
    Serial.print("\t");
    Serial.print("Uin:");
    Serial.print("\t");
    Serial.print((((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1])*150.49);
    Serial.print("\t");
    Serial.print("Iin:");
    Serial.print("\t");
    Serial.print((((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3])*8.72);
    Serial.print("\t");
    Serial.print("Uout:");
    Serial.print((((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5])*208.79);
    Serial.print("\t");

    if(bitRead(buff[0],7) == 1){
      Serial.print("Uout = Umax");
      Serial.print("\t");
    } else {
      Serial.print("Uout < Umax");
      Serial.print("\t");
    }

    if(bitRead(buff[0],6) == 1){
      Serial.print("Tcooler >= 95°C");
      Serial.print("\t");
    } else {
      Serial.print("Tcooler < 95°C");
      Serial.print("\t");
    }

    if(bitRead(buff[0],5) == 1){
      Serial.print("Bateria NO conectada");
      Serial.print("\t");
    } else {
      Serial.print("Batería conectada");
      Serial.print("\t");
    }

    if(bitRead(buff[0],4) == 1){
      Serial.print("Uin <= 26V");
      Serial.print("\t");
    } else {
      Serial.print("Uin > 26V");
      Serial.print("\t");
    }
  }

}
