/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR DE MOTORES
 *  SU USO SE RECOMIENDA EN:
 *  - CASO DE FALLA DEL PROGRAMA PRINCIPAL
 *  - SE DESEAN SOLO LECTURAS DE LOS KELLYs
 */
 
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define pint1 A0
#define pint2 A1
#define timi 100

/* ////////// VARIABLES CAN SHIELD ////////// */

// Define PIN usado para comunicacion CAN SHIELD - ARDUINO.
// Por defecto, despues de v1.1, es D9.
const int SPI_CS_PIN = 9;

unsigned char len = 0;
unsigned char buff[7];

unsigned char flagRecv = 0;
unsigned long canId;


// GAP //

int sensor1 = 0;
int sensor2 = 0;


////////// KELLY //////////

unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};
unsigned char CPP_MONITOR1[1] = {0x33};
unsigned char CPP_MONITOR2[1] = {0x37};
unsigned char COM_SW_ACC[2] = {0x42, 0};
unsigned char COM_SW_BRK[2] = {0x43, 0};
unsigned char COM_SW_REV[2] = {0x44, 0};
int engineData = 11;
////// END KELLY ///////

long lastTime = 0;

MCP_CAN CAN(SPI_CS_PIN);       

void setup() {
  
  Serial.begin(57600);

START_INIT:

  if (CAN_OK == CAN.begin(CAN_1000KBPS))                    // Inicia CAN BUS con baudrate de 1000 kbps
  {
    Serial.println("CAN BUS Shield esta ready papi!!!!!!!!!!!!!!!!!!!!!");
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


void loop() {

  //// LECTURA DATOS GAP ////

  //sensor1 = analogRead(A0);
  //sensor2 = analogRead(A1);

  //Serial.println("GAP_VOLTAJE_1," + sensor1);
  //Serial.println("GAP_VOLTAJE_2," + sensor2);

  //// FIN GAP ////

  //// LECTURA DATOS KELLYs ////    
    
  CAN.readMsgBuf(&len, buff);
  canId = CAN.getCanId();
  
  if(canId == 0xC8){
    if(engineData/10 == 1){
      Serial.print("I1_A,");Serial.print(buff[0]);Serial.print("\n");
      Serial.print("I1_B,");Serial.print(buff[1]);Serial.print("\n");
      Serial.print("I1_C,");Serial.print(buff[2]);Serial.print("\n");
      Serial.print("V1_A,");Serial.print(buff[3]);Serial.print("\n");
      Serial.print("V1_B,");Serial.print(buff[4]);Serial.print("\n");
      Serial.print("V1_C,");Serial.print(buff[5]);Serial.print("\n");
      engineData = 2*10 + engineData%10;
    } else if(engineData/10 == 2){
      Serial.print("ENG_TEMP_1,");Serial.print(buff[2]);Serial.print("\n");      // Temperatura motor: Celcius
      engineData = 3*10 + engineData%10;
    } else if(engineData/10 == 3){
      Serial.print("ENG_RPM_1,");Serial.print((buff[0])<<8|buff[1]);Serial.print("\n");
      engineData = 10 + engineData%10;
    } 
  } else if(canId == 0x12C){
    if(engineData%10 == 1){
      Serial.print("I2_A,");Serial.print(buff[0]);Serial.print("\n");
      Serial.print("I2_B,");Serial.print(buff[1]);Serial.print("\n");
      Serial.print("I2_C,");Serial.print(buff[2]);Serial.print("\n");
      Serial.print("V2_A,");Serial.print(buff[3]);Serial.print("\n");
      Serial.print("V2_B,");Serial.print(buff[4]);Serial.print("\n");
      Serial.print("V2_C,");Serial.print(buff[5]);Serial.print("\n");
      engineData = engineData/10*10 + 2;
    } else if(engineData%10 == 2){
      Serial.print("ENG_TEMP_2,");Serial.print(buff[2]);Serial.print("\n");      // Temperatura motor: Celcius
      engineData = engineData/10*10 + 3;
    } else if(engineData%10 == 3){
      Serial.print("ENG_RPM_2,");Serial.print((buff[0])<<8|buff[1]);Serial.print("\n");
      engineData = engineData/10*10 + 1;
    } 
  }

  //// FIN KELLYs ////

  //// INICIO REQUEST DATOS KELLYs ////

  if((millis() - lastTime) > 256){
    if(engineData == 11){
        CAN.sendMsgBuf(0xC7, 0, 1, CCP_A2D_BATCH_READ2);
        CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
        lastTime = millis();
    } else if (engineData == 22){
        CAN.sendMsgBuf(0xC7, 0, 1, CPP_MONITOR1);
        CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
        lastTime = millis();
    } else if (engineData == 33){
        CAN.sendMsgBuf(0xC7, 0, 1, CPP_MONITOR2);
        CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
        lastTime = millis();
    }
    if(millis() - lastTime > 1024){
      engineData = 11;
    }
      
  }

  //// FIN REQUEST ////
  
   Serial.flush();
 
}