/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR DE MOTORES
 *  SU USO SE RECOMIENDA EN:
 *  - CASO DE FALLA DEL PROGRAMA PRINCIPAL
 *  - SE DESEAN SOLO LECTURAS DE LOS KELLYs
 */

 /*  PROTOCOLO DE ENVÍO: 12 bytes
 *  Bytes
 *  ╔  ╗
 *  ║00║ || ID DATO (Motor) ||
 *  ╠  ╣
 *  ║01║ || TIPO DE DATO ||
 *  ╠  ╣
 *  ║02║ CAN BUFF[0]
 *  ╠  ╣
 *  ║03║ CAN BUFF[1]
 *  ╠  ╣
 *  ║04║ CAN BUFF[2]
 *  ╠  ╣
 *  ║05║ CAN BUFF[3]
 *  ╠  ╣
 *  ║06║ CAN BUFF[4]
 *  ╠  ╣
 *  ║07║ CAN BUFF[5]
 *  ╠  ╣
 *  ║08║ CAN BUFF[6]
 *  ╚  ╝
 */

#define revMode false
 
#include <mcp_can.h>
#include <SPI.h>
#include <SoftEasyTransfer.h>
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
#define pinRX 7
#define pinTX 8
#define radio 0.550
#define pi 3.141592653

SoftwareSerial mySerial(pinRX, pinTX); // RX, TX
SoftEasyTransfer ET; 

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  unsigned char buff[8];
};

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;

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
unsigned char engData[2] = {B10000001,B10000001}; 


int RPM[2] = {0,0};
////// END KELLY ///////

long lastKelly1Time = 0;
long lastKelly2Time = 0;
long lastVelocityTime = 0;

MCP_CAN CAN(SPI_CS_PIN);       

