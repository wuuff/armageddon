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

const byte deadcity[] PROGMEM = {8,8,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B00100000,
B01100010,
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

const byte deadlauncher[] PROGMEM = {8,8,
B00000000,
B00000000,
B00000000,
B00000000,
B00000000,
B01100110,
B01011010,
B10000001,
};


#define LAUNCHER_ONE 1
#define LAUNCHER_TWO 2
#define MAX_PMISSILES 10
#define PSPEED 4
#define PRADIUS 6
#define EXPAND 0
#define SHRINK 1

#define MAX_EMISSILES 10
#define MAX_CHANCE 50

// State variables
uint8_t oddLoop = 0;

uint8_t cities[8] = {1,1,1,1,1,1,1,1}; //Whether the cities or launchers are alive

uint8_t targetX;
uint8_t targetY;
uint8_t pDests[MAX_PMISSILES][2] = {{100,100},{100,100},{100,100},{100,100},{100,100},{100,100},{100,100},{100,100},{100,100},{100,100}};
float pMissiles[MAX_PMISSILES][3] = {{100,100,0},{100,100,0},{100,100,0},{100,100,0},{100,100,0},{100,100,0},{100,100,0},{100,100,0},{100,100,0},{100,100,0}};
uint8_t pDetonations[MAX_PMISSILES][4] = {{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0},{100,100,0,0}};

uint8_t etotal = 5;
uint8_t echance = 1;
float espeed = 0.2;
uint8_t eDests[MAX_EMISSILES] = {100,100,100,100,100,100,100,100,100,100};
float eMissiles[MAX_EMISSILES][4] = {{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100},{100,100,100,100}};

void setup() {
  gb.begin();
  gb.titleScreen(F("Armageddon"));
  gb.battery.show = false;
  gb.pickRandomSeed();
}

void drawTargets(){
  gb.display.drawFastHLine(targetX-1,targetY,3);
  gb.display.drawFastVLine(targetX,targetY-1,3);

  for(uint8_t i = 0; i < MAX_PMISSILES; i++){
    //Check for a valid destination without a current detonation
    if( pDests[i][0] <= 84 && pDetonations[i][0] > 84 ){
      gb.display.drawPixel(pDests[i][0], pDests[i][1]);
    }
  }
}

void drawCities(){
  uint8_t alldead = 1;
  for(uint8_t i = 0; i < 8; i++){
      if( i == 2 || i == 5 ){
        if( cities[i] ){
          gb.display.drawBitmap(i*10+2,40,launcher);
        }else{
          gb.display.drawBitmap(i*10+2,40,deadlauncher);
        }
      }else{
        if( cities[i] ){
          alldead = 0;
          gb.display.drawBitmap(i*10+2,40,city);
        }else{
          gb.display.drawBitmap(i*10+2,40,deadcity);
        }
      }
    }

    if( alldead ){
      gb.display.cursorX = 84/2 - 5*3;
      gb.display.cursorY = 48/2 - 5;
      gb.display.print("THE END");
    }
}

void drawMissiles(){
  //Player Missiles
  for(uint8_t i = 0; i < MAX_PMISSILES; i++){
    //Check for a valid destination without a current detonation
    if( pDests[i][0] <= 84 && pDetonations[i][0] > 84 ){
      if(pMissiles[i][2] == LAUNCHER_ONE){
        gb.display.drawLine(25,40,pMissiles[i][0], pMissiles[i][1]);
      }else{
        gb.display.drawLine(56,40,pMissiles[i][0], pMissiles[i][1]);
      }
    }
  }

  //Enemy Missiles
  for(uint8_t i = 0; i < MAX_EMISSILES; i++){
    //Check for a valid destination
    if( eDests[i] <= 84 ){
        gb.display.drawLine(eMissiles[i][0],eMissiles[i][1],eMissiles[i][2], eMissiles[i][3]);
    }
  }
}

void drawDetonations(){
  for(uint8_t i = 0; i < MAX_PMISSILES; i++){
    if( pDetonations[i][0] <= 84 ){
      gb.display.drawCircle(pDetonations[i][0],pDetonations[i][1],pDetonations[i][2]);
    }
  }
}

void launchMissile(uint8_t launcher){
  //If there is no free slot in pDests, do not launch
  for(uint8_t i = 0; i < MAX_PMISSILES; i++){
    if( pDests[i][0] > 84 ){
      pDests[i][0] = targetX;
      pDests[i][1] = targetY;
      if(launcher == LAUNCHER_ONE){
        pMissiles[i][0] = 25; //X-coord of left launcher
        pMissiles[i][2] = LAUNCHER_ONE;
      }else{
        pMissiles[i][0] = 56; //X-coord of right launcher
        pMissiles[i][2] = LAUNCHER_TWO;
      }
      pMissiles[i][1] = 40; //Y-coord of both launchers
      break;
    }
  }
}

void tryLaunchEnemy(){
  uint8_t someActive = 0;
  // Check whether there are any active missiles
  // If none are active, always spawn one to avoid
  // long pauses without any enemy missiles
  for(uint8_t i = 0; i < MAX_EMISSILES; i++){
    if( eDests[i] <= 84 ){
      someActive = 1;
      break;
    }
  }
  
  if( etotal > 0 && (!someActive || echance >= random(100)) ){ //echance of 100
    for(uint8_t i = 0; i < MAX_EMISSILES; i++){
      if( eDests[i] > 84 ){
        eDests[i] = random(8); //Target one of the 6 cities or 2 launch sites
        eMissiles[i][0] = random(84); //Screen width
        eMissiles[i][1] = 0; //Top of screen
        eMissiles[i][2] = eMissiles[i][0]; //Start and end are same
        eMissiles[i][3] = 0; //Top of screen
        break; //Only spawn one
      }
    }
  }
}

void stepMissiles(){
  //Player Missiles
  for(uint8_t i = 0; i < MAX_PMISSILES; i++){
    //Check for a valid destination without a current detonation
    if( pDests[i][0] <= 84 && pDetonations[i][0] > 84 ){
      //If the missile is close enough to the destination, detonate
      if( abs( pDests[i][0] - pMissiles[i][0] ) < PSPEED && abs( pDests[i][1] - pMissiles[i][1] ) < PSPEED ){
        pDetonations[i][0] = pDests[i][0];
        pDetonations[i][1] = pDests[i][1];
        pDetonations[i][2] = 0; //Start detonation at radius of 0
        pDetonations[i][3] = EXPAND; //Detonation is increasing in size
      //Otherwise, keep moving towards destination
      }else{
        float dir = atan2(pDests[i][1]-pMissiles[i][1], pDests[i][0]-pMissiles[i][0]);
        pMissiles[i][0] += PSPEED * cos(dir);
        pMissiles[i][1] += PSPEED * sin(dir);
      }
    }
  }

  //Enemy Missiles
  for(uint8_t i = 0; i < MAX_EMISSILES; i++){
    //Check for a valid destination
    if( eDests[i] <= 84 ){
       //If enemy missile is close enough to the destination, detonate
      if( abs( (eDests[i]*10+6) - eMissiles[i][2] ) < PSPEED && abs( 44 - eMissiles[i][3] ) < PSPEED ){
        cities[eDests[i]] = 0; //Destroy city/launcher
        eDests[i] = 100; //Reset enemy missile
      //Otherwise, keep moving towards destination
      }else{
        float dir = atan2( 44-eMissiles[i][3], (eDests[i]*10+6)-eMissiles[i][2] );
        eMissiles[i][2] += espeed * cos(dir);
        eMissiles[i][3] += espeed * sin(dir);
      }
    }
  }
}

void stepDetonations(){
  if( oddLoop ){
    for(uint8_t i = 0; i < MAX_PMISSILES; i++){
      if( pDetonations[i][0] <= 84 ){
        if( pDetonations[i][3] == EXPAND ){ //If detonation is expanding
          if( pDetonations[i][2] < PRADIUS ){
            pDetonations[i][2]++;
          }else{
            pDetonations[i][3] = SHRINK; //Start shrinking
          }
        }
        //Check this now instead of using else because it may have just
        //set to SHRINK; immediately checking will remove a delay at full
        //size.  If a delay is desired, this if can be switched to an else.
        if( pDetonations[i][3] == SHRINK ){ //If detonation is shrinking
          if( pDetonations[i][2] > 0 ){
            pDetonations[i][2]--;
          }else{
            pDetonations[i][0] = 100; //Detonation is complete, remove it
            pDests[i][0] = 100; //Remove this destination
          }
        }
      }
    }
  }
}

void stepCollision(){
  //Player Detonations with Enemy Missiles
  for(uint8_t i = 0; i < MAX_PMISSILES; i++){
      if( pDetonations[i][0] <= 84 ){
        for(uint8_t j = 0; j < MAX_EMISSILES; j++){
          if( pDetonations[i][2] >= sqrt( (eMissiles[j][2]-pDetonations[i][0])*(eMissiles[j][2]-pDetonations[i][0]) + (eMissiles[j][3]-pDetonations[i][1])*(eMissiles[j][3]-pDetonations[i][1]) ) ){
            eDests[j] = 100; //Remove enemy missile
          }
        }
      }
  }
}

void loop() {
  if(gb.update()){
    oddLoop++;
    oddLoop%=2;
    
    //Player input
    if( gb.buttons.pressed(BTN_A) ){
      launchMissile(LAUNCHER_ONE);
    }
    if( gb.buttons.pressed(BTN_B) ){
      launchMissile(LAUNCHER_TWO);
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
    tryLaunchEnemy();
    stepMissiles();
    stepDetonations();
    stepCollision();

    //Drawing
    drawTargets();
    drawCities();
    drawMissiles();
    drawDetonations();
    
  }
}
