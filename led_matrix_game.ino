#include <binary.h>
#include "LedControl.h"

/* 
 * Pin 12 is connected to the DATA IN-pin
 * Pin 11 is connected to the CLK-pin
 * Pin 10 is connected to the LOAD(/CS)-pin  
 * There are 4 8x8 matrices attached to the arduino 
 */
LedControl lc=LedControl(12,11,10,4); 

int obstacle_low_position;
int obstacle_low_table;
bool obstacle_low_active;

int obstacle_low2_position;
int obstacle_low2_table;
bool obstacle_low2_active;

int obstacle_mid_position;
int obstacle_mid_table;
bool obstacle_mid_active;

int obstacle_mid2_position;
int obstacle_mid2_table;
bool obstacle_mid2_active;

int obstacle_high_position;
int obstacle_high_table;
bool obstacle_high_active;

int obstacle_high2_position;
int obstacle_high2_table;
bool obstacle_high2_active;

volatile int buttonReading = 0;
volatile int buttonReading2 = 0;

int playerAction = 0; // 0: Do nothing, 1: Transition, 2: Jump, 3: Duck
int prevPlayerAction = 0;
int current_player_location = 0;
int transitionTime;;
 
volatile int playerActionWait = 0;
const int buttonPin = 2;    // the number of the pushbutton pin
const int buttonPin2 = 3;    // the number of the pushbutton pin


int buttonState;             // the current reading from the input pin
int buttonState2;             // the current reading from the input pin

int obstacleSpeed;
int led_step;
int lives_remaining;
volatile int player_air_time = 0;

byte g[7] = {B00111100, B01000010, B01000000, B01000000, B01011110, B01000010, B00111100}; //G
byte b[7] = {B01111100, B01000010, B01000010, B01111100, B01000010, B01000010, B01111100}; //B

void buttonISR(){
  buttonReading = digitalRead(buttonPin);
  // Ff the button state has changed:
  if (buttonReading != buttonState) {
    buttonState = buttonReading;
    // Jump
      if (buttonState == HIGH) {;
        playerAction = 2;
        playerActionWait = millis();
        player_air_time = 0;
      }
    }
}

void buttonISR2(){
  buttonReading2 = digitalRead(buttonPin2);
      // if the button state has changed:
    if (buttonReading2 != buttonState2) {
      buttonState2 = buttonReading2;
      // Duck
      if (buttonState2 == HIGH) {
        playerAction = 3;
        playerActionWait = millis();
        player_air_time = 0;
      }
    }
}

void setup()
{
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, CHANGE); //interrupt
  attachInterrupt(digitalPinToInterrupt(buttonPin2), buttonISR2, CHANGE); //interrupt

  // Wake up the Matrix from power-saving mode
  lc.shutdown(0,false); 
  lc.shutdown(1,false); 
  lc.shutdown(2,false); 
  lc.shutdown(3,false); 
    
  // Set a medium brightness for the LEDs
  lc.setIntensity(0,8);
  lc.setIntensity(1,8);
  lc.setIntensity(2,8);
  lc.setIntensity(3,8);
    
  delay(50); 
  lc.clearDisplay(0);

  initGame();
}

void initGame(){
  obstacle_low_position = 7;
  obstacle_low_table = 0;
  obstacle_low_active = false;
  
  obstacle_low2_position = 7;
  obstacle_low2_table = 0;
  obstacle_low2_active = false;
  
  obstacle_mid_position = 7;
  obstacle_mid_table = 0;
  obstacle_mid_active = false;

  obstacle_mid2_position = 7;
  obstacle_mid2_table = 0;
  obstacle_mid2_active = false;

  obstacle_high_position = 7;
  obstacle_high_table = 0;
  obstacle_high_active = false;

  obstacle_high2_position = 7;
  obstacle_high2_table = 0;
  obstacle_high2_active = false;

  obstacleSpeed = 200;
  transitionTime = 1000;
  led_step = 0;

  playerActionWait = 0;
  playerAction = 0;
  lives_remaining = 3;
}

void softInitGame(){
  obstacle_low_position = 7;
  obstacle_low_table = 0;
  obstacle_low_active = false;
  
  obstacle_low2_position = 7;
  obstacle_low2_table = 0;
  obstacle_low2_active = false;
  
  obstacle_mid_position = 7;
  obstacle_mid_table = 0;
  obstacle_mid_active = false;

  obstacle_mid2_position = 7;
  obstacle_mid2_table = 0;
  obstacle_mid2_active = false;

  obstacle_high_position = 7;
  obstacle_high_table = 0;
  obstacle_high_active = false;

  obstacle_high2_position = 7;
  obstacle_high2_table = 0;
  obstacle_high2_active = false;

  playerActionWait = 0;
  playerAction = 0;
}

