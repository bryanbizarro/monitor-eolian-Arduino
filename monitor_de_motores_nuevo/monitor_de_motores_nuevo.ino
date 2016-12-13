/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR DE MOTORES
 *  SU USO SE RECOMIENDA EN:
 *  - CASO DE FALLA DEL PROGRAMA PRINCIPAL
 *  - SE DESEAN SOLO LECTURAS DE LOS KELLYs
 */

 /*  PROTOCOLO DE ENVÍO: 12 bytes
 *  Bytes
 *  ╔  ╗
 *  ║00║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║01║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║02║ Reservados para ID de MOTORES. Valores:1     ,0 para MPPT, 1 para MOTORES, 2 para BMS
 *  ╠  ╣
 *  ║03║ Reservados para ID de MOTORES. Valores:0,1
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

#define revMode false
 
#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define pint1 A0
#define pint2 A1
#define timi 512
#define maxTimi 2048
#define tiempoVelocidad 1000
#define recvIdKelly1 0xC8
#define sendIdKelly1 0xD2
#define recvIdKelly2 0xC9
#define sendIdKelly2 0xD3
#define pinRX 10
#define pinTX 11
#define radio 0.550
#define pi 3.141592653

SoftwareSerial mySerial(pinRX, pinTX); // RX, TX
unsigned char dataToSend[13];

/* ////////// VARIABLES CAN SHIELD ////////// */

// Define PIN usado para comunicacion CAN SHIELD - ARDUINO.
// Por defecto, versiones 0.9b y v1.0 es D10, despues de v1.1 es D9.
const int SPI_CS_PIN = 9;

unsigned char len = 0;
unsigned char buff[7];

unsigned char flagRecv = 0;
unsigned long canId;


// GAP //

int sensor1 = 0;
int sensor2 = 0;


////////// KELLY //////////

unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};  // [0]Brake A/D [1]TPS A/D [2]Operation Voltage A/D [3]Vs A/D [4] B+ A/D
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};  // [0]Ia A/D [1]Ib A/D [2]Ic A/D [3]Va A/D [4]Vb A/D [5]Vc A/D
unsigned char CPP_MONITOR1[1] = {0x33};         // [0]PWM [1]EnableMotorRotation [2]MotorTemp [3]ControllerTemp [4]HighSideFETMOSTemp [5]LowSideFETMOSTemp
unsigned char CPP_MONITOR2[1] = {0x37};         // [0]MSB RPM [1]LSB RPM [2]SomeValue [3]MSB ERROR CODE [4]LSB ERROR CODE     //JUNTAR MSB Y LSB PARA RPM
unsigned char COM_SW_ACC[2] = {0x42, 0};        // [0]Current Throttle Switch Status
unsigned char COM_SW_BRK[2] = {0x43, 0};        // [0]Current Brake Switch Status
unsigned char COM_SW_REV[2] = {0x44, 0};        // [0]Current Reverse switch status

int engineData = 11;
byte engData[2] = {B10000001,B10000001}; 
int RPM[2] = {0,0};
////// END KELLY ///////

long lastKelly1Time = 0;
long lastKelly2Time = 0;
long lastVelocityTime = 0;

MCP_CAN CAN(SPI_CS_PIN);       

