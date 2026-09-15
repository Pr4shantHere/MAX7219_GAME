// Requirements:-
// Arduino uno, Jumper Wires, 2 buttons, 2 resistors and a MAX 7219 module (it is an 8 by 8 matrix module but requires less setup than a simple matrix display)

// connect CS to 7
// VCC to 5v
// GND to GND
// DIN to 11
// CLK to 13
// One button to 3
// One button to 2


#include <SPI.h>
#define CS 7


#define DECODE_MODE 9
#define INTENSITY 10
#define SCAN_LIMIT 11
#define SHUTDOWN 12
#define DISPLAY_TEST 16

#define UP_BTN 3
#define DOWN_BTN 2

void SendData(uint8_t address, uint8_t value){
  digitalWrite(CS, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CS, HIGH);
}


// defining all possible combination of pillars
byte pillars[6][8] = {
             {B00000011,B00000011,B00000011,B00000011,B00000011,B00000000,B00000000,B00000000},
             {B00000011,B00000011,B00000011,B00000011,B00000000,B00000000,B00000000,B00000011},
             {B00000011,B00000011,B00000011,B00000000,B00000000,B00000000,B00000011,B00000011}, 
             {B00000011,B00000011,B00000000,B00000000,B00000000,B00000011,B00000011,B00000011}, 
             {B00000011,B00000000,B00000000,B00000000,B00000011,B00000011,B00000011,B00000011},
             {B00000000,B00000000,B00000000,B00000011,B00000011,B00000011,B00000011,B00000011}
  };

byte display_pillar[8];
byte skull_byte[8] = {
  B00111110,
  B01111111,
  B01101101,
  B01101101,
  B01111011,
  B00111110,
  B00101010,
  B00000000
};

int pillar = 0;
int x = 9; // this will be used to confirm if one pillar has passed the screen
byte player_byte = B01000000;
byte col_byte = B01100000; // if the byte arrangement of the player row is like this, then it means collision has occured, and game over
int player_row = 4; // 1 -> 8
int game_over = 0;

// actions for the buttons
void decreaseRow(){
  if(player_row != 1){
    player_row -= 1;
  }
}

void increaseRow(){
  if(player_row != 8){ 
    player_row += 1;
  }
}

void setup() {
  pinMode(CS, OUTPUT);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();
  Serial.begin(115200);

  //SendData(DISPLAY_TEST, 0x01);
  //delay(1000);
  SendData(DISPLAY_TEST, 0x00);
  SendData(DECODE_MODE, 0x00);
  SendData(INTENSITY, 0x01);
  SendData(SCAN_LIMIT, 0x0f);
  SendData(SHUTDOWN, 0x01);

  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  if(game_over == 0){
    pillar = random(6);

    for(int i = 0; i < 8; i++){
      display_pillar[i] = pillars[pillar][i];
    }
  }
}

void ScrollFrame(byte frame[]){
  for(int i = 0; i<8; i++){
    frame[i] = frame[i] << 1;
  }

  x += 1;
  if(x > 9){
    x = 1;
  }
}


void DisplayFrame(byte frame[]){
  for(int i = 1; i<9; i++){
    if(i != player_row){
      SendData(i, frame[i-1]);
    }
    else{
      SendData(i, frame[i-1] | player_byte);
    }
  }
}

void DisplayGO(){
  for(int i = 1; i < 9; i++){
    SendData(i, skull_byte[i - 1]);
  }
}

unsigned long delayTime = 500;
unsigned long previousTime = millis();
unsigned long previousTimeUpButton = millis();
unsigned long previousTimeDownButton = millis();
unsigned long debounceDelayButton = 50;
int lastStageUpButton = LOW;
int lastStageDownButton = LOW;
unsigned long timeNow;

void loop(){
  timeNow = millis();
  int UpReading = digitalRead(UP_BTN);
  int DownReading = digitalRead(DOWN_BTN);

  if(game_over == 0){
    DisplayFrame(display_pillar);

    if(timeNow - previousTime > delayTime){
      ScrollFrame(display_pillar);
      if(x == 9){
        pillar = random(6);
        for(int i = 0; i < 8; i++){
          display_pillar[i] = pillars[pillar][i];
        }
      }
      previousTime += delayTime;
    }
  }
  else{
    DisplayGO();
    x = 9;
    game_over = 0;
  }

  // player movement
  if(UpReading != lastStageUpButton){
    if(timeNow - previousTimeUpButton > debounceDelayButton){
      if(UpReading == HIGH){
        if(game_over == 1){
          game_over = 0;
        }
        decreaseRow();
        previousTimeUpButton += debounceDelayButton;
        lastStageUpButton = UpReading;
      }
      if(UpReading == LOW){
        previousTimeUpButton += debounceDelayButton;
        lastStageUpButton = UpReading;
      }
    }
  }

  if(DownReading != lastStageDownButton){
    if(timeNow - previousTimeDownButton > debounceDelayButton){
      if(DownReading == HIGH){
        if(game_over == 1){
          game_over = 0;
        }
        increaseRow();
        previousTimeDownButton += debounceDelayButton;
        lastStageDownButton = DownReading;
      }
      if(DownReading == LOW){
        previousTimeDownButton += debounceDelayButton;
        lastStageDownButton = DownReading;
      }
    }
  }

  // collission
  for(int i = 0; i < 8; i++){
    if(i == player_row - 1){
      if(display_pillar[i] == col_byte){
        if(game_over == 0){
          game_over = 1;
        }
      }
    }
  }
}
