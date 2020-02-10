#include <Adafruit_Sensor.h>
#include <LiquidCrystal.h>
#include "DHT.h"
#define DHT11_PIN 8
#define DHTTYPE DHT11
DHT dht(DHT11_PIN, DHTTYPE);

#define val_status "Stand by"
//****Display Settings****
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
//****DTH11  Settings****
float t; //Variabile temporanea
int h; //Variabile temporanea
//****Led and switch Settings****
int t_set;  //Variabile per impostare la temperatura
int h_set;  //Variabile per impostare l'umidtà 
int p1=11;  //Pulsante 1
int p2=10;  //Pulsante 2
int p3=9;   //Pulsante 3
int lamp=12; //Lampada a irraggiamento
int buzzer=13; 
int vent=A0; //Ventola - utilizzo un piedino analogico come digitale
int s_max;   //Contatore che esprime il tempo massimo in secondi
int check_stop; //Verifico richesta di stop durante il processo di essiccazione
//****hysteresis Settings****
int hyster_vet[] = {1,2,3,4,5,6}; //Vettore con tutti i valori di isteresi
int Hstack=1;  //Variabile temporanea
int hyster=1; //Variabile temporanea
int k; //Contatore
//****START simboli speciali****
byte gradi[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000,
};
byte arrow_left[8] = {
  B01000,
  B01100,
  B01110,
  B01111,
  B01111,
  B01110,
  B01100,
  B01000,
};
byte arrow_right[8] = {
  B00010,
  B00110,
  B01110,
  B11110,
  B11110,
  B01110,
  B00110,
  B00010,
};
byte hy[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00000,
  B00111,
  B00100,
  B11100,
};
byte ok[8] = {
  B00000,
  B00000,
  B00001,
  B00001,
  B10010,
  B01010,
  B00100,
  B00000,
};
byte drying[8] = {
  B00000,
  B01001,
  B01001,
  B10010,
  B10010,
  B00000,
  B11111,
  B00000,
};
byte drying_reverse[8] = {
  B00000,
  B10010,
  B10010,
  B01001,
  B01001,
  B00000,
  B11111,
  B00000,
};
byte fan[8] = {
  B00000,
  B10011,
  B11010,
  B00100,
  B01011,
  B11001,
  B00000,
  B00000,
};
byte fan_reverse[8] = {
  B00000,
  B11001,
  B01011,
  B00100,
  B11010,
  B10011,
  B00000,
  B00000,
};
//****END simboli speciali****

//****START auto-update****
//Funzione di auto-update della temperatura e umidita
void update_val(){
  t= dht.readTemperature(); //Leggo la temperatura
  h= dht.readHumidity(); //Leggo l'umidità
  Serial.println(t);
  Serial.println(h);
}
//****END auto-update****

//*************START-BUZZER**************
void start_beep(){                    // Suono di avvio del sistema
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  delay(300);
  digitalWrite(buzzer,HIGH);
  delay(200);
  digitalWrite(buzzer,LOW);
  }
void saved_beep(){                  // Suono dopo un salvataggio
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  }
void completed_beep(){              // Suono che conferma la fine di una essiccazione
  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  delay(500);
  digitalWrite(buzzer, HIGH);
  delay(800);
  digitalWrite(buzzer, LOW);
  }
void stop_beep(){                  // Suono di stop durante il processo di essiccazione
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(200);
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  }
//**********END BUZZER*****************

//*************START-STOP**************
//Funzione per fermare il processo di essiccazione
void stop_(){
     digitalWrite(lamp,LOW);  //Spengo tutti i dispositivi
     digitalWrite(vent,LOW);
     lcd.clear();
     lcd.setCursor(8, 0);
     lcd.write(byte(4));
     lcd.setCursor(2, 1);
     lcd.print("Drying STOP");
     stop_beep();
     delay(2000);
     loop();                //Ritorno nel loop - Menu principale
  }
//**********END STOP*****************