void setup() {
  
  Serial.begin(57600);
  mySerial.begin(115200);
  dataToSend[0]   = 255; //Header
  dataToSend[1]   = 255; //Header
  dataToSend[4]   = 255; //Middle
  dataToSend[12]  = 255; //END

START_INIT:

  if (CAN_OK == CAN.begin(CAN_1000KBPS))                    // Inicia CAN BUS con baudrate de 1000 kbps
  {
    Serial.println("CAN BUS Shield MOTORRES iniciado!");
  }
  else
  {
    Serial.println("Falla de inicio CAN BUS Shield MOTORES");
    Serial.println("Reiniciando CAN BUS Shield MOTORES");
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

double getVelocidad(){
  double meanRPM = (RPM[0] + RPM[1])/2;
  return 6*3.141592653*radio*meanRPM;
}

void loop() {

  //// LECTURA DATOS GAP ////

  //sensor1 = analogRead(A0);
  //sensor2 = analogRead(A1);

  //Serial.println("GAP_VOLTAJE_1," + sensor1);
  //Serial.println("GAP_VOLTAJE_2," + sensor2);

  //// FIN GAP ////

  if(millis() - lastKelly1Time > maxTimi){
    engData[0] = 1;
    lastKelly1Time = millis();
  }
  if(millis() - lastKelly2Time > maxTimi){
    engData[1] = 1; 
    lastKelly1Time = millis();
  }
  //Serial.print("kelly1 ");Serial.print(millis() - lastKelly1Time);Serial.print("  kelly2 ");Serial.print(millis() - lastKelly2Time);Serial.print("  engineData ");Serial.println(engineData);
  
  if(((millis() - lastKelly1Time) > timi) && bitRead(engData[0],8)){
    if((1111111&engData[0]) == 1){
      CAN.sendMsgBuf(recvIdKelly1, 0, 1, CCP_A2D_BATCH_READ2);
    } else if ((1111111&engData[0]) == 2){
      CAN.sendMsgBuf(recvIdKelly1, 0, 1, CPP_MONITOR2);
    } else if ((1111111&engData[0]) == 3){
      CAN.sendMsgBuf(recvIdKelly1, 0, 1, CPP_MONITOR1);
    } else if (((1111111&engData[0]) == 4) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly1, 0, 2, COM_SW_ACC);
    } else if (((1111111&engData[0]) == 5) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly1, 0, 2, COM_SW_REV);
    }
    bitWrite(engData[0],8,0);
    lastKelly1Time = millis();
  }

  if(((millis() - lastKelly2Time) > timi) && bitRead(engData[1],8)){
    if((1111111&engData[1]) == 1){
      CAN.sendMsgBuf(recvIdKelly2, 0, 1, CCP_A2D_BATCH_READ2);
    } else if ((1111111&engData[1]) == 2){
      CAN.sendMsgBuf(recvIdKelly2, 0, 1, CPP_MONITOR2);
    } else if ((1111111&engData[1]) == 3){
      CAN.sendMsgBuf(recvIdKelly2, 0, 1, CPP_MONITOR1);
    } else if (((1111111&engData[1]) == 4) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly2, 0, 2, COM_SW_ACC);
    } else if (((1111111&engData[1]) == 5) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly2, 0, 2, COM_SW_REV);
    }
    bitWrite(engData[1],8,0);
    lastKelly2Time = millis();
  }

  //// LECTURA DATOS KELLYs ////   

  //Serial.print("                               1st engineData = ");Serial.println(engineData);
    
  CAN.readMsgBuf(&len, buff);
  canId = CAN.getCanId();
  
  if((canId == sendIdKelly1) && !bitRead(engData[0],8)){
    if((1111111&engData[0]) == 1){         
      dataToSend[2] = 1;
      dataToSend[3] = 01;
      for(int j = 5; j < 12; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();
      Serial.print("I1_A,");Serial.print(buff[0]);Serial.print(" | ");
      Serial.print("I1_B,");Serial.print(buff[1]);Serial.print(" | ");
      Serial.print("I1_C,");Serial.print(buff[2]);Serial.print(" | ");
      Serial.print("V1_A,");Serial.print(buff[3]/1.84);Serial.print(" | ");
      Serial.print("V1_B,");Serial.print(buff[4]/1.84);Serial.print(" | ");
      Serial.print("V1_C,");Serial.print(buff[5]/1.84);Serial.print("\n");
      engData[0] += 1;
    } else if((1111111&engData[0]) == 2){  // Si el 1er digito de engineData es '2' se procede a leer RPM.

      dataToSend[2] = 1;
      dataToSend[3] = 02;
      for(int j = 5; j < 12; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();
      
      Serial.print("ENG1_RPM,");Serial.print((buff[0])<<8|buff[1]);Serial.print(" | ");
      Serial.print("ENG1_ERR_CODE,");Serial.print((buff[3])<<8|buff[4]);Serial.print("\n");
      RPM[0] = (buff[0])<<8|buff[1];
      engData[0] += 1;
    } else if((1111111&engData[0]) == 3){                        // Si el 1er digito de engineData es '3' se procede a leer la temperatura.

      dataToSend[2] = 1;
      dataToSend[3] = 03;
      for(int j = 5; j < 12; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();
      
      Serial.print("ENG1_PWM,");Serial.print(buff[0]);Serial.print(" | ");
      Serial.print("ENG1_EMR,");Serial.print(buff[1]);Serial.print(" | ");
      Serial.print("ENG1_TEMP,");Serial.print(buff[2]);Serial.print(" | ");      // Temperatura motor: Celcius
      Serial.print("Kelly1_Temp,");Serial.print(buff[3]);Serial.print("\n");
      if(revMode) { 
        engData[0] += 1;
      } else {
        engData[0] = 1;
      }
    } else if((1111111&engData[0]) == 4){  
                            
      Serial.print("ENG1_Current_Throttle_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Throttle Status
      engData[0] += 1;
      
    } else if((1111111&engData[0]) == 5){     
                         
      Serial.print("ENG1_Current_Reserve_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Reverse Status
      engData[0] = 1;
      
    } 
    bitWrite(engData[0],8,1);
  } else if((canId == sendIdKelly2) && !bitRead(engData[1],8)){
    if((1111111&engData[1]) == 1){         

      dataToSend[2] = 1;
      dataToSend[3] = 11;
      for(int j = 5; j < 12; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();

      Serial.print("I2_A,");Serial.print(buff[0]);Serial.print(" | ");
      Serial.print("I2_B,");Serial.print(buff[1]);Serial.print(" | ");
      Serial.print("I2_C,");Serial.print(buff[2]);Serial.print(" | ");
      Serial.print("V2_A,");Serial.print(buff[3]/1.84);Serial.print(" | ");
      Serial.print("V2_B,");Serial.print(buff[4]/1.84);Serial.print(" | ");
      Serial.print("V2_C,");Serial.print(buff[5]/1.84);Serial.print("\n");
      engData[1] += 1;
      
    } else if((1111111&engData[1]) == 2){  // 2do digito de engineData es 2, lee RPM.

      dataToSend[2] = 1;
      dataToSend[3] = 12;
      for(int j = 5; j < 12; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();
      
      Serial.print("ENG2_RPM,");Serial.print((buff[0])<<8|buff[1]);Serial.print(" | ");
      Serial.print("ENG2_ERR_CODE,");Serial.print((buff[3])<<8|buff[4]);Serial.print("\n");
      RPM[1] = (buff[0])<<8|buff[1];
      engData[1] += 1;
      
    } else if((1111111&engData[1]) == 3){                        // 2do digito de engineData es 3, lee temperatura.

      dataToSend[2] = 1;
      dataToSend[3] = 13;
      for(int j = 5; j < 12; j++){
        dataToSend[j] = buff[j-5];
      }
      SendMsg();
      
      Serial.print("ENG2_PWM,");Serial.print(buff[0]);Serial.print(" | ");      // PWM
      Serial.print("ENG2_EMR,");Serial.print(buff[1]);Serial.print(" | ");      // Enable Motor Rotation
      Serial.print("ENG2_TEMP,");Serial.print(buff[2]);Serial.print(" | ");     // Temperatura motor: Celcius
      Serial.print("Kelly2_Temp,");Serial.print(buff[3]);Serial.print("\n");
      
      if(revMode) { 
        engData[1] += 1;
      } else {
        engData[1] = 1;
      }
      
    } else if((1111111&engData[1]) == 4){  
                            
      Serial.print("ENG2_Current_Throttle_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Throttle Status
      engData[1] += 1;
      
    } else if((1111111&engData[1]) == 5){     
                         
      Serial.print("ENG2_Current_Reverse_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Reverse Status
      engData[1] = 1;
      
    } 
    bitWrite(engData[1],8,1);
  }


  //// FIN KELLYs ////

  //// INICIO REQUEST DATOS KELLYs ////


  //// FIN REQUEST ////

  if(millis() - lastVelocityTime > tiempoVelocidad){
    Serial.print("VELOCIDAD,");Serial.println(getVelocidad());
    lastVelocityTime = millis();
  }
  
   Serial.flush();
 
}
