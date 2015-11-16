/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | The latest version, 3.0, of the keypad library supports up to 10
|| | active keys all being pressed at the same time. This sketch is an
|| | example of how you can get multiple key presses from a keypad or
|| | keyboard.
|| #
*/

#include <Keypad.h>
#include <EEPROM.h>
#include <stdlib.h>

const byte ROWS = 2; //four rows
const byte COLS = 3; //three columns
//matrix layout numbered in chars to reference later in layers
char keys[ROWS][COLS] = {
  {'0', '1', '2'},
  {'3', '4', '5'},
};

int layer = 0;
int mode = 0; // 0 is normal, 1 is when keys 1 & 3 have been held 5 seconds, 2 is waiting for mode change input by user
int keyHolds[6] = {0,0,0,0,0,0};

int ledBlink = LOW;

//define LED pins on the board in order
const int ledPins[6] = {14, 15, 16, 2, 1, 0};


byte rowPins[ROWS] = {12, 13}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 9, 8}; //connect to the column pinouts of the keypad

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



int keyLayers[3][6] = {
  {'q', 'w', 'e', 'a', 's', 'd'},
  {KEY_RIGHT_SHIFT, KEY_UP_ARROW, KEY_RETURN, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW},
  {KEY_INSERT, KEY_HOME, KEY_PAGE_UP, KEY_DELETE, KEY_END, KEY_PAGE_DOWN},
};



//layer 5
int mouseCommands[6] = {MOUSE_LEFT, 3, MOUSE_RIGHT, 4, 5, 6}; 

void setup() {
  

  pinMode(ledPins[0], OUTPUT);
  pinMode(ledPins[1], OUTPUT);
  pinMode(ledPins[2], OUTPUT);
  pinMode(ledPins[3], OUTPUT);
  pinMode(ledPins[4], OUTPUT);
  pinMode(ledPins[5], OUTPUT);
 
  delay(2000);
  Serial.begin(9600);
  Mouse.begin();
  Keyboard.begin();
  //RawHID.begin();

  //get layer from saved data
  int checkLayer = EEPROM.read(0);
  if ( checkLayer > 5 ) { layer = 0; }
  else { layer = checkLayer; }

  kpd.setDebounceTime(1);

  activateLedMode();
  
}

unsigned long loopCount = 0;
unsigned long startTime = millis();

unsigned long mouseUpTime = 0;
unsigned long mouseLeftTime = 0;
unsigned long mouseDownTime = 0;
unsigned long mouseRightTime = 0;