//*************START-LAYER**************
//LAYER DI AVVIO 
void layer_start(){
  update_val(); //Aggiorno TEMP/UMIDITA
  lcd.clear();
  lcd.print("Temp");
  lcd.setCursor(9, 0); 
  lcd.print(t); 
  lcd.setCursor(14, 0); 
  lcd.write(byte(0));
  lcd.print("C");
  lcd.setCursor(0, 1); 
  lcd.print("Humidity");
  lcd.setCursor(13, 1); 
  lcd.print(h); 
  lcd.print("%");  
  delay(200);
}
//LAYER DEI PROFILI DI ESSICCAZIONE
void layer_profile(){
   String profile[] = {"Default","Profilo 1","Profilo 2","Profilo 3","Profilo 4","Profilo 5","Profilo 6","Profilo 7"}; //VETTORE PROFILI
   int profile_temp[] = {40,30,37,60,42,40,34,50}; //VETTORE TEMPERATURA
   int profile_Humidity[] = {30,10,7,6,5,9,20,25}; //VETTORE UMIDITA'
   int profile_clock[] = {10,20,30,40,50,60,70,80}; //VETTORE TEMPO DI ESSICCAZIONE
   String stack;  
   int cont=0;
  while(digitalRead(p3)==LOW){
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Oven mode");
    lcd.setCursor(0, 1); 
    lcd.write(byte(2));
    lcd.setCursor(15, 1); 
    lcd.write(byte(1));
    lcd.setCursor(2, 1);
    lcd.print(stack);
     if(digitalRead(p2)==HIGH){    //Incremento lo scorrimento dei profili
      if(cont>6){
        }else{
         cont++;
        }
      }
    if(digitalRead(p1)==HIGH){    //Decremento lo scorrimento dei profili
      cont--;
      }
     delay(100);
     //ELENCO DEI PROFILI CON RELATIVI PARAMETRI DI FUNZIONAMENTO
      switch (cont) {
      case 0:
      stack=profile[0];         //Salvo il nome del profilo su una variabile
      t_set=profile_temp[0];    //Salvo la temperatura del profilo su una variabile
      h_set=profile_Humidity[0];  //Salvo l'umidità del profilo su una variabile
      break;
      case 1:
      stack=profile[1];
      t_set=profile_temp[1];
      h_set=profile_Humidity[1];
      break;
      case 2:
      stack=profile[2];
      t_set=profile_temp[2];
      h_set=profile_Humidity[2];
      break;
      case 3:
      stack=profile[3];
      t_set=profile_temp[3];
      h_set=profile_Humidity[3];
      break;
       case 4:
      stack=profile[4];
      t_set=profile_temp[4];
      h_set=profile_Humidity[4];
      break;
      case 5:
      stack=profile[5];
      t_set=profile_temp[5];
      h_set=profile_Humidity[5];
      break;
      case 6:
      stack=profile[6];
      t_set=profile_temp[6];
      h_set=profile_Humidity[6];
      break;
      case 7:
      stack=profile[7];
      t_set=profile_temp[7];
      h_set=profile_Humidity[7];
      break;
      case 8:
      stack=profile[8];
      t_set=profile_temp[8];
      h_set=profile_Humidity[8];
      break;
}
   
  }
  while(digitalRead(p3)==HIGH){ //Aspetto il rilascio del pulsante
  } 
  int s;
  int event_fan,event_temp; //Varibili per le animazioni dei caratteri speciali
  event_fan=0;
  s=profile_clock[cont]; //Salvo il tempo di essiccazione
  for(s_max=s;s_max--; s_max==0){ //Inizio un ciclo for per il processo di essiccazione
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("DryingTime");
  lcd.setCursor(13, 0);
  lcd.print(s_max);
  lcd.setCursor(15, 0);
  lcd.print("s");
  lcd.setCursor(2, 1); 
  lcd.print(t); 
  lcd.setCursor(7, 1); 
  lcd.write(byte(0));
  lcd.print("C");
  lcd.setCursor(13, 1); 
  lcd.print(h); 
  lcd.print("%"); 
  update_val(); //Aggiorno costantemente Temp/Umi
  Serial.println(t); 
  if(t< t_set-hyster){            //Controllo se la temperatura registrata è minore di quella impostata considerando il valore di isteresi
      digitalWrite(lamp,HIGH);
      if(event_temp==0){         //Gestisco le varie animazioni dei caratteri del display
        lcd.setCursor(0, 0);
        lcd.write(byte(5));
        event_temp++;
        }else{
        lcd.setCursor(0, 0);
         lcd.write(byte(6));  
        event_temp--;
       }
    }else{
      digitalWrite(lamp,LOW);  // Se t> t_set-hyster spengo la lampada
      }
  if(h> h_set){                  //La stessa operazione della temperatura la faccio con l'umidità
      digitalWrite(vent,HIGH);
      if(event_fan==0){
        lcd.setCursor(0, 1);
        lcd.write(byte(7));
        event_fan++;
        }else{
        lcd.setCursor(0, 1);
        lcd.write(byte(3));  
        event_fan--;
       }
    }else{
      digitalWrite(vent,LOW);  // Se h> h_set spengo la ventola
      }
  update_val();
  delay(1000);
     if(digitalRead(p3)==HIGH){ //Se il pulsante 3 è alto allora chiedo interruzione sul programma principale e eseguo la funzione di arresto
      s_max=0;
      stop_();
      check_stop=1; //Variabile di segnalazione
    }
  }
  if(check_stop==1){ //Controllo lo stato della rischiesta di stop
    check_stop=0;
  }else{                 //Se non sono stati rischiesti stop e la essiccazione è stata completata con successo spengo tutti i dispositivi e ritorno al menu principale
  digitalWrite(lamp,LOW);
  digitalWrite(vent,LOW);
  lcd.clear();
  lcd.setCursor(8, 0);
  lcd.write(byte(4));
  lcd.setCursor(0, 1);
  lcd.print("Drying completed"); //Messaggio di verifica 
  completed_beep();
  delay(2000);
  }
}

