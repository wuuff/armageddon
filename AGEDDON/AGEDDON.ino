#include <SPI.h>
//Use for PROGMEM
#include <avr/pgmspace.h>
#include <Gamebuino.h>
Gamebuino gb;

const byte city[] PROGMEM = {8,8,
B00000000,
B00000000,
B00000000,
B00000000,
B00100010,
B01110110,
B01111110,
B11111111,
};

const byte launcher[] PROGMEM = {8,8,
B00011000,
B00011000,
B00011000,
B00100100,
B00100100,
B01000010,
B01000010,
B10000001,
};



// State variables
uint8_t targetX;
uint8_t targetY;


void setup() {
  gb.begin();
  gb.titleScreen(F("Armageddon"));
  gb.battery.show = false;
  gb.pickRandomSeed();
}

void drawCursor(){
  gb.display.drawFastHLine(targetX-1,targetY,3);
  gb.display.drawFastVLine(targetX,targetY-1,3);
}

void drawCities(){
  for(uint8_t i = 0; i < 8; i++){
      if( i == 2 || i == 5 ){
        gb.display.drawBitmap(i*10+2,40,launcher);
      }else{
        gb.display.drawBitmap(i*10+2,40,city);
      }
    }
}

void loop() {
  if(gb.update()){
    //Player input
    if( gb.buttons.pressed(BTN_A) ){
      
    }
    if( gb.buttons.pressed(BTN_B) ){
      
    }
    
    if( gb.buttons.repeat(BTN_LEFT,1) ){
      targetX = targetX-2 > 0 ? targetX-2 : 0;
    }
    if( gb.buttons.repeat(BTN_RIGHT,1) ){
      targetX = targetX+2 < 84 ? targetX+2 : 84;
    }
    if( gb.buttons.repeat(BTN_UP,1) ){
      targetY = targetY-2 > 0 ? targetY-2 : 0;
    }
    if( gb.buttons.repeat(BTN_DOWN,1) ){
      targetY = targetY+2 < 48 ? targetY+2 : 48;
    }


    //Game logic


    //Drawing
    drawCursor();
    drawCities();
    
  }
}
