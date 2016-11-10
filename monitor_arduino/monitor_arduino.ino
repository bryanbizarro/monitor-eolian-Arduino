/*  EOLIAN FENIX - 2016 - UNIVERSIDAD DE CHILE
 *  PROGRAMA MONITOR PRINCIPAL
 *  PERMITE LEER DATOS DE CAN BUS
 *  LECTURA EN TIEMPO REAL DE:
 *  BMS - KELLYs - MPPT
 */

#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>


//  SOFTWARE SERIAL ALLOWS TO READ A SECOND BUFFER SERIAL IN tx & rx PINS

int rx = 10;
int tx = 11;

SoftwareSerial mySerial(rx, tx); 

//  END SOFTWARE SERIAL

/////// KELLY //////////

unsigned char CCP_A2D_BATCH_READ1[1] = {0x1B};
unsigned char CCP_A2D_BATCH_READ2[1] = {0x1A};
unsigned char CPP_MONITOR1[1] = {0x33};
unsigned char CPP_MONITOR2[1] = {0x37};
unsigned char COM_SW_ACC[2] = {0x42, 0};
unsigned char COM_SW_BRK[2] = {0x43, 0};
unsigned char COM_SW_REV[2] = {0x44, 0};
int engineData = 11;

////// END KELLY ///////


// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
////////////////////////////////////////////////////////////////////
/// Tx

int del = 5;
int timi = 0;

/// Rx
unsigned char flagRecv = 0;
unsigned char len = 0;
char str[20];

unsigned long canId;
unsigned char buff[7];

// Read2Serial vars
int MPPTId;
char inChar;
int index=0;
char inData[13];
int charsRead = 0;


/*////////////////// delayTime vars //////////////////*/
long lastTime = 0;




///////////////////////////////////////////////////////////////////

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
  mySerial.begin(9600);
  Serial.begin(57600);

START_INIT:

  if (CAN_OK == CAN.begin(CAN_1000KBPS))                  // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield esta ready papi!!!!");
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


