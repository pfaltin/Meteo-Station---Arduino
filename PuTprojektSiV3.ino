#include <LiquidCrystal.h>
LiquidCrystal lcd(30, 31, 35, 34, 33, 32);

// anemo
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
int anmeoMetarPIN = 3;
int anmeoMetarVrijednost = 0;


//smjer

int analogInA = A6;  // Analog input pin a hall izlaz
int analogInB = A7;  // Analog input pin b hall izlaz
int a, b, smjer, aMax, aMin, bMax, bMin;
float a_nul,b_nul, b_k, stupanjA, stupanjB;
char smjerTxt[3];

// baro

int baroSenzor = A5;   // odabir pina za očitanje senzora Motorola MPX4100A
int tlakIz = 0;        // varijabla za izračunati tlak


// - - - tremometar DS1820
#include <OneWire.h>
OneWire  ds(8);  // pin 8 , 4.7K otpornik između D i Vcc
float temperatura;

// slanje podataka u easyweather.dat  
unsigned long izvj=0;//broj poslanog izvjestaja
int brzina[600], brzinaIndex,brzinaSred,brzinaUdar; //niz za spremanje vrijednosti brzina, 




// cekalica
unsigned long vrijeme, vrijeme1, vrijeme2;



void setup(){
   lcd.begin(16, 2);
  Serial.begin(9600);
  // smjer
    aMax=600; 
    aMin=400;
    bMax=600;
    bMin=400;

  //anemoMetar 
  pinMode(anmeoMetarPIN, INPUT);
  attachInterrupt(1, ISR_anemoMetar, RISING);
  brzinaIndex=0;

}

