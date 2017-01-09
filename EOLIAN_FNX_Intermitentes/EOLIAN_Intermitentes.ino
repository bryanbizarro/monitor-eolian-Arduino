
#define botonIzq 2
#define botonDer 3
#define luzIzq 4
#define luzDer 5

bool intIzq = false, intDer = false;
long lastIzqTime = 0, lastDerTime = 0;

void setup() {
  pinMode(botonIzq,INPUT);
  pinMode(botonDer,INPUT);
  pinMode(luzIzq,OUTPUT);
  pinMode(luzDer,OUTPUT);
}

void loop() {
  intIzq = digitalRead(botonIzq);
  intDer = digitalRead(botonDer);
  if(intIzq){
    if((millis() - lastIzqTime) > 1000){
      if(!intIzq){
        digitalWrite(luzIzq, HIGH);
        lastIzqTime = millis();
      } else {
        digitalWrite(luzIzq,LOW);
        lastIzqTime = millis();
      }
    } else if(lastIzqTime < 1000){
      digitalWrite(luzIzq,HIGH);
      lastIzqTime = millis();
    }
  } else {
    digitalWrite(luzIzq, LOW);
  }
  if(intDer){
    if((millis() - lastDerTime) > 1000){
      if(!intDer){
        intDer = true;
        digitalWrite(luzDer, HIGH);
      } else {
        intDer = false;
        digitalWrite(luzDer,LOW);
      }
      lastDerTime = millis();
    } else if(lastDerTime < 1000){
      digitalWrite(luzDer,HIGH);
      lastDerTime = millis();
    }
  } else {
    digitalWrite(luzDer, LOW);
  }

}