bool read2Serial(){
  while(mySerial.available() || (charsRead < 13)){  //Revisar si es necesario un timeout
    //Serial.println("SerialIsAvailable");
    if(index < 12){
      //Serial.print(index);Serial.println(" < 13");
      inChar = mySerial.read();
      inData[index] = inChar;
      index++;
      //Serial.print("inChar: ");Serial.println(inChar);
    } else {
      inChar = mySerial.read();
      inData[index] = inChar;
      index++;
      //Serial.print("inChar: ");Serial.println(inChar);
      //Serial.println("buff full");
      //Serial.print(inData[0]);Serial.print(inData[1]);Serial.print(inData[2]);Serial.print(inData[3]);
      //Serial.print(inData[4]);Serial.print(inData[5]);Serial.print(inData[6]);Serial.print(inData[7]);
      //Serial.print(inData[8]);Serial.print(inData[9]);Serial.print(inData[10]);Serial.print(inData[11]);
      //Serial.println(inData[12]);
      if((inData[0] == 255) && (inData[1] == 255) && (inData[4] == 255) && (inData[12] == 255)){
        //Serial.println("data ok");
        if(inData[2] == 0){  //REVISAR que igualdad se cumpla!!!
          MPPTId = inData[3];
          index = 0;
          for(int j = 5;j<12;j++){
            buff[index] = inData[j];
          }
          int MPPT_TEMP  = buff[6];
          int  Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
          int  Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
          int Uout  = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
          int BVLR = (bitRead(buff[0],7));
          int OVT  = (bitRead(buff[0],6));
          int NOC  = (bitRead(buff[0],5));
          int UNDV = (bitRead(buff[0],4));
  
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_BVLR,");Serial.print(BVLR);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_OVT,");Serial.print(OVT);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_NOC,");Serial.print(NOC);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UNDV,");Serial.print(UNDV);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_TEMP,");Serial.print(MPPT_TEMP);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UIN,");Serial.print(Uin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_IIN,");Serial.print(Iin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT");Serial.print(MPPTId);Serial.print("_UOUT");Serial.print(Uout);Serial.print("\n");
          delay(timi);
        } else if (inData[2] == 2){
          Serial.print("ARDUINO2_ERROR");
        } else if (inData[2] == 1){
          Serial.print("ARDUINO_ReadyPapi!");
        } else {
          Serial.print("BIT_ERROR_READERROR");
        }
      } else {
        for(int j = 1; j<13;j++){
          inData[j-1] = inData[j];
        }
        index -= 1;
      }
    }
    charsRead++;
  }
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop(){

  
  CAN.readMsgBuf(&len, buff);
  unsigned long canId = CAN.getCanId();

/*////////////////////   LECTURA DE DATOS DE BMS   ////////////////////*/
  
  if (canId == 0x036) // Lectura de Voltajes en tiempo real (solo con veloc de CAN BUS mayor a 256)
  
  {
      int cellID = (buff[0]);
      int byte1 = buff[1];
      int byte2 = buff[2];
      unsigned int instVolt = (buff[1]<<8)|buff[2];
      int internalResist =  (buff[3]<<8)|buff[4];
      int openVolt =  (buff[5]<<8)|buff[6];

      Serial.print("CELL_INSTVTG,");Serial.print(cellID);Serial.print(",");Serial.print(instVolt);Serial.print("\n");
      delay(timi);
  }

  else if (canId == 0x081) // Lectura de temperaturas
  
  {
      int thermistorID = (buff[0]);
      int temperature = (buff[1]);
      
      Serial.print("TEMP,");Serial.print(thermistorID);Serial.print(",");Serial.print(temperature);Serial.print("\n");
      delay(timi);
  }
  else if( canId == 0x100)
  {
      //int packSOC         = buff[0];
      //int packCurrent     = (buff[1]<<8)|buff[2];
      //int packInstVolt    = (buff[3]<<8)|buff[4];
      //int packOpenVolt    = (buff[5]<<8)|buff[6];

      Serial.print("PACK_SOC,");Serial.print(buff[0]);Serial.print("\n");
      delay(timi);
      Serial.print("PACK_CURRENT,");Serial.print((buff[1]<<8)|buff[2]);Serial.print("\n");
      delay(timi);
      Serial.print("PACK_INST_VTG,");Serial.print((buff[3]<<8)|buff[4]);Serial.print("\n");
      delay(timi);
      Serial.print("PACK_OPEN_VTG,");Serial.print((buff[5]<<8)|buff[6]);Serial.print("\n");
      delay(timi);

  }
  else if( canId == 0x101)
  {
      unsigned int packAbsCurrent  = (buff[0]<<8)|buff[1];
      unsigned int maximumPackVolt = (buff[4]<<8)|buff[5];
      unsigned int minimumPackVolt = (buff[6]<<8)|buff[7];

      Serial.print("PACK_ABSCURRENT,");Serial.print(packAbsCurrent);Serial.print("\n");
      delay(timi);
      Serial.print("MAXIM_PACK_VTG,");Serial.print(maximumPackVolt);Serial.print("\n");
      delay(timi);
      Serial.print("MINIM_PACK_VTG,");Serial.print(minimumPackVolt);Serial.print("\n");
      delay(timi);
  }
  else if( canId == 0x102)
  {
      int highTemperature   = buff[0];
      int highThermistorID  = buff[1];
      int lowTemperature    = buff[2];
      int lowThermistorID   = buff[3];
      int avgTemp           = buff[4];
      int internalTemp      = buff[5];
      
      Serial.print("HIGH_TEMP,");Serial.print(highTemperature);Serial.print("\n");
      delay(timi);
      Serial.print("LOW_TEMP,");Serial.print(lowTemperature);Serial.print("\n");
      delay(timi);
      Serial.print("HIGH_TID,");Serial.print(highThermistorID);Serial.print("\n");
      delay(timi);
      Serial.print("LOW_TID,");Serial.print(lowThermistorID);Serial.print("\n");
      delay(timi);
      Serial.print("AVG_TEMP,");Serial.print(avgTemp);Serial.print("\n");
      delay(timi);
      Serial.print("INT_TEMP,");Serial.print(internalTemp);Serial.print("\n");
      delay(timi);
  }
    
/*////////////////////   FIN BMS   ////////////////////*/

/*////////////////////   LECTURA DE DATOS KELLYS   ////////////////////*/

  else if(canId == 0xC8)  // Lectura datos Motor Izquierdo
  {
    if(engineData/10 == 1){         // Si el primer digito de engineData es '1' se proceden a leer corrientes y voltajes.
      Serial.print("I1_A,");Serial.print(buff[0]);Serial.print("\n");
      Serial.print("I1_B,");Serial.print(buff[1]);Serial.print("\n");
      Serial.print("I1_C,");Serial.print(buff[2]);Serial.print("\n");
      Serial.print("V1_A,");Serial.print(buff[3]);Serial.print("\n");
      Serial.print("V1_B,");Serial.print(buff[4]);Serial.print("\n");
      Serial.print("V1_C,");Serial.print(buff[5]);Serial.print("\n");
      engineData = 2*10 + engineData%10;
    } else if(engineData/10 == 2){  // Si el 1er digito de engineData es '2' se procede a leer la temperatura.
      Serial.print("ENG_TEMP_1,");Serial.print(buff[2]);Serial.print("\n");      // Temperatura motor: Celcius
      engineData = 3*10 + engineData%10;
    } else if(engineData/10 == 3){  // Si el 1er digito de engineData es '3' se procede a leer RPM.
      Serial.print("ENG_RPM_1,");Serial.print((buff[0])<<8|buff[1]);Serial.print("\n");
      engineData = 10 + engineData%10;
    }
  }
  
  else if(canId == 0x12C) // Lectura de datos del motor derecho.

  /* 
  Recordar que '%' es 'resto de la división entera'
  Entonces, 12%10 devuelve 2, el resto de 12/10.
  Además, corresponde al último digito del número.
  */
  
  {
    if(engineData%10 == 1){         // 2do digito de engineData es 1, lee corrientes y voltajes
      Serial.print("I2_A,");Serial.print(buff[0]);Serial.print("\n");
      Serial.print("I2_B,");Serial.print(buff[1]);Serial.print("\n");
      Serial.print("I2_C,");Serial.print(buff[2]);Serial.print("\n");
      Serial.print("V2_A,");Serial.print(buff[3]);Serial.print("\n");
      Serial.print("V2_B,");Serial.print(buff[4]);Serial.print("\n");
      Serial.print("V2_C,");Serial.print(buff[5]);Serial.print("\n");
      engineData = engineData/10*10 + 2;
    } else if(engineData%10 == 2){  // 2do digito de engineData es 2, lee temperatura.
      Serial.print("ENG_TEMP_2,");Serial.print(buff[2]);Serial.print("\n");      // Temperatura motor: Celcius
      engineData = engineData/10*10 + 3;
    } else if(engineData%10 == 3){  // 2do digito de engineData es 3, lee RPM.
      Serial.print("ENG_RPM_2,");Serial.print((buff[0])<<8|buff[1]);Serial.print("\n");
      engineData = engineData/10*10 + 1;
    } 
  }

/*////////////////////   FIN KELLYS   ////////////////////*/

/*////////////////////   BLOQUE DE REQUEST KELLYS   ////////////////////*/

  if((millis() - lastTime) > 128){
    if(engineData == 11){
        CAN.sendMsgBuf(0xC7, 0, 1, CCP_A2D_BATCH_READ2);
        CAN.sendMsgBuf(0x6B, 0, 1, CCP_A2D_BATCH_READ2);
    } else if (engineData == 22){
        CAN.sendMsgBuf(0xC7, 0, 1, CPP_MONITOR1);
        CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR1);
    } else {
        CAN.sendMsgBuf(0xC7, 0, 1, CPP_MONITOR2);
        CAN.sendMsgBuf(0x6B, 0, 1, CPP_MONITOR2);
    }
    lastTime = millis();
  }

/*////////////////////   FIN REQUEST KELLYS   ////////////////////*/
    
  Serial.flush();

  if(charsRead > 12){
    charsRead = 0;
  }

    /////// Fin Loop ///////
} 