void loop() {

  //If keys 1 and 3 have been held 3 seconds plus the two "one second loop counters" = 5 seconds, switch modes
  if (( keyHolds[0] > 3 ) && ( keyHolds[2] > 3 ) && ( mode == 0 )) {
    Keyboard.release(keyLayers[layer][0]);
    Keyboard.release(keyLayers[layer][2]);
    mode = 1;
    activateLedMode();
  }
  
  //if in mode 5 (mouse) then check if keys are held for continuous movement.
  //there is a 200 ms threshold to increase mouse movement, with a max of 5 px movement per cycle
  if ( layer == 5 ) {

       if ( keyHolds[1] != 0 ) { // move mouse up
         int negInt = keyHolds[1] * -1;
         Mouse.move(0, negInt); 
         if ( keyHolds[1] != 5 ) { 
           if ( (millis()-mouseUpTime)>200 ) {
             keyHolds[1]++; 
             mouseUpTime = millis();
           }
         }
       }
       
       if ( keyHolds[3] != 0 ) { // move mouse left
         int negInt = keyHolds[3] * -1;
         Mouse.move(negInt, 0); 
         if ( keyHolds[3] != 5 ) { 
           if ( (millis()-mouseLeftTime)>200 ) {
             keyHolds[3]++; 
             mouseLeftTime = millis();
           }
         }
       }
       
       if ( keyHolds[4] != 0 ) { // move mouse down
         Mouse.move(0, keyHolds[4]); 
         if ( keyHolds[4] != 5 ) { 
           if ( (millis()-mouseDownTime)>200 ) {
             keyHolds[4]++; 
             mouseDownTime = millis();
           }
         }
       }
       
       if ( keyHolds[5] != 0 ) { // move mouse right
         Mouse.move(keyHolds[5], 0); 
         if ( keyHolds[5] != 5 ) { 
           if ( (millis()-mouseRightTime)>200 ) {
             keyHolds[5]++; 
             mouseRightTime = millis();
           }
         }
       }

  }
  
  
  //the one second loop counter
  if ( (millis()-startTime)>1000 ) {
    startTime = millis();

    //if in programming mode, blink leds once each second
    if ( mode == 2) {
      if ( ledBlink == LOW ) { ledBlink = HIGH; }
      else if ( ledBlink == HIGH ) { ledBlink = LOW; }
      
      for (int lp = 0; lp<6; lp++) {
       digitalWrite(ledPins[lp], ledBlink); 
      }
  }


    //If keys 1 and 3 are held but other keys are not held, count it so above statement can catch it
    if ( mode == 0 ) {
      if  (( keyHolds[0] > 0 ) && ( keyHolds[1] == 0 ) && ( keyHolds[2] > 0 ) && ( keyHolds[3] == 0 ) && ( keyHolds[4] == 0 ) && ( keyHolds[5] == 0 )) {
        keyHolds[0] = keyHolds[0] + 1;
        keyHolds[2] = keyHolds[2] + 1;
      }
    }
  
  }//end of one second loop counter

  // Fills kpd.key[ ] array with up-to 10 active keys.
  // Returns true if there are ANY active keys.
  if (kpd.getKeys())
  { 
    for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
              pressKey(kpd.key[i].kchar); //subroutine handles pressing keys
              break;
            case HOLD:
              logHold(kpd.key[i].kchar); //subroutine handles holding keys
              break;
            case RELEASED:
              releaseKey(kpd.key[i].kchar); //subroutine handles releasing keys
             // break;
           // case IDLE: //no current need to handle idle keys
        }

      }
    }
  }
}  // End loop

//subroutine handles pressing keys
void pressKey(char sendKey) {

  //convert char of key to int so we don't need a set of case switches or if then elses
  int sendKeyInt = digit_to_int(sendKey);

   if (( layer == 0 ) || ( layer == 1 ) || ( layer == 2 )) {
     if ( mode == 0 ) { Keyboard.press(keyLayers[layer][sendKeyInt]); }
   }
   //layer 5 is mouse commands. let other subroutine handle that
   else if ( layer == 5 ) {
     mouseCommand(1, mouseCommands[sendKeyInt]);
   }

}

//subroutine handles releasing keys
void releaseKey(char sendKey) {
  

     if ( sendKey == '0' ) {
       keyHolds[0] = 0;
       if ( mode == 0 ) { Keyboard.release(keyLayers[layer][0]); }
       else if (( mode == 1 && keyHolds[2] == 0)) { mode = 2; }
       else if ( mode == 2 ) { mode = 0; layer = 0; activateKeyMode(0); }
       if ( layer == 5 ) { mouseCommand(0, mouseCommands[MOUSE_LEFT]); }
     }
     else if ( sendKey == '1' ) {
       keyHolds[1] = 0;
       if ( mode == 0 ) { Keyboard.release(keyLayers[layer][1]); }
       else if ( mode == 2 ) { mode = 0; layer = 1; activateKeyMode(1); }
     }
     else if ( sendKey == '2' ) {
       keyHolds[2] = 0;
       if ( mode == 0 ) { Keyboard.release(keyLayers[layer][2]); }
       else if (( mode == 1 && keyHolds[0] == 0)) { mode = 2; }
       else if ( mode == 2 ) { mode = 0; layer = 2; activateKeyMode(2); }
       if ( layer == 5 ) { mouseCommand(0, MOUSE_RIGHT); }
     }
     else if ( sendKey == '3' ) {
       keyHolds[3] = 0;
       if ( mode == 0 ) { Keyboard.release(keyLayers[layer][3]); }
       else if (( mode == 1 && keyHolds[0] == 0)) { mode = 2; }
       else if ( mode == 2 ) { mode = 0; layer = 3; activateKeyMode(3); }
     }
     else if ( sendKey == '4' ) {
       keyHolds[4] = 0;
       if ( mode == 0 ) { Keyboard.release(keyLayers[layer][4]); }
       else if (( mode == 1 && keyHolds[0] == 0)) { mode = 2; }
       else if ( mode == 2 ) { mode = 0; layer = 4; activateKeyMode(4); }
     }
     else if ( sendKey == '5' ) {
       keyHolds[5] = 0;
       if ( mode == 0 ) { Keyboard.release(keyLayers[layer][5]); }
       else if (( mode == 1 && keyHolds[0] == 0)) { mode = 2; }
       else if ( mode == 2 ) { mode = 0; layer = 5; activateKeyMode(5); }
     }

  
  
}