void loop()
{

    // - - - anemoMetar - - -
  //static variables for wind sensor
  static int anmeoMetarVrijednost_OBRT;
  static long cekajDo;  
 

   if (millis() >= cekajDo) {
      
      anmeoMetarVrijednost_OBRT=anmeoMetarVrijednost;
      //reset the value
      anmeoMetarVrijednost = 0;
      cekajDo = millis() + 1000;   // Make sure we wait for 1 minute.
   }
  if (brzinaIndex < sizeof(brzina)){
      brzinaIndex++;}  
    else{
    brzinaIndex=0;
    }
    

// - - - kraj anemoMetar - - -
   
//- - - smjer - - 

//citanje hall senzora
a=analogRead(A6);
b=analogRead(A7);
//kalibracija max/min vrijednosti
if (a>aMax) aMax=a;
if (b>bMax) bMax=b;
if (a<aMin) aMin=a;
if (b<bMin) bMin=b;
//određivanje srednjih
a_nul = aMax-(aMax-aMin)/2;
b_nul = bMax-(bMax-bMin)/2;
//izračun vrijednosti kuta za nivo na AI
stupanjA=float(aMax-aMin)/180;
//podešavanje izračuna smjera prema razlici u očitanju senzora
if (a>a_nul && b>b_nul) b_k=(180-(aMax-a)/stupanjA);
if (a<a_nul && b>b_nul) b_k=(180-(aMax-a)/stupanjA);
if (a>a_nul && b<b_nul) b_k=(180+(aMax-a)/stupanjA);
if (a<a_nul && b<b_nul) b_k=(180+(aMax-a)/stupanjA);

smjer=int(b_k);
if  ( 337 > smjer && smjer < 22   )char  smjerTxt[] = "N";
if  ( 21  > smjer && smjer < 78   )char  smjerTxt[] = "NE";
if  ( 77  > smjer && smjer < 112  )char  smjerTxt[] = "E";
if  ( 113 > smjer && smjer < 158  )char  smjerTxt[] = "SE";
if  ( 157 > smjer && smjer < 203  )char  smjerTxt[] = "S";
if  ( 202 > smjer && smjer < 247  )char  smjerTxt[] = "SW";
if  ( 246 > smjer && smjer < 293  )char  smjerTxt[] = "W";
if  ( 292 > smjer && smjer < 338  )char  smjerTxt[] = "NW";

  // - - - kraj smjer - - -
  
  // - - - baro - - - 

 tlakIz = int(tlak(analogRead(baroSenzor)));

  // - - - kraj baro - - - 
 
 
 
 
 // Svake sekunde:
  if (millis() > vrijeme+1000){
  // - - - temperatura - - -  
  temperatura = temperaturaDS();
   // - - - kraj temperatura - - -  
   brzina[brzinaIndex]=anmeoMetarVrijednost_OBRT;
   
    lcd.clear();
    vrijeme = millis();  
  }
  
 // svakih 30 sec :
  if (millis() > vrijeme1+30000){
    vrijeme1 = millis();
    int i, brZb;
    for (i=0; i<= sizeof(brzina); i++){
      brzinaUdar=max(brzinaUdar,(brzina[i] + brzina[i-1] + brzina[i-2])/3);      
      brZb=brZb+brzina[i];
      }
    brzinaSred=brZb/600;
      // slanje podataka u easyweather.dat  
      // izvj,0,0,2013-00-00,12:00:00 AM,0,0,0,0,temp,0,0,0,tlak,brzinaSred,0,brzinaUdar,0,0,smjerTxt,0,0,0,0,0,0,0,0
      izvj++;
      Serial.println();
      Serial.print(izvj,DEC);
      Serial.print(",0,0,2013-00-00,12:00:00 AM,0,0,0,0,");
      Serial.print(temperatura);
      Serial.print(",0,0,0,");
      Serial.print(tlakIz, 1);
      Serial.print(",");
      Serial.print(brzinaSred,1);
      Serial.print(",");
      Serial.print(brzinaUdar);
      Serial.print(",0,0,");
      Serial.print(smjerTxt);
      Serial.print(",0,0,0,0,0,0,0,0");
      
      // slanje na serijski port Serial1
      Serial1.println();
      Serial1.print(izvj,DEC);
      Serial1.print(",0,0,2013-00-00,12:00:00 AM,0,0,0,0,");
      Serial1.print(temperatura);
      Serial1.print(",0,0,0,");
      Serial1.print(tlakIz, 1);
      Serial1.print(",");
      Serial1.print(brzinaSred,1);
      Serial1.print(",");
      Serial1.print(brzinaUdar);
      Serial1.print(",0,0,");
      Serial1.print(smjerTxt);
      Serial1.print(",0,0,0,0,0,0,0,0");
      
    }
  



  
  
// - - - KONTROLNI IZLAZI - - - 
   
  Serial.println("\n--------------");
  
  //kontrolni izlaz anemo
  Serial.print("w: "); 
  Serial.print(anmeoMetarVrijednost_OBRT, DEC); 
  Serial.print("; "); 
  
  //kontrolni izlaz smjera
  Serial.print("\tSmjer=");
  Serial.print(smjer);
  
  //kontrolni baro
  Serial.print("\t  P = ");
  Serial.print(tlakIz);
  Serial.print(" hPa");
  Serial.print("\t");
  Serial.print(analogRead(baroSenzor));
  
  //kontrolni temp 
  Serial.print("\t T= ");
  Serial.print(temperatura);
  Serial.print(" C");
  
  
  


  
  // smjer
  lcd.setCursor(0,1);
  lcd.print("s=");
  lcd.print(smjer);
  lcd.print("      ");
  // brzina
  lcd.setCursor(0,0);
  lcd.print("w=");
  lcd.setCursor(3,0);
  lcd.print(anmeoMetarVrijednost_OBRT, DEC);
  // baro
  lcd.setCursor(7,0);
  lcd.print("p=");
  lcd.print(tlakIz);
  lcd.print("hPa");
  //temp
  lcd.setCursor(7, 1);
  lcd.print("t=");
  lcd.print(temperatura);
  lcd.print("C");

 
  
  
}//--------------- kraj loop-a ----------------- 

//----------------FUNKCIJE--------------------
//   anemoMetar 
void ISR_anemoMetar()
{
  // check the value again - since it takes some time to
  // activate the interrupt routine, we get a clear signal.
  anmeoMetarVrijednost++;
}
// kraj anemoMetar 

// baro

float tlak(float senzorTlak){
  float tlakIzracun;
  //senzorTlak=(analogRead(baroSenzor));
  //senzorTlak=943;
  tlakIzracun = (((senzorTlak/1024)+0.1518)/0.01059)*10;
  return tlakIzracun;
  
  /* Motorola MPX Data Sheet :Transfer Function
    Vout = VS (P x 0.01059 – 0.1518) +/– (Pressure Error x Temp. Factor x 0.01059 x VS)
    VS = 5.1 V ± 0.25 Vdc
  */
}

// kraj baro


//  tremometar DS1820  
float temperaturaDS(){ 
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float tempDS;

  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC nije dobar!");
  }
  // ROM byte indicira tip senzora
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
      } 
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  delay(1000);     
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);
// citanje podataka  
  for ( i = 0; i < 9; i++) {          
    data[i] = ds.read();
  }

  // konverzija binarnog podatka u temperaturu
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {
            raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  
    else if (cfg == 0x20) raw = raw & ~3;
    else if (cfg == 0x40) raw = raw & ~1;
    }
  tempDS = (float)raw / 16.0;
  return tempDS;
}//  kraj tremometar DS1820
  


