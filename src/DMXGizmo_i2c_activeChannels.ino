

/*****************************************************************
This is the firmware for the as-yet unnamed DMX controller designed
by Derek Bever. 
*****************************************************************/
                                                                

#include <EEPROM.h>
#include <DmxSimple.h>
#include <serLCD.h>
#include <String.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
// #define LCD_TX A3 //serial connection to LCD


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET A3

Adafruit_SSD1306 display(OLED_RESET);



#define OPERATION_enter 1  //these #define statements simply match operations to
#define OPERATION_at 2     //to numbers for use in our switch case
#define OPERATION_full 3
#define OPERATION_out 4
#define OPERATION_release 5
#define OPERATION_input 6
#define OPERATION_clear 7
#define OPERATION_up 10
#define OPERATION_left 11
#define OPERATION_mid 12
#define OPERATION_right 13
#define OPERATION_down 14

#define buttonUp A0
#define buttonLeft A1
#define buttonMid A2
#define buttonRight A3
#define buttonDown A4


#define ENTER 35 //This is the ASCII value of #, used as our enter key
#define AT 65 //ASCII value for A, which will be our 'at' command
#define FULL 66 //B, which will be our 'full' command
#define OUT 67 //C, which will act for 'out'
#define RELEASE 68 //D, which is a 'release all' command
#define CLEAR 42 //*, which we'll be using for 'clear'
#define LAST 76
#define NEXT 78
#define UP 85
#define DOWN 69


//byte channelLevels[512];
int buttonCase = 0;
int inputTemp = 0;
int levelTemp = 0;
int inputMode = 0;
int i=0;
int j=0;
unsigned long buttonTime = 0;
unsigned long timeCheck = 0;

//serLCD lcd(LCD_TX); //Instruct serLCD library which pin our LCD is connected to

//build the keypad
const byte ROWS = 5; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
  {'L','N','U','E'},
};
byte rowPins[ROWS] = {10,11,12,13,A0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5,8,7,9}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int activeChannel = 0;
int activeLevel = 0;
int buttonFlag = 1;
int escape = 0;

void setup(){
  for(i=1; i<513; i++){
    if(EEPROM.read(i) != 0){
      EEPROM.write(i, 0);
    }
  }
  clearAll();
  update();
  keypad.setDebounceTime(75);  
  
//    Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
 
  display.clearDisplay();
  display.display();
  update();
  
}