void player(){
  if (playerAction == 0) { // Default
    if(prevPlayerAction != playerAction) {
      if(prevPlayerAction == 2){
        lc.setLed(2,1, 3,false); //head
        lc.setLed(2,2, 3,false); //body-top
        lc.setLed(2,2, 2,false); //armyleft
        lc.setLed(2,2, 4,false); //arm-right
        lc.setLed(2,3, 3,false); //body-bottom
        lc.setLed(2,4, 2,false); //leg-left
        lc.setLed(2,4, 4,false); //leg-right 
      }else if(prevPlayerAction == 3){
        lc.setLed(2,5, 3,false); //head
        lc.setLed(2,6, 3,false); //body-top
        lc.setLed(2,6, 2,false); //armyleft
        lc.setLed(2,6, 4,false); //arm-right        
      }
      prevPlayerAction = playerAction;
    }
    lc.setLed(2,3, 3,true); //head
    lc.setLed(2,4, 3,true); //body-top
    lc.setLed(2,4, 2,true); //armyleft
    lc.setLed(2,4, 4,true); //arm-right
    lc.setLed(2,5, 3,true); //body-bottom
    lc.setLed(2,6, 2,true); //leg-left
    lc.setLed(2,6, 4,true); //leg-right
    current_player_location = 0;
  } else if(playerAction == 1){ //Wait
   if(player_air_time == 5){
      playerAction = 0;
    }
  } else if(playerAction == 2) { // Jump
     if(prevPlayerAction != playerAction){
      if(prevPlayerAction == 0){
        lc.setLed(2,3, 3,false); //head
        lc.setLed(2,4, 3,false); //body-top
        lc.setLed(2,4, 2,false); //armyleft
        lc.setLed(2,4, 4,false); //arm-right
        lc.setLed(2,5, 3,false); //body-bottom
        lc.setLed(2,6, 2,false); //leg-left
        lc.setLed(2,6, 4,false); //leg-right
      }else if(prevPlayerAction == 3){
        lc.setLed(2,5, 3,false); //head
        lc.setLed(2,6, 3,false); //body-top
        lc.setLed(2,6, 2,false); //armyleft
        lc.setLed(2,6, 4,false); //arm-right        
      }
      prevPlayerAction = playerAction;
    }
    lc.setLed(2,1, 3,true); //head
    lc.setLed(2,2, 3,true); //body-top
    lc.setLed(2,2, 2,true); //armyleft
    lc.setLed(2,2, 4,true); //arm-right
    lc.setLed(2,3, 3,true); //body-bottom
    lc.setLed(2,4, 2,true); //leg-left
    lc.setLed(2,4, 4,true); //leg-right
    playerAction = 1; // Go to transition
    current_player_location = 2;
  }else if(playerAction == 3){ // Duck
    if(prevPlayerAction != playerAction){
      if(prevPlayerAction == 0){
        lc.setLed(2,3, 3,false); //head
        lc.setLed(2,4, 3,false); //body-top
        lc.setLed(2,4, 2,false); //armyleft
        lc.setLed(2,4, 4,false); //arm-right
        lc.setLed(2,5, 3,false); //body-bottom
        lc.setLed(2,6, 2,false); //leg-left
        lc.setLed(2,6, 4,false); //leg-right
      }else if(prevPlayerAction == 2){
        lc.setLed(2,1, 3,false); //head
        lc.setLed(2,2, 3,false); //body-top
        lc.setLed(2,2, 2,false); //armyleft
        lc.setLed(2,2, 4,false); //arm-right
        lc.setLed(2,3, 3,false); //body-bottom
        lc.setLed(2,4, 2,false); //leg-left
        lc.setLed(2,4, 4,false); //leg-right        
      }
      prevPlayerAction = playerAction;
    }
    lc.setLed(2,5, 3,true); //head
    lc.setLed(2,6, 3,true); //body-top
    lc.setLed(2,6, 2,true); //armyleft
    lc.setLed(2,6, 4,true); //arm-right
    playerAction = 1; // Go to transition
    current_player_location = 3;
  }
    player_air_time++;

}

void showLives(){
  byte g[7] = {B00111100};

  if(lives_remaining == 3){
    lc.setLed(3,0,0, true);
    lc.setLed(3,0,1, true);
    lc.setLed(3,0,2, true);
  }else if(lives_remaining == 2){
    lc.setLed(3,0,0, true);
    lc.setLed(3,0,1, true);
    lc.setLed(3,0,2, false);
  }else if(lives_remaining == 1){
    lc.setLed(3,0,0, true);
    lc.setLed(3,0,1, false);
    lc.setLed(3,0,2, false);
  }
}

