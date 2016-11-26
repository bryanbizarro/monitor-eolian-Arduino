/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR PRINCIPAL
 *  PERMITE LEER DATOS DE CAN BUS
 *  LECTURA EN TIEMPO REAL DE:
 *  BMS - KELLYs - MPPT
 */

/*  PROTOCOLO DE ENVÍO: 12 bytes
 *  Bytes
 *  ╔  ╗
 *  ║00║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║01║ || 255 - FIJO COMO PROTOCOLO ||
 *  ╠  ╣
 *  ║02║ Reservados para ID de Kelly. Valores:1     ,0 para MPPT, 1 para MOTORES, 2 para BMS
 *  ╠  ╣
 *  ║03║ Reservados para ID de Kelly. Valores:0,1
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

#define recvId1 0x95
#define sendId1 0x96

// Pines de Serial Virtual, librería SoftwareSerial
int rx = 10;
int tx = 11;
SoftwareSerial mySerial(rx, tx); 

// Define PIN usado para comunicacion CAN SHIELD - ARDUINO.
// Por defecto, versiones 0.9b y v1.0 es D10, despues de v1.1 es D9.
const int SPI_CS_PIN = 9;

// Direcciones TX de request Kelly
unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};  // [0]Brake A/D [1]TPS A/D [2]Operation Voltage A/D [3]Vs A/D [4] B+ A/D
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};  // [0]Ia A/D [1]Ib A/D [2]Ic A/D [3]Va A/D [4]Vb A/D [5]Vc A/D
unsigned char CPP_MONITOR1[1] = {0x33};         // [0]PWM [1]EnableMotorRotation [2]MotorTemp [3]ControllerTemp [4]HighSideFETMOSTemp [5]LowSideFETMOSTemp
unsigned char CPP_MONITOR2[1] = {0x37};         // [0]MSB RPM [1]LSB RPM [2]SomeValue [3]MSB ERROR CODE [4]LSB ERROR CODE     //JUNTAR MSB Y LSB PARA RPM
unsigned char COM_SW_ACC[2] = {0x42, 0};        // [0]Current Throttle Switch Status
unsigned char COM_SW_BRK[2] = {0x43, 0};        // [0]Current Brake Switch Status
unsigned char COM_SW_REV[2] = {0x44, 0};        // [0]Current Reverse switch status


// Delay de eco Request/Receive de CAN | Delay entre requests
int del = 5;
int entremensajes = 10;

/// Buff RX inicial
unsigned char flagRecv = 0;
unsigned char len = 0;

//Buff CAN BMS y de entrada RedSerial
unsigned char buff[8];
unsigned char dataToSend[13];

//CanID
unsigned long canId;

///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
  mySerial.begin(57600);
  Serial.begin(57600);  // Iniciar Serial para debug

  //Protocolo
  dataToSend[0]   = 255; //Header
  dataToSend[1]   = 255; //Header
  dataToSend[4]   = 255; //Middle
  dataToSend[13]  = 255; //End

START_INIT:

  if (CAN_OK == CAN.begin(CAN_500KBPS)) //Boud de los Kelly
  {
    Serial.println("CAN BUS Shield Kelly iniciado!");
    dataToSend[2] = 1; //Referente a Kelly
    dataToSend[3] = 30; //Reconocimiento del CAN
  }
  else
  {
    Serial.println("Falla de inicio CAN BUS Shield Kelly");
    Serial.println("Reiniciando CAN BUS Shield Kelly");
    dataToSend[2] = 1; //Referente a Kelly
    dataToSend[3] = 31; // No reconocimiento del CAN
    delay(100);
    goto START_INIT; // Reinicio de conexion
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

      /////////////////// REQUEST Y LECTURA //////////////////////

      // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
      
      CAN.sendMsgBuf(recvId1, 0, 1, CCP_A2D_BATCH_READ1);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      //  flagRecv = 0; //borrar flag
      CAN.readMsgBuf(&len, buff);
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 00;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
      
      delay(entremensajes);
        
      CAN.sendMsgBuf(0x12C, 0, 1, CCP_A2D_BATCH_READ1);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      //  flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 10;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(recvId1, 0, 1, CCP_A2D_BATCH_READ2);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      //flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 01;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(0x12C, 0, 1, CCP_A2D_BATCH_READ2);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      //flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 11;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(recvId1, 0, 1, CPP_MONITOR1);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      //flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 02;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(0x12C, 0, 1, CPP_MONITOR1);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 12;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(recvId1, 0, 1, CPP_MONITOR2);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 03;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(0x12C, 0, 1, CPP_MONITOR2);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 13;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(recvId1, 0, 2, COM_SW_ACC);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 04;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(0x12C, 0, 2, COM_SW_ACC);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 14;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(recvId1, 0, 2, COM_SW_BRK);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 05;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(0x12C, 0, 2, COM_SW_BRK);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 15;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(recvId1, 0, 2, COM_SW_REV);// HEX C8 = DEC 200 = ID Kelly IZQUIERDO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 06;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

      CAN.sendMsgBuf(0x12C, 0, 2, COM_SW_REV);// HEX 12C = DEC 300 = ID Kelly DERECHO
      delay(del);                       // Eco
      
      CAN.readMsgBuf(&len, buff);
      
      if (flagRecv) { //cheque si recibe datos // PROBAR FLAGRECEIVE Y CANID
      flagRecv = 0; //borrar flag
      
        dataToSend[2] = 1;              // Send KELLY ID 
        dataToSend[3] = 16;             // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
        
        for(int j = 5; j < 13; j++){    // Bypass
          dataToSend[j] = buff[j-5];
        }
        SendMsg();
      }
            
      delay(entremensajes);

  Serial.flush();
    /////// Fin Loop ///////
} 

