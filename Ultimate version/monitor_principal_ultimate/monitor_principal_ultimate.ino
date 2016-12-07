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
 *  ║02║ Reservados para ID de BMS. Valores:2     ,0 para MPPT, 1 para MOTORES, 2 para BMS
 *  ╠  ╣
 *  ║03║ Reservados para ID de BMS. Valores:0
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
 #include <SoftwareSerial.h>

//  SOFTWARE SERIAL ALLOWS TO READ A SECOND BUFFER SERIAL IN tx & rx PINS

int rx = 10;
int tx = 11;

SoftwareSerial SerialKelly(rx, tx); 

int del = 5;
int timi = 1;

// Read2Serial vars
int MPPTId;
int KellyId;

char inChar;
char inData[13];
int charsRead = 0;
int index=0;


// MPPTs vars

unsigned int Uin, Iin, Uout;


/*////////////////// delayTime vars //////////////////*/
long lastKellyTime = 0;
long lastMpptTime = 0;




void setup() {
  SerialKelly.begin(9600);
  Serial.begin(57600);
}

void loop() {


  while(SerialKelly.available() || (charsRead < 14)){  //Revisar si es necesario un timeout
    //Serial.println("SerialIsAvailable");
    if(index < 14){
      //Serial.print(index);Serial.println(" < 13");
      inChar = SerialKelly.read();
      inData[index] = inChar;
      index++;
      //Serial.print("inChar: ");Serial.println(inChar);
    }else{
      //inChar = SerialKelly.read();
      //inData[index] = inChar;
      //index++;
      //Serial.print("inChar: ");Serial.println(inChar);
      //Serial.println("buff full");
      //Serial.print(inData[0]);Serial.print(inData[1]);Serial.print(inData[2]);Serial.print(inData[3]);
      //Serial.print(inData[4]);Serial.print(inData[5]);Serial.print(inData[6]);Serial.print(inData[7]);
      //Serial.print(inData[8]);Serial.print(inData[9]);Serial.print(inData[10]);Serial.print(inData[11]);
      //Serial.println(inData[12]);
      if((inData[0] == 255) && (inData[1] == 255) && (inData[4] == 255) && (inData[13] == 255)){
        //Serial.println("data ok");


        // MPPT //
        if(inData[2] == 0){  //REVISAR que igualdad se cumpla!!!
          MPPTId = inData[3]; //0 o 1
          int index1 = 0;
          
          for(int j = 5;j<13;j++){    // Sin byte [13] = 255
            buff[index1] = inData[j];
            index1++;
          }

          if (MPPTId == 30){
            Serial.print("CAN KELLY RECONOCIDO");Serial.print("\n");
          }
          else if (MPPTId == 31){
            Serial.print("CAN KELLY NO RECONOCIDO");Serial.print("\n");
          }
          else if (MPPTId == 0){
            
          int MPPT_TEMP  = buff[6];
          int Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
          int Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
          int Uout = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
          int BVLR = (bitRead(buff[0],7));
          int OVT  = (bitRead(buff[0],6));
          int NOC  = (bitRead(buff[0],5));
          int UNDV = (bitRead(buff[0],4));
  
          Serial.print("MPPT_IZ");Serial.print("_BVLR,");Serial.print(BVLR);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");Serial.print("_OVT,");Serial.print(OVT);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");Serial.print("_NOC,");Serial.print(NOC);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");Serial.print("_UNDV,");Serial.print(UNDV);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");Serial.print("_TEMP,");Serial.print(MPPT_TEMP);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");Serial.print("_UIN,");Serial.print(Uin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");;Serial.print("_IIN,");Serial.print(Iin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_IZ");Serial.print("_UOUT");Serial.print(Uout);Serial.print("\n");
          delay(timi);
          }
          else if (MPPTId == 1){
            
          int MPPT_TEMP  = buff[6];
          int Uin  = ((bitRead(buff[0],1)<<1|bitRead(buff[0],0))<<8)|buff[1];
          int Iin  = ((bitRead(buff[2],1)<<1|bitRead(buff[2],0))<<8)|buff[3];
          int Uout = ((bitRead(buff[4],1)<<1|bitRead(buff[4],0))<<8)|buff[5];
          int BVLR = (bitRead(buff[0],7));
          int OVT  = (bitRead(buff[0],6));
          int NOC  = (bitRead(buff[0],5));
          int UNDV = (bitRead(buff[0],4));
  
          Serial.print("MPPT_DER");Serial.print("_BVLR,");Serial.print(BVLR);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");Serial.print("_OVT,");Serial.print(OVT);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");Serial.print("_NOC,");Serial.print(NOC);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");Serial.print("_UNDV,");Serial.print(UNDV);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");Serial.print("_TEMP,");Serial.print(MPPT_TEMP);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");Serial.print("_UIN,");Serial.print(Uin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");;Serial.print("_IIN,");Serial.print(Iin);Serial.print("\n");
          delay(timi);
          Serial.print("MPPT_DER");Serial.print("_UOUT");Serial.print(Uout);Serial.print("\n");
          delay(timi);
          }
          
        // Fin MPPT //
        
        // Kelly //
        }else if (inData[2] == 1){

          KellyId = inData[3]; // Primer Digito= Kelly iz/der, Segundo Digito = Cual request
          int index1 = 0;
          for(int j = 5;j<13;j++){    // Sin byte [13] = 255
            buff[index1] = inData[j];
            index1++;
          }
          if (KellyId== 30){
            Serial.print("CAN KELLY RECONOCIDO");Serial.print("\n");
          }
          else if (KellyId== 31){
            Serial.print("CAN KELLY NO RECONOCIDO");Serial.print("\n");
          }
          else if (KellyId== 00){          //CCP_A2D_BATCH_READ1 IZQUIERDO
            int Brake = buff[0];
            int TPS = buff[1];
            int OperationVolt = buff[2];
            int Vs = buff[3];
            int Bmas = buff[4];

            Serial.print("Kelly_IZ");Serial.print("Brake");Serial.print(Brake);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("TPS");Serial.print(TPS);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("OperationVolt");Serial.print(OperationVolt);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Vs");Serial.print(Vs);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Bmas");Serial.print(Bmas);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 10){          //CCP_A2D_BATCH_READ1 DERECHO
            int Brake = buff[0];
            int TPS = buff[1];
            int OperationVolt = buff[2];
            int Vs = buff[3];
            int Bmas = buff[4];

            Serial.print("Kelly_DER");Serial.print("Brake");Serial.print(Brake);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("TPS");Serial.print(TPS);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("OperationVolt");Serial.print(OperationVolt);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Vs");Serial.print(Vs);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Bmas");Serial.print(Bmas);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 01){          //CCP_A2D_BATCH_READ2 IZQUIERDO
            int Ia = buff[0];
            int Ib = buff[1];
            int Ic = buff[2];
            int Va = buff[3];
            int Vb = buff[4];
            int Vc = buff[5];

            Serial.print("Kelly_IZ");Serial.print("Ia");Serial.print(Ia);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Ib");Serial.print(Ib);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Ic");Serial.print(Ic);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Va");Serial.print(Va);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Vb");Serial.print(Vb);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("Vc");Serial.print(Vc);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 11){          //CCP_A2D_BATCH_READ2 DERECHO
            int Ia = buff[0];
            int Ib = buff[1];
            int Ic = buff[2];
            int Va = buff[3];
            int Vb = buff[4];
            int Vc = buff[5];

            Serial.print("Kelly_DER");Serial.print("Ia");Serial.print(Ia);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Ib");Serial.print(Ib);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Ic");Serial.print(Ic);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Va");Serial.print(Va);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Vb");Serial.print(Vb);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("Vc");Serial.print(Vc);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 02){          //CCP_MONITOR1 IZQUIERDO
            int PWM = buff[0];
            int EnableMotorRotation = buff[1];
            int MotorTemp = buff[2];
            int ControllerTemp = buff[3];
            int HighSideFETMOSTemp = buff[4];
            int LowSideFETMOSTemp = buff[5];

            Serial.print("Kelly_IZ");Serial.print("PWM");Serial.print(PWM);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("EnableMotorRotation");Serial.print(EnableMotorRotation);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("MotorTemp");Serial.print(MotorTemp);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("ControllerTemp");Serial.print(ControllerTemp);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("HighSideFETMOSTemp");Serial.print(HighSideFETMOSTemp);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("LowSideFETMOSTemp");Serial.print(LowSideFETMOSTemp);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 12){          //CCP_MONITOR1 DERECHO
            int PWM = buff[0];
            int EnableMotorRotation = buff[1];
            int MotorTemp = buff[2];
            int ControllerTemp = buff[3];
            int HighSideFETMOSTemp = buff[4];
            int LowSideFETMOSTemp = buff[5];

            Serial.print("Kelly_DER");Serial.print("PWM");Serial.print(PWM);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("EnableMotorRotation");Serial.print(EnableMotorRotation);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("MotorTemp");Serial.print(MotorTemp);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("ControllerTemp");Serial.print(ControllerTemp);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("HighSideFETMOSTemp");Serial.print(HighSideFETMOSTemp);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("LowSideFETMOSTemp");Serial.print(LowSideFETMOSTemp);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 03){          //CCP_MONITOR2 IZQUIERDO
            int RPM = (buff[0])<<8|buff[1];
            int SomeValue = buff[2];
            int MSB_ERROR_CODE = buff[3];
            int LSB_ERROR_CODE;

            Serial.print("Kelly_IZ");Serial.print("RPM");Serial.print(RPM);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("SomeValue");Serial.print(SomeValue);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("MSB_ERROR_CODE");Serial.print(MSB_ERROR_CODE);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_IZ");Serial.print("LSB_ERROR_CODE");Serial.print(LSB_ERROR_CODE);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 13){          //CCP_MONITOR2 DERECHO
            int RPM = (buff[0])<<8|buff[1];
            int SomeValue = buff[2];
            int MSB_ERROR_CODE = buff[3];
            int LSB_ERROR_CODE;

            Serial.print("Kelly_DER");Serial.print("RPM");Serial.print(RPM);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("SomeValue");Serial.print(SomeValue);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("MSB_ERROR_CODE");Serial.print(MSB_ERROR_CODE);Serial.print("\n");
            delay(timi);
            Serial.print("Kelly_DER");Serial.print("LSB_ERROR_CODE");Serial.print(LSB_ERROR_CODE);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 04){          //CCP_COM_SW_ACC IZQUIERDO
            int CurrentThrottleSwitchStatus = buff[0];
            Serial.print("Kelly_IZ");Serial.print("CurrentThrottleSwitchStatus");Serial.print(CurrentThrottleSwitchStatus);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 14){          //CCP_COM_SW_ACC DERECHO
            int CurrentThrottleSwitchStatus = buff[0];
            Serial.print("Kelly_DER");Serial.print("CurrentThrottleSwitchStatus");Serial.print(CurrentThrottleSwitchStatus);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 05){          //COM_SW_BRK IZQUIERDO
            int CurrentBrakeSwitchStatus = buff[0];
            Serial.print("Kelly_IZ");Serial.print("CurrentBrakeSwitchStatus");Serial.print(CurrentBrakeSwitchStatus);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 15){          //COM_SW_BRK DERECHO
            int CurrentThrottleSwitchStatus = buff[0];
            Serial.print("Kelly_DER");Serial.print("CurrentBrakeSwitchStatus");Serial.print(CurrentBrakeSwitchStatus);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 06){          //COM_SW_REV IZQUIERDO
            int CurrentReverseSwitchStatus = buff[0];
            Serial.print("Kelly_IZ");Serial.print("CurrentReverseSwitchStatus");Serial.print(CurrentReverseSwitchStatus);Serial.print("\n");
            delay(timi);
          }
          else if (KellyId== 16){          //COM_SW_REV DERECHO
            int CurrentReverseSwitchStatus = buff[0];
            Serial.print("Kelly_DER");Serial.print("CurrentReverseSwitchStatus");Serial.print(CurrentReverseSwitchStatus);Serial.print("\n");
            delay(timi);
            
          // Fin Kelly //
          
          } else {
          Serial.print("BIT_ERROR_READERROR");
          }
      } //Fin Kelly
      
      }else{ //Si no lee bien altiro, acumular datos
        for(int j = 0; j<13;j++){
          inData[j] = inData[j+1];
        }
        index -= 1;
      }
      }
    charsRead++;
  } //While Serial available
}//Fin loop