void obstacle_low(){
    lc.setLed(obstacle_low_table,6, obstacle_low_position + 1,false);
    if(obstacle_low_position == -1 and obstacle_low_table == 3){
      obstacle_low_table = 0;
      obstacle_low_position = 7;
      obstacle_low_active = false;
    }else if(obstacle_low_position < 0){
      obstacle_low_table++;
      obstacle_low_position = 7;
      lc.setLed(obstacle_low_table,6, obstacle_low_position,true);
      obstacle_low_position--;
    }else{
       lc.setLed(obstacle_low_table,6, obstacle_low_position,true);
       obstacle_low_position--;
    }
}

void obstacle_low2(){
    lc.setLed(obstacle_low2_table,6, obstacle_low2_position + 1,false);
    if(obstacle_low2_position == -1 and obstacle_low2_table == 3){
      obstacle_low2_table = 0;
      obstacle_low2_position = 7;
      obstacle_low2_active = false;
    }else if(obstacle_low2_position < 0){
      obstacle_low2_table++;
      obstacle_low2_position = 7;
      lc.setLed(obstacle_low2_table,6, obstacle_low2_position,true);
      obstacle_low2_position--;
    }else{
     lc.setLed(obstacle_low2_table,6, obstacle_low2_position,true);      
     obstacle_low2_position--;
    }
}
void obstacle_mid(){
    lc.setLed(obstacle_mid_table,4, obstacle_mid_position + 1,false);
    if(obstacle_mid_position == -1 and obstacle_mid_table == 3){
      obstacle_mid_table = 0;
      obstacle_mid_position = 7;
      obstacle_mid_active = false;
    }else if(obstacle_mid_position < 0){
      obstacle_mid_table++;
      obstacle_mid_position = 7;
      lc.setLed(obstacle_mid_table,4, obstacle_mid_position,true);
      obstacle_mid_position--;
    }else{
      lc.setLed(obstacle_mid_table,4, obstacle_mid_position,true);
      obstacle_mid_position--;
    }
}
void obstacle_mid2(){
    lc.setLed(obstacle_mid2_table, 4, obstacle_mid2_position + 1,false);
    if(obstacle_mid2_position == -1 and obstacle_mid2_table == 3){
      obstacle_mid2_table = 0;
      obstacle_mid2_position = 7;
      obstacle_mid2_active = false;
    }else if(obstacle_mid2_position < 0){
      obstacle_mid2_table++;
      obstacle_mid2_position = 7;
      lc.setLed(obstacle_mid2_table, 4, obstacle_mid2_position,true);
      obstacle_mid2_position--;
    }else{
      lc.setLed(obstacle_mid2_table, 4, obstacle_mid2_position,true);
      obstacle_mid2_position--;
    }
}
void obstacle_high(){
    lc.setLed(obstacle_high_table,2, obstacle_high_position + 1,false);
    if(obstacle_high_position == -1 and obstacle_high_table == 3){
      obstacle_high_table = 0;
      obstacle_high_position = 7;
      obstacle_high_active = false;
    }else if(obstacle_high_position < 0){
      obstacle_high_table++;
      obstacle_high_position = 7;
      lc.setLed(obstacle_high_table,2, obstacle_high_position,true);
      obstacle_high_position--;
    }else{
      lc.setLed(obstacle_high_table,2, obstacle_high_position,true);
      obstacle_high_position--;
    }
}

void obstacle_high2(){
    lc.setLed(obstacle_high2_table,2, obstacle_high2_position + 1,false);
    if(obstacle_high2_position == -1 and obstacle_high2_table == 3){
      obstacle_high2_table = 0;
      obstacle_high2_position = 7;
      obstacle_high2_active = false;
    }else if(obstacle_high2_position < 0){
      obstacle_high2_table++;
      obstacle_high2_position = 7;
      lc.setLed(obstacle_high2_table,2, obstacle_high2_position,true);
      obstacle_high2_position--;
    }else{
      lc.setLed(obstacle_high2_table,2, obstacle_high2_position,true);
     obstacle_high2_position--;
    }
}

void checkCollision(){
  if (obstacle_low_table == 2 and (current_player_location == 0 or current_player_location == 3) and (obstacle_low_position == 1 or obstacle_low_position == 2 or obstacle_low_position == 3)) {
    gameOver();
  } else if (obstacle_low2_table == 2 and (current_player_location == 0 or current_player_location == 3) and (obstacle_low2_position == 1 or obstacle_low2_position == 2 or obstacle_low2_position == 3)) {
    gameOver();
  } else if (obstacle_mid_table == 2 and (current_player_location == 0 or current_player_location == 2)){
    if (current_player_location == 0 and (obstacle_mid_position == 1 or obstacle_mid_position == 2 or obstacle_mid_position == 3)) {
      gameOver();
    } else if (current_player_location == 2 and (obstacle_mid_position == 1 or obstacle_mid_position == 2 or obstacle_mid_position == 3)) {
      gameOver();
    }
  } else if (obstacle_mid2_table == 2 and (current_player_location == 0 or current_player_location == 2)){
    if (current_player_location == 0 and (obstacle_mid2_position == 1 or obstacle_mid2_position == 2 or obstacle_mid2_position == 3)){
      gameOver();
    } else if (current_player_location == 2 and (obstacle_mid2_position == 1 or obstacle_mid2_position == 2 or obstacle_mid2_position == 3)){
      gameOver();
    }
  } else if (obstacle_high_table == 2 and current_player_location == 2 and (obstacle_high_position == 1 or obstacle_high_position == 2 or obstacle_high_position == 3)) {
    gameOver();
  } else if (obstacle_high2_table == 2 and current_player_location == 2 and (obstacle_high2_position == 1 or obstacle_high2_position == 2 or obstacle_high2_position == 3)) {
    gameOver();
  }
}