void loop(){   
//  while(buttonFlag == 0){
//    buttonFlag = digitalRead(buttonUp) && digitalRead(buttonDown);
//  }
  char inputChar = keypad.getKey();
  if (inputChar == ENTER) {
    buttonCase = OPERATION_enter;
  }
  else if (inputChar == AT) {
    buttonCase = OPERATION_at;
  }
  else if (inputChar == FULL) {
    buttonCase = OPERATION_full;
  }
  else if (inputChar == OUT) {
    buttonCase = OPERATION_out;
  }
  else if (inputChar == RELEASE) {
    buttonCase = OPERATION_release;
  }
  else if (inputChar == CLEAR) {
    buttonCase = OPERATION_clear;
  }
  else if (inputChar == LAST) {
    buttonCase = OPERATION_left;
  }
  else if (inputChar == NEXT) {
    buttonCase = OPERATION_right;
  }
  else if (inputChar == UP) {
    buttonCase = OPERATION_up;
  }
  else if (inputChar == DOWN) {
    buttonCase = OPERATION_down;
  }
    
  else if (inputChar){
    buttonCase = OPERATION_input;
  }  
  else{
    buttonCase = 0;
  }
    
  
  switch (buttonCase) {
    case OPERATION_enter:
      if(inputTemp == 0 && levelTemp == 0){
        break;
      }
//      activeChannel = inputTemp;
//      activeLevel = levelTemp;
      if(inputMode == 0) {
        activeLevel = EEPROM.read(activeChannel);
      }
      
      writeChannels();
      inputMode = 0;
      inputTemp = 0;
      levelTemp = 0;
      display.setCursor(64, 0);
      display.print("Set!");
      display.display();
      delay(100);
      update();
      break;
      
    case OPERATION_at:
      if(inputTemp == 0 && activeChannel == 0){
        break;
      }
      inputMode = 1;
      inputTemp = activeChannel;
      display.setCursor(75,0);
      display.print("@");
      display.display();
      delay(100);
      update();
      break;
    
    case OPERATION_full:
      levelTemp = 255;
      activeLevel = levelTemp;
      writeChannels();
      display.setCursor(75,0);
      display.print("Full");
      display.display();
      delay(100);
      update();
      break;
      
    case OPERATION_out:
      levelTemp = 0;
      activeLevel = levelTemp;      
      writeChannels();
      display.setCursor(75,0);
      display.print("Out");
      display.display();
      delay(100);
      update();
      break;
      
    case OPERATION_release:
      activeChannel = 0;
      activeLevel = 0;
      levelTemp = 0;
      inputTemp = 0;
      clearAll();
      update();
      break;
    
    case OPERATION_clear:
      inputMode = 0;
      activeChannel = 0;
      activeLevel = 0;
      inputTemp = 0;
      levelTemp = 0;
      update();
      break;      
    
    case OPERATION_input:
    if (inputMode == 0) {
        inputChar -= 48; //this is an ASCII offset. converts input value to integer value
        inputTemp = (inputTemp * 10) + inputChar; //concatenates each new digit
        if (inputTemp > 512) {
          inputTemp -= inputChar;
          inputTemp = inputTemp/10;
        }
        activeChannel = inputTemp;
        activeLevel = 0;
        update();
        break;
      }
      else if (inputMode == 1) {
        inputChar -= 48;
        levelTemp = (levelTemp * 10) + inputChar;
        if (levelTemp > 255) {
          levelTemp -= inputChar;
          levelTemp = levelTemp/10;
        }
        activeLevel = levelTemp;
        update();
        break;
      }
    case OPERATION_up:
      if(activeChannel == 0){
        break;
      }
      inputTemp = activeChannel;
      activeLevel += 15;
      if(activeLevel > 255){
        activeLevel = 255;
      }
      levelTemp = activeLevel;
      writeChannels();
      update();
      break;
      
    case OPERATION_down:
      if(activeChannel == 0){
        break;
      }
      inputTemp = activeChannel;
      activeLevel -= 15;
      if(activeLevel < 0){
        activeLevel = 0;
      }
      levelTemp = activeLevel;
      writeChannels();
      update();
      break;
    
    case OPERATION_left:
      if(activeChannel < 1){
        break;
      }
      activeChannel -= 1;
      activeLevel = EEPROM.read(activeChannel);
      inputTemp = 0;
      levelTemp = 0;
      update();
      break;
      
    case OPERATION_right:
      if(activeChannel > 511){
        break;
      }
      activeChannel += 1;
      activeLevel = EEPROM.read(activeChannel);
      levelTemp = 0;
      inputTemp = 0;
      update();
      break;
      
  }
  }


void writeChannels(){
  DmxMaster.write(activeChannel, activeLevel);
//  channelLevels[activeChannel] = activeLevel;
  EEPROM.write(activeChannel, activeLevel);
  inputTemp = 0;
  levelTemp = 0;
 }
 
void clearAll(){ //clearAll simply re-writes every DMX channel to 0
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0,0);
  display.print("RELEASE");
  display.display();
  for (i=1; i < 513; i++) {
    DmxMaster.write(i, 0);
    if(EEPROM.read(i) != 0){
      EEPROM.write(i, 0);
    }
  }    
}

void update(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("Ch=");
  if(activeChannel != 0){
    display.print(activeChannel);
  }
  display.setCursor(0,16);
  display.print("lvl=");
  display.print(activeLevel);
  display.display();
}