//****LAYER DI ISTERESI****
void layer_hyster(){   
  while(digitalRead(p3)==LOW){
   //CARICO LE INFORMAZIONI DEL SEGUENTE LAYER
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.setCursor(3, 0);
   lcd.print("Hysteresis");
   lcd.setCursor(3, 1);
   lcd.write(byte(2));
   lcd.setCursor(12, 1);
   lcd.write(byte(1));
   lcd.setCursor(7, 1);
   lcd.print(Hstack); //Visualizzo il valore di isteresi
   lcd.setCursor(8, 1);
   lcd.write(byte(0));
   lcd.setCursor(9, 1);
   lcd.print("C");

   if(digitalRead(p2)==HIGH){ //incremento il valore di isteresi
      k++;
      }
    if(digitalRead(p1)==HIGH){ //decremento il valore di isteresi
      k--;
      }
     delay(100);
     //Alenco valori di isteresi che possono essere selezionati
      switch (k) {
      case 0:
      Hstack=hyster_vet[0];
      break;
      case 1:
      Hstack=hyster_vet[1];
      break;
      case 2:
      Hstack=hyster_vet[2];
      break;
      case 3:
      Hstack=hyster_vet[3];
      break;
      case 4:
      Hstack=hyster_vet[4];
      break;
      }
  }
  while(digitalRead(p3)==HIGH){
    hyster==Hstack;    //Salvo il valore di isteresi
    lcd.clear();       //Aggiorno il display per caricare il nuovo valore di isteresi
    lcd.setCursor(8, 0);
    lcd.write(byte(4)); //simbolo "OK"
    lcd.setCursor(0, 1);
    lcd.print("Updated Completed");
    saved_beep();  //conferma salvataggio
    delay(3000);  
  }
}
//*************END-LAYER****************
void setup() {
  Serial.begin(9600);
  //DICHIARO I SEGUENTI DISPOSITIVI: BUZZER, LAMPADA E VENTOLA.
  pinMode(buzzer, OUTPUT);
  pinMode(lamp, OUTPUT);
  pinMode(vent, OUTPUT);
  //PER SICUREZZA VENGONO SPENTI TUTTI GLI ATTUATORI
  digitalWrite(vent,LOW);
  digitalWrite(lamp,LOW);
  lcd.begin(16, 2);  
  dht.begin();
  //DICHIARO I SIMBOLI PERSONALIZZATI DEL DISPLAY LCD
  //ALCUNI SIMBOLO HANNO DUE FRAME, HANNO LA FUNZIONE DI ANIMARE IL SIMBOLO
  lcd.createChar(0, gradi);  //Simbolo per la temperatura
  lcd.createChar(1, arrow_left); //Freccia sinistra di selezione
  lcd.createChar(2, arrow_right); //Freccia destra di selezione
  lcd.createChar(4, ok); //Simbolo di conferma
  lcd.createChar(5, drying); //Simbolo di essiccazione FRAME_1
  lcd.createChar(6, drying_reverse); //Simbolo di essiccazione FRAME_2
  lcd.createChar(7, fan); //Simbolo della ventilazione FRAME_1
  lcd.createChar(3, fan_reverse); //Simbolo della ventilazione FRAME_2
  //INIZIO LA SEQUENZA DI AVVIO
  start_beep();    //Bip iniziale di conferma
  lcd.setCursor(2, 0); 
  lcd.print("Press enter");
  lcd.setCursor(3, 1); 
  lcd.print(val_status); //VAL_STATUS rappresenta lo stato del dispositivo
  while(digitalRead(p3)==LOW){  //Aspetto la pressione del pulsante enter
  }
  while(digitalRead(p3)==HIGH){
  } 
}
void loop() { 
   layer_start();             //Eseguo il seguente layer per mostare la temperatura/umidita
   if(digitalRead(p2)==HIGH){ //Se viene premuto il pulsante 2 entro in "LAYER_PROFILE"
      layer_profile();
  }  
  if(digitalRead(p1)==HIGH){  //Se viene premuto il pulsante 1 entro in "LAYER_HYSTER"
      layer_hyster();
  } 
}