void winGame() {

  lc.setRow(2,0,g[0]);
  lc.setRow(2,1,g[1]);
  lc.setRow(2,2,g[2]);
  lc.setRow(2,3,g[3]);
  lc.setRow(2,4,g[4]);
  lc.setRow(2,5,g[5]);
  lc.setRow(2,6,g[6]);
  
  lc.setRow(1,0,g[0]);
  lc.setRow(1,1,g[1]);
  lc.setRow(1,2,g[2]);
  lc.setRow(1,3,g[3]);
  lc.setRow(1,4,g[4]);
  lc.setRow(1,5,g[5]);
  lc.setRow(1,6,g[6]);
  delay(3000);
  
  initGame();
  clearDisplay();
}

void gameOver(){

  if(lives_remaining > 1){
    clearDisplay();
    lives_remaining--;
    softInitGame();
  }else{
    lc.setRow(1,0,g[0]);
    lc.setRow(1,1,g[1]);
    lc.setRow(1,2,g[2]);
    lc.setRow(1,3,g[3]);
    lc.setRow(1,4,g[4]);
    lc.setRow(1,5,g[5]);
    lc.setRow(1,6,g[6]);
  
    lc.setRow(2,0,b[0]);
    lc.setRow(2,1,b[1]);
    lc.setRow(2,2,b[2]);
    lc.setRow(2,3,b[3]);
    lc.setRow(2,4,b[4]);
    lc.setRow(2,5,b[5]);
    lc.setRow(2,6,b[6]);
    
    delay(3000);
    
    initGame();
    clearDisplay();
  }
}

void setRow(int row){
  for(int column = 0; column < 8; column++){
      lc.setLed(0,row, column,true); 
      lc.setLed(1,row, column,true); 
      lc.setLed(2,row, column,true); 
      lc.setLed(3,row, column,true); 

  }
}

void clearDisplay(){
    lc.clearDisplay(0);
    lc.clearDisplay(1);
    lc.clearDisplay(2);
    lc.clearDisplay(3);

}

int random_number;

void loop() {

  setRow(7);

  if (led_step % 8 == 0) {
    bool new_obstacle = false;
    
    while (new_obstacle == false) {
        random_number = millis() % 6;
        //Serial.print(random_number);
        //Serial.print("\n");
      if (random_number == 0 && obstacle_low_active == false) {
         obstacle_low_active = true;
         new_obstacle = true;
      } else if (random_number == 1 && obstacle_low2_active == false) {
        obstacle_low2_active = true;
        new_obstacle = true;
      } else if (random_number == 2 && obstacle_mid_active == false) {
        obstacle_mid_active = true;
        new_obstacle = true;
      } else if (random_number == 3 && obstacle_mid2_active == false) {
        obstacle_mid2_active = true;
        new_obstacle = true;
      } else if (random_number == 4 && obstacle_high_active == false) {
        obstacle_high_active = true;
        new_obstacle = true;
      } else if (random_number == 5 && obstacle_high2_active == false ) {
        obstacle_high2_active = true;
        new_obstacle = true;
      }
    }    
  }

  if(obstacle_low_active == true){
    obstacle_low();
  }
  if(obstacle_low2_active == true){
    obstacle_low2();
  }
  if(obstacle_mid_active == true){
    obstacle_mid();
  }
  if(obstacle_mid2_active == true){
    obstacle_mid2();
  }
  if(obstacle_high_active == true){
    obstacle_high();
  }
  if(obstacle_high2_active == true){
    obstacle_high2();
  }
  player();
  

  if(led_step % 32 == 0){
    if(obstacleSpeed <= 2){
      winGame();
    }else if(obstacleSpeed <= 10){
      obstacleSpeed -= 2;
    }else if(obstacleSpeed <= 30){
      obstacleSpeed -= 5;
    }else if(obstacleSpeed <= 100){
      obstacleSpeed -= 10;
    }else{
       obstacleSpeed -= 50;
    }
  }
   
  led_step++;
  delay(obstacleSpeed);
  checkCollision();
  showLives();
}
