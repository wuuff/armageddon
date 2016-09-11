#include <SPI.h>
//Use for PROGMEM
#include <avr/pgmspace.h>
#include <Gamebuino.h>
Gamebuino gb;

void setup() {
  gb.begin();
  gb.titleScreen(F("Armageddon"));
  gb.battery.show = false;
  gb.pickRandomSeed();
}

void loop() {
  if(gb.update()){
    gb.display.print("Hello world");
  }
}