void setup() {
  
  Serial.begin(115200);
  mySerial.begin(115200);

  ET.begin(details(mydata), &mySerial);
  
  
START_INIT:

  if (CAN_OK == CAN.begin(CAN_1000KBPS))                    // Inicia CAN BUS con baudrate de 1000 kbps
  {
    Serial.println("CAN BUS Shield MOTORRES iniciado!");
    mydata.buff[0] = 65;
    mydata.buff[1] = 70;
    Serial.write(mydata.buff,8);Serial.print("\n");
    ET.sendData();
    ET.sendData();
    ET.sendData();
  }
  else
  {
    Serial.println("Falla de inicio CAN BUS Shield MOTORES");
    Serial.println("Reiniciando CAN BUS Shield MOTORES");
    mydata.buff[0] = 65;
    mydata.buff[1] = 75;
    Serial.write(mydata.buff,8);Serial.print("\n");
    ET.sendData();
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

//    Serial.print("| Kelly1: ");Serial.print(millis() - lastKelly1Time);
//    Serial.print(" |  | Kelly2: ");Serial.print(millis() - lastKelly2Time);
//    Serial.print(" |  | engData: ");Serial.print(engData[0],BIN);Serial.print(" | ");Serial.println(engData[1],BIN);

  if(millis() - lastKelly1Time > maxTimi){
    engData[0] = B10000001;
    lastKelly1Time = millis();
  }
  if(millis() - lastKelly2Time > maxTimi){
    engData[1] = B10000001; 
    lastKelly2Time = millis();
  }
  
  
  if(((millis() - lastKelly1Time) > timi) && bitRead(engData[0],7)){
    if((B1111111&engData[0]) == 1){
      CAN.sendMsgBuf(recvIdKelly1, 0, 1, CCP_A2D_BATCH_READ2);
    } else if ((B1111111&engData[0]) == 2){
      CAN.sendMsgBuf(recvIdKelly1, 0, 1, CPP_MONITOR2);
    } else if ((B1111111&engData[0]) == 3){
      CAN.sendMsgBuf(recvIdKelly1, 0, 1, CPP_MONITOR1);
    } else if (((B1111111&engData[0]) == 4) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly1, 0, 2, COM_SW_ACC);
    } else if (((B1111111&engData[0]) == 5) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly1, 0, 2, COM_SW_REV);
    }
    bitWrite(engData[0],7,0);
    lastKelly1Time = millis();
  }

  if(((millis() - lastKelly2Time) > timi) && bitRead(engData[1],7)){
    if((B1111111&engData[1]) == 1){
      CAN.sendMsgBuf(recvIdKelly2, 0, 1, CCP_A2D_BATCH_READ2);
    } else if ((B1111111&engData[1]) == 2){
      CAN.sendMsgBuf(recvIdKelly2, 0, 1, CPP_MONITOR2);
    } else if ((B1111111&engData[1]) == 3){
      CAN.sendMsgBuf(recvIdKelly2, 0, 1, CPP_MONITOR1);
    } else if (((B1111111&engData[1]) == 4) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly2, 0, 2, COM_SW_ACC);
    } else if (((B1111111&engData[1]) == 5) && (revMode == true)){
      CAN.sendMsgBuf(recvIdKelly2, 0, 2, COM_SW_REV);
    }
    bitWrite(engData[1],7,0);
    lastKelly2Time = millis();
  }

  //// LECTURA DATOS KELLYs ////   

  //Serial.print("                               1st engineData = ");Serial.println(engineData);
    
  CAN.readMsgBuf(&len, buff);
  canId = CAN.getCanId();
  
  if((canId == sendIdKelly1) && !bitRead(engData[0],7)){
    //Serial.print(sendIdKelly1);
    mydata.buff[0] = sendIdKelly1;
    if((B1111111&engData[0]) == 1){ 
      
      mydata.buff[1] = 1;        
      for(int j = 0; j < 6; j++){
        mydata.buff[j+2] = buff[j];
      }
      Serial.print("I1_A,");Serial.print(buff[0]);Serial.print(" | ");
      Serial.print("I1_B,");Serial.print(buff[1]);Serial.print(" | ");
      Serial.print("I1_C,");Serial.print(buff[2]);Serial.print(" | ");
      Serial.print("V1_A,");Serial.print(buff[3]/1.84);Serial.print(" | ");
      Serial.print("V1_B,");Serial.print(buff[4]/1.84);Serial.print(" | ");
      Serial.print("V1_C,");Serial.print(buff[5]/1.84);Serial.print("\n");
      engData[0] += 1;
    } else if((B1111111&engData[0]) == 2){  // Si el 1er digito de engineData es '2' se procede a leer RPM.
      mydata.buff[1] = 2;        
      for(int j = 0; j < 5; j++){
        mydata.buff[j+2] = buff[j];
      }
      
      Serial.print("ENG1_RPM,");Serial.print((buff[0])<<8|buff[1]);Serial.print(" | ");
      Serial.print("ENG1_ERR_CODE,");Serial.print((buff[3])<<8|buff[4]);Serial.print("\n");
      RPM[0] = (buff[0])<<8|buff[1];
      engData[0] += 1;
    } else if((B1111111&engData[0]) == 3){                        // Si el 1er digito de engineData es '3' se procede a leer la temperatura.
      mydata.buff[1] = 3;        
      for(int j = 0; j < 6; j++){
        mydata.buff[j+2] = buff[j];
      }
      
      Serial.print("ENG1_PWM,");Serial.print(buff[0]);Serial.print(" | ");
      Serial.print("ENG1_EMR,");Serial.print(buff[1]);Serial.print(" | ");
      Serial.print("ENG1_TEMP,");Serial.print(buff[2]);Serial.print(" | ");      // Temperatura motor: Celcius
      Serial.print("Kelly1_Temp,");Serial.print(buff[3]);Serial.print("\n");
      if(revMode) { 
        engData[0] += 1;
      } else {
        engData[0] = 1;
      }
    } else if((B1111111&engData[0]) == 4){ 

      mydata.buff[1] = 4;        
      for(int j = 0; j < 1; j++){
        mydata.buff[j+2] = buff[j];
      }                            
      Serial.print("ENG1_Current_Throttle_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Throttle Status
      engData[0] += 1;
      
    } else if((B1111111&engData[0]) == 5){     

      mydata.buff[1] = 5;        
      for(int j = 0; j < 1; j++){
        mydata.buff[j+2] = buff[j];
      }
      Serial.print("ENG1_Current_Reserve_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Reverse Status
      engData[0] = 1;
      
    } 
    ET.sendData();
    bitWrite(engData[0],7,1);
  } else if((canId == sendIdKelly2) && !bitRead(engData[1],7)){
    mydata.buff[0] = sendIdKelly2;
    if((B1111111&engData[1]) == 1){         

      mydata.buff[1] = 1;        
      for(int j = 0; j < 6; j++){
        mydata.buff[j+2] = buff[j];
      }

      Serial.print("I2_A,");Serial.print(buff[0]);Serial.print(" | ");
      Serial.print("I2_B,");Serial.print(buff[1]);Serial.print(" | ");
      Serial.print("I2_C,");Serial.print(buff[2]);Serial.print(" | ");
      Serial.print("V2_A,");Serial.print(buff[3]/1.84);Serial.print(" | ");
      Serial.print("V2_B,");Serial.print(buff[4]/1.84);Serial.print(" | ");
      Serial.print("V2_C,");Serial.print(buff[5]/1.84);Serial.print("\n");
      engData[1] += 1;
      
    } else if((B1111111&engData[1]) == 2){  // 2do digito de engineData es 2, lee RPM.

      mydata.buff[1] = 2;        
      for(int j = 0; j < 5; j++){
        mydata.buff[j+2] = buff[j];
      }
      
      Serial.print("ENG2_RPM,");Serial.print((buff[0])<<8|buff[1]);Serial.print(" | ");
      Serial.print("ENG2_ERR_CODE,");Serial.print((buff[3])<<8|buff[4]);Serial.print("\n");
      RPM[1] = (buff[0])<<8|buff[1];
      engData[1] += 1;
      
    } else if((B1111111&engData[1]) == 3){                        // 2do digito de engineData es 3, lee temperatura.

      mydata.buff[1] = 3;        
      for(int j = 0; j < 6; j++){
        mydata.buff[j+2] = buff[j];
      }
      
      Serial.print("ENG2_PWM,");Serial.print(buff[0]);Serial.print(" | ");      // PWM
      Serial.print("ENG2_EMR,");Serial.print(buff[1]);Serial.print(" | ");      // Enable Motor Rotation
      Serial.print("ENG2_TEMP,");Serial.print(buff[2]);Serial.print(" | ");     // Temperatura motor: Celcius
      Serial.print("Kelly2_Temp,");Serial.print(buff[3]);Serial.print("\n");
      
      if(revMode) { 
        engData[1] += 1;
      } else {
        engData[1] = 1;
      }
      
    } else if((B1111111&engData[1]) == 4){  

      mydata.buff[1] = 4;        
      for(int j = 0; j < 1; j++){
        mydata.buff[j+2] = buff[j];
      }
      Serial.print("ENG2_Current_Throttle_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Throttle Status
      engData[1] += 1;
      
    } else if((B1111111&engData[1]) == 5){     

      mydata.buff[1] = 5;        
      for(int j = 0; j < 1; j++){
        mydata.buff[j+2] = buff[j];
      }
      Serial.print("ENG2_Current_Reverse_Switch_Status,");Serial.print(buff[0]);Serial.print("\n");      // Reverse Status
      engData[1] = 1;
      
    } 
    bitWrite(engData[1],7,1);
  }


  //// FIN KELLYs ////

  //// INICIO REQUEST DATOS KELLYs ////


  //// FIN REQUEST ////

  if(millis() - lastVelocityTime > tiempoVelocidad){
    //Serial.print("VELOCIDAD,");Serial.println(getVelocidad());
    lastVelocityTime = millis();
  }
  
   Serial.flush();
 
}
