// nRF24L01 přijímač

// připojení knihoven
#include <SPI.h>
#include "RF24.h"
// nastavení propojovacích pinů
#define CE 7
#define CS 8
// inicializace nRF s piny CE a CS
RF24 nRF(CE, CS);
// nastavení adres pro přijímač a vysílač,
// musí být nastaveny stejně v obou programech!
byte adresaPrijimac[]= "prijimac00";
byte adresaVysilac[]= "vysilac00";

void setup() {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(9600);
  // zapnutí komunikace nRF modulu
  nRF.begin();
  // nastavení výkonu nRF modulu,
  // možnosti jsou RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX,
  // pro HIGH a MAX je nutný externí 3,3V zdroj
  nRF.setPALevel(RF24_PA_LOW);
  // nastavení zapisovacího a čtecího kanálu
  nRF.openWritingPipe(adresaPrijimac);
  nRF.openReadingPipe(1,adresaVysilac);
  // začátek příjmu dat
  nRF.startListening();
}

void loop() {
  // proměnné pro příjem a odezvu
  int prijem;
  unsigned long odezva;
  // v případě, že nRF je připojené a detekuje
  // příchozí data, začni s příjmem dat
  if( nRF.available()){
    // čekání na příjem dat
    while (nRF.available()) {
      // v případě příjmu dat se provede zápis
      // do proměnné prijem
      nRF.read( &prijem, sizeof(prijem) );
    }
    // vytisknutí přijatých dat na sériovou linku
    Serial.print("Prijata volba: ");
    Serial.print(prijem);
    // dekódování přijatých dat
    switch( prijem ) {
      // pro známou hodnotu dat (1,2,3)
      // se odešle odezva:
      case 1:
        // v případě 1 odešli počet milisekund
        // od připojení napájení
        odezva = millis();
        break;
      case 2:
        // v případě 2 počet sekund
        // od připojení napájení
        odezva = millis()/1000;
        break;
      case 3:
        // v případě 3 počet mikrosekund
        // od připojení napájení
        odezva = micros();
        break;
      // v případě ostatních dat bude odezva 0
      default:
        odezva = 0;
        break;
    }
    // ukončení příjmu dat
    nRF.stopListening();
    // odeslání odezvy 
    nRF.write( &odezva, sizeof(odezva) );     
    // přepnutí do příjmu dat pro další komunikaci
    nRF.startListening();
    // vytištění odezvy po sériové lince     
    Serial.print(", odezva: ");
    Serial.println(odezva);  
  }
  
}