//subroutine handles holding keys
void logHold(int sendKey) {
  
  //remember that we are holding this key
  keyHolds[digit_to_int(sendKey)] = 1;

  //if mouse movement, remember what time key was pressed for acceleration threshold in main loop
  if ( layer == 5 ) {
    if ( sendKey == '1' ) { mouseUpTime =  millis(); }
    else if ( sendKey == '3' ) { mouseLeftTime = millis(); }
    else if ( sendKey == '4' ) { mouseDownTime =  millis(); }
    else if ( sendKey == '5' ) { mouseRightTime =  millis();  }   
  }
}

//when layer is changed
void activateKeyMode(int newLayer) {
 layer = newLayer;
 for (int kh = 0; kh<6; kh++) {
   keyHolds[kh] = 0;
 }
 EEPROM.write(0, newLayer);
 activateLedMode();

}

void activateLedMode() {
  
   //during mode 0 only one led is on, representing the layer. turn all leds off then turn that single led on
  if ( mode == 0 ) {
    ledBlink = LOW;
    for (int lp = 0; lp<6; lp++) {
      digitalWrite(ledPins[lp], ledBlink); 
    }
    //turn on only one led to represent the layer
    digitalWrite(ledPins[layer], HIGH);
  }
  //during mode 1 all leds turn on (waiting for user to choose layer) then blink during main loop
  else if ( mode == 1 ) {
    for (int lp = 0; lp<6; lp++) {
      digitalWrite(ledPins[lp], HIGH); 
    }
  }
  
}

//handling different mouse command actions
void mouseCommand(int method, int action) {
  
  //the left click is held & you right click, do a middle click
  if (( action == MOUSE_RIGHT ) && (Mouse.isPressed(MOUSE_LEFT))) {
    Mouse.click(MOUSE_MIDDLE);
  }
  //if the command sent is a left or right click, then if it's a click or a release. this is required for dragging.
  else if (( action == MOUSE_LEFT ) || ( action == MOUSE_RIGHT )) {
    if ( method == 0 ) { Mouse.release(action); }
    else if ( method == 1 ) { Mouse.press(action); }
  }
  else
  { 
       //during mouse movement, forget holding opposite direction in case both are held,
       //can't go opposite directions at same time because this will mess up diagonal movement
       if ( action == 3 ) { keyHolds[4] = 0; Mouse.move(0, -1); }// move mouse up, release down just in case
       else if ( action == 4 ) { keyHolds[5] = 0; Mouse.move(-1, 0); }// move mouse left, release right just in case
       else if ( action == 5 ) { keyHolds[1] = 0; Mouse.move(0, 1); }// move mouse down, release up just in case
       else if ( action == 6 ) { keyHolds[3] = 0; Mouse.move(1, 0); }// move mouse right, left just in case
  }  
  
}

//this is for converting a char to int. I tried making keys[ROWS][COLS] an int so that
//every subroutine used the same numbers, example keyHolds[i], but
//some subroutines failed to transfer the int variable to commands
int digit_to_int(char d)
{
 char str[2];
 str[0] = d;
 str[1] = '\0';

 return (int)strtol(str, (char **)NULL, 10);
}

