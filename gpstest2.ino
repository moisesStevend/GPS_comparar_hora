#include<SoftwareSerial.h>
#include <TimerOne.h>
#include <EEPROM.h>
 
#define RX 4
#define TX 5
#define pinPulso 6 
SoftwareSerial mySerial(RX,TX);

struct Tiempo{
    int h;
    int m;
    int s;
};

volatile Tiempo t_set={EEPROM.read(0),EEPROM.read(1),EEPROM.read(2)};
int intervalo=EEPROM.read(3);//min

void write_eeprom(int h, int m, int s){
  EEPROM.write(0,h);
  EEPROM.write(1,m);
  EEPROM.write(2,s);  
}

int read_eeprom(int i){
  return EEPROM.read(i);
}

void menuSetGPS(int menu){
  if(menu){
    Serial.println("-------------RADIO OBSERVATORIO DE JICAMARCA--------------");
    Serial.println("----------------------MENU GPS----------------------------");
    Serial.println("1) Setear a la hora:");
    Serial.println("2) Obtener hora seteada:");  
    Serial.println("3) Setear intervalo:");
    Serial.println("4) Obtener intervalo seteada:");  
    Serial.println("----------------------------------------------------------");  
  }
  Serial.println("  *)Ingrese Opcion: ");
}

void pulso(int pin, int t){
  digitalWrite(pinPulso,1);
  delay(t);
  digitalWrite(pinPulso,0);  
}

void setup() {
   Timer1.initialize(intervalo*1000000);
   Timer1.attachInterrupt( timerIsr );
   pinMode(pinPulso,OUTPUT);
   Serial.begin(9600);
   mySerial.begin(9600);
   menuSetGPS(true);
}

void comparar_hora(){
  Serial.println("se activo el pulso");
  pulso(pinPulso,1);
}

int pas;
volatile int int_s=0, int_min=0;

void loop() {
  while (1){
    if(Serial.available()>0){
      char op=Serial.read();
      if(op=='\n') {continue;}
      op=int(op-48);//convertir a int
      switch(op){
        case 1:{
          pas=0;
          Serial.println("  *)Ingrese seteo en formato 00:00:00 a continuacion: ");
          do{
              if(Serial.available()>0){
                int hora_s=Serial.parseInt();
                int minuto_s=Serial.parseInt();    
                int segundo_s=Serial.parseInt(); 
                
                if(Serial.read()=='\n'){
                    t_set.h=hora_s;
                    t_set.m=minuto_s;
                    t_set.s=segundo_s;
                    write_eeprom(t_set.h, t_set.m, t_set.s);
                    Serial.println("    !)Seteo "+String(t_set.h)+":"+String(t_set.m)+":"+String(t_set.s)+" correcto!");
                    menuSetGPS(false);
                    break;
                 }else{
                   if(pas){
                     Serial.println("    !)Mal formato ¡ERROR!");
                     menuSetGPS(false);
                     break;
                   }                   
                   pas=1;
                 }
              }//fin de if 
          }while(1);        
          break;
        }
        case 2:{
          Serial.println("    !)Hora seteada a: "+String(t_set.h)+":"+String(t_set.m)+":"+String(t_set.s));
          menuSetGPS(false);
          break;
        }
        case 3:{
          Serial.println("  *)Ingrese numero (en minutos) para el intervalo de comparacion: ");
          if(Serial.read()=='\n'){
            //no esa nada, //flush
          }
          do{
              if(Serial.available()>0){
                int op2=Serial.read();             
                if(op2=='\n') {
                  Serial.println("    !)Intervalo "+String(EEPROM.read(3))+" intervalos");
                  menuSetGPS(false);
                  break;
                }else if(op2==10){
                  continue;
                }
                op2=int(op2-48);//convertir a int
                EEPROM.write(3,op2);    
                Timer1.detachInterrupt();  
                Timer1.initialize(op2*1000000);
                Timer1.attachInterrupt( timerIsr );    
              }//fin de if 
          }while(1);       
          break;
        }//fin de case 3
        case 4:{
          Serial.println("    !)Intervalo seteada a: "+String(EEPROM.read(3)));
          menuSetGPS(false);
          break;
        }
        default :{
          Serial.println("    !)Opcion no valida !ERROR¡ ");
          menuSetGPS(false);
          break;
        }
      } 
    }  
  }
}

void timerIsr(){
  int_s+=1;
  if(int_s==60){
    //pulso(pinPulso,1);//pulso durante 1ms  
    int_s=0;
    int_min+=1;
    if(int_min==EEPROM.read(3)){
      comparar_hora();
      int_s=0;
      int_min=0;
    }      
  }
  
}
