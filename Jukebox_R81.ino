//Robert DiNapoli 2024
v1.01

// 000 - stop current song. next song in queue should play. In record player mode, will return the record from the turntable to the magazine.
// 100 to 299 - queue the song to play 
// 600 to 699 - lights commands - see below
// 700 to 730 - set the volume - mp3 player only
// 750 - back to normal play mode, mode 0 - non continuous 
// 751 - continuous play, mode 1 - sequential play - current folder. must be free play, or have credits inserted. available on phono, but not for startup.
// 752 - continuous play, mode 2 - random play - current folder. must be free play, or have credits inserted. not available on phono
// 801 to 898 - if on mp3player, switch folder / directory ( 1 - 98)
// 995 - manual magazine scan - phono only
// 996 - encoder alignment test - phono only
// 997 - display preferences: version, folder, volume, play mode: 0 normal, 1 sequential, 2 random
// 998 - save preferences to the eeprom

//600 - lights off
//601 - orange solid on
//602 - red solid on
//603 - green solid on
//604 - blue solid on
//605 - custom color solid on
//606 - red alternative blink - 2 second timer
//607 - blue alternative blink - 2 second timer
//608 - green alternative blink - 2 second timer
//609 - custom alternative blink with custom timer
//610 - sparkle red - 2 second timer
//611 - sparkle green - 2 second timer
//612 - sparkle blue - 2 second timer
//613 - sparkle custom with custom timer - will sparkle with multiple colors!
//614 - sparkle2 - red green blue - 100ms timer
//615 - sparkle2 - light green, teal, skyblue - 100ms timer
//616 - sparkle2 - blue, white, aqua - 100ms timer
//617 - sparkle2 - red, white, blue - 100ms timer
//618 - sparkle2 - custom colors with custom timer
//619 - sparkle2 - random colors - 100ms timer
//620 - plasma - red green blue - 5 second timer
//621 - plasma - light green, teal, skyblue - 5 second timer
//622 - plasma - blue, white, aqua - 5 second timer
//623 - plasma - red, white, blue - 5 second timer
//624 - plasma - custom colors with custom timer
//625 - plasma - random colors - 5 second timer
//626 - cycle - red green blue - 4 second timer
//627 - cycle - light green, teal, skyblue - 4 second timer
//628 - cycle - blue, white, aqua - 4 second timer
//629 - cycle - red, white, blue - 4 second timer
//630 - cycle - custom colors with custom timer
//631 - cycle - random colors - 4 second timer
//632 - gradient - red green blue - 4 second timer
//633 - gradient - light green, teal, skyblue - 4 second timer
//634 - gradient - blue, white, aqua - 4 second timer
//635 - gradient - red, white, blue - 4 second timer
//636 - gradient - custom colors with custom timer
//637 - gradient - random colors - 4 second timer
//638 - fade colors - red green blue - 4 second timer
//639 - fade colors - light green, teal, skyblue - 4 second timer
//640 - fade colors - blue, white, aqua - 4 second timer
//641 - fade colors - red, white, blue - 4 second timer
//642 - fade colors - custom colors with custom timer
//643 - fade colors - random colors - 4 second timer
//644 - moving bars - red green blue - 4 second timer
//645 - moving bars - light green, teal, skyblue - 4 second timer
//646 - moving bars - blue, white, aqua - 4 second timer
//647 - moving bars - red, white, blue - 4 second timer
//648 - moving bars - custom colors with custom timer
//649 - moving bars - random colors - 4 second timer
//650 - glow - red - 7 second timer
//651 - glow - green - 7 second timer
//652 - glow - blue - 7 second timer
//653 - glow - red, customer color with custom timer
//654 - glow - random color - 7 second timer

//690 - color test - enter 000 000 000 to exit
//697 - input custom brightness values
//698 - input customer timer value
//699 - create custom pattern - 3 colors, timer and brightness
//jukebox selection - 100 - 299
//in the mp3 player, there is no position 0. entering 100 on the keypad stores song 1 in the queue
//for the phono player, magazine starts at postion 0 and go to 99. (uses the same queue for mp3 player, so need to adjust song number and subtract.)

//lights: make any selection, reset and reselect, your selection, record playing. we only handle make any selection or record playing
//interrupt 0(2), interrupt 1(3), interrupt 2(21), interrupt 3(20), interrupt 4(19), interrupt 5(18)
//interrupt 5 - pin 18 - coin insert



//this code uses the ALA library by bportaluri to make rgb light effects. Great concepts and code - but lots of serious bugs that make the code non functional
//in the aspect we need to use it.. It's been over 20 years since I've touched C/C++ and my memory mangagement / pointer skills are pretty rusty. I think I've patched
//everything enough to have it working properly, and also ripped out the code we don't need. Copy the ALA-FIXED directory to the arduino libraries folder.
//restart the arduino ide. you can then go sketch -> include library and select the ala-fixed library. 

#define RGBLIGHTS  //comment out this line if you aren't going to use RGB lights in your front panel

#include <SoftwareSerial.h>
#include <AbleButtons.h> //John Scott
#include <ArduinoQueue.h> //Einar Arnason
#include <Arduino.h>
#include <AceTMI.h> // SimpleTmi1637Interface //Brian T Park
#include <AceSegment.h> // Tm1637Module
#include <EEPROM.h> 
#ifdef RGBLIGHTS
  #include <Ala.h>
  #include "AlaLedRgb.h"
  AlaLedRgb leds;
#endif

//library for led digit display
using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;


//SoftwareSerial softSerial(10,11); //rx, tx
SoftwareSerial softSerial(12,13); //rx, tx

//setup song queue - as songs are entered, we throw them into this queue
struct songinfo
{
    int folder;
    int track;
};
typedef struct songinfo SongInfo;

//commands to the mp3 player need a delay between them, sometimes up to 800ms for the mp3 player busy to register after playing a song
//we will enter all mp3 player commands into a queue, and then pick them off one at a time after an appropriate delay
struct commandinfo
{
    uint8_t *cmd;
    int size;
};
typedef struct commandinfo CommandInfo;

#define VERSION 101
#ifdef RGBLIGHTS
  #define LED1RED 2
  #define LED1GREEN 3
  #define LED1BLUE 4
  #define LED2RED 5
  #define LED2GREEN 6
  #define LED2BLUE 7
  #define LED3RED 8
  #define LED3GREEN 9
  #define LED3BLUE 10
#endif
//rx - 12
//tx - 13
#define SWITCH_FREEPLAY 15  
#define SWITCH_MP3PLAYER 14  
#define SWITCH_CREDITS 18 //interrupt 5
#define CAM2_SWITCH 21
#define KEY_RESET 22
#define KEY_9 24
#define KEY_8 26
#define KEY_7 28
#define KEY_6 30
#define KEY_5 32
#define KEY_4 34
#define KEY_3 36
#define KEY_2 38
#define KEY_1 40
#define KEY_0 42
#define LED_RECORDPLAYING_LIGHT 44
#define LED_MAKEANYSELECTION_LIGHT 46
#define TOGGLESHIFT_COIL 47
#define MP3PLAYER_BUSY 48
#define MAGAZINE_MOTOR 49
#define LED_DATA_PIN 50 //spi data pin for led module tm1637
#define DETENT_COIL 51
#define LED_CLOCK_PIN 52 //spi clock pin for led module tm1637
#define TRANSFER_MOTOR 53 //record player



//input keypad setup
using Button = AblePullupClickerButton;
Button KeyReset(KEY_RESET);
Button Key9(KEY_9);
Button Key8(KEY_8);
Button Key7(KEY_7);
Button Key6(KEY_6);
Button Key5(KEY_5);
Button Key4(KEY_4);
Button Key3(KEY_3);
Button Key2(KEY_2);
Button Key1(KEY_1);
Button Key0(KEY_0);



//variables setup
int debugMode = 1; //log debug info to serial monitor
int isFreePlay = 0; //switch on - freeplay - switch off - use credits
int isMp3Player = 0; //switch on - use mp3player - switch off use physical phono
String inputSelectionString = ""; //command being entered
String currentSelectionString = ""; //current song playing
int credits = 0; //count of user credits
int creditFlag = 0; //credit switch was triggered from an Interrupt routine on value of 1
int inputSelectionNumber = 0; //numerical value of command entered by user
int currentFolderNumber = 1; // default to folder 1 by default. handle up to 98 folders
int makeAnySelectionLight = 0; //blue for testing
int recordPlayingLight = 0;  //green for testing
int isMP3Playing = 0; //1 - mp3 player or phono idle. 0 - a song is playing 
int inputInProgress = 0; //the user is currently entering a command
int playMode = 0; //0 - normal   1 - random   2 - sequential
int lastSongPlayed = 0; //for sequetial mode keep track of the last song played, or start at song 1
int currentVolume = 0; 
int totalSongsAvailable; //for the mp3 player, the total songs across all folders. only used for random mode 3
//led stuff
const uint8_t NUM_DIGITS = 4;
const uint8_t DELAY_MICROS = 100;
int ledDigits[3] = {10, 10, 10}; //value of 10 turns off the digits using the PATTERNS matrix below
String blinkString = ""; //if set, we are going to blink this string on the led display
int blinkCounter = 0; //counter used for how many times we have blinked already
int blinkAmount = 0; //how many times to blink
int blinkDelay = 300; //in milliseconds
const uint8_t PATTERNS2[75]= {
/*  0     1     2     3     4     5     6     7     8     9     :     ;     */
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x00, 
/*  <     =     >     ?     @     A     B     C     D     E     F     G     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 
/*  H     I     J     K     L     M     N     O     P     Q     R     S     */
    0x76, 0x30, 0x1E, 0x75, 0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x6D, 
/*  T     U     V     W     X     Y     Z     [     \     ]     ^     _     */
    0x78, 0x3E, 0x1C, 0x1D, 0x64, 0x6E, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 
/*  `     a     b     c     d     e     f     g     h     i     j     k     */
    0x00, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x30, 0x1E, 0x75, 
/*  l     m     n     o     p     q     r     s     t     u     v     w     */
    0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x1D, 
/*  x     y     z     */
    0x64, 0x6E, 0x5B
};
//song and command stucture / queues
SongInfo si; //one instance of a song structure for program use
CommandInfo ci; //one instance of a command structure for program use
ArduinoQueue<SongInfo> songList(200); //queue of songs
ArduinoQueue<CommandInfo> commandList(200); //queue of commands 
//timers
uint16_t blinkNowMillis = millis();
uint16_t blinkPrevMillis = millis();
uint16_t commandNowMillis = millis();
uint16_t commandPrevMillis = millis();
uint16_t playCommandNowMillis = millis();
uint16_t playCommandPrevMillis = millis();
uint16_t creditNowMillis = millis();
uint16_t creditPrevMillis = millis();
uint8_t nextcmd[] = {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF};
uint8_t stopcmd[] = {0x7E, 0xFF, 0x06, 0x0e, 0x00, 0x00, 0x00, 0xEF};
uint8_t playcmd[] = {0x7E, 0xFF, 0x06, 0x0f, 0x00, 0x02, 0x01, 0xEF};
uint8_t volcmd[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 0x1e, 0xEF}; //max volume of 30
/*a query with 0x48 will show you the number of songs on the whole flash card - in all folders. so if have 2 songs in folder 01 and 4 songs in
/folder 2, the 48 query will return 6 songs. Using the 0x08 command, you can play the absolute song number - no matter which folder the song is in*/
/*I was intending to use this as a random player for the whole flash card... but maybe in a later version*/
uint8_t querycmd[] = {0x7E, 0xFF, 0x06, 0x48, 0x00, 0x00, 0x00, 0xEF}; 
uint8_t playabsolutesongnumbercmd[] = {0x7E, 0xFF, 0x06, 0x08, 0x00, 0x00, 0xca, 0xEF};
//record player stuff
boolean cs2; //cam switch 2
int isRecordPlaying = 0; //0 - no record playing  1 - record playing
int scanningStage = 0; //0 - no scanning   1 - pre scan    2 - scan
int recordNumberOrigHex = 0; //the record number to play 0 - 99 - does not get modified
int recordNumberAdjHex = 0; //the record number to play 0 - 99 - will be adjusted for pre scan
int activateToggleShiftCoil = 0; // 1 - activate toggle shift and play opposite side of the record
int isMagazineActiveFlag = 0; //0 - magazine off. 1 - detent coil is engaged and magazine turning
int transferRecordStage = 0;  //0 - no action. 1 - we need to transfer a record to the turntable  2 - record on turntable and will need to be returned
uint16_t transferRecordNowMillis = millis();
uint16_t transferRecordPrevMillis = millis();
uint16_t scanRecordWatchdogNowMillis = millis();
uint16_t scanRecordWatchdogPrevMillis = millis();
int transferRecordTimerFlag = 0; //set to 1 to initiate a delay of 2000 seconds in transferRecord()
//rgb lights
#ifdef RGBLIGHTS
  byte customColor1R = 255;
  byte customColor1G = 165;
  byte customColor1B = 0;
  byte customColor2R = 255;
  byte customColor2G = 165;
  byte customColor2B = 0;
  byte customColor3R = 255;
  byte customColor3G = 165;
  byte customColor3B = 0;
  int customColorTimer = 1000;
  byte customBrightnessR = 55;
  byte customBrightnessG = 55;
  byte customBrightnessB = 55;
  int currentLightsPattern = ALA_OFF; //default lights off
#endif 

//led digit display setup
using TmiInterface = SimpleTmi1637Interface;
TmiInterface tmiInterface(LED_DATA_PIN, LED_CLOCK_PIN, DELAY_MICROS);
Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);


/*====================================================================================================================*/
/*====================================================================================================================*/
void setup() {
delay(1000); //stabilize before starting
//pin setup  
pinMode(SWITCH_MP3PLAYER, INPUT_PULLUP); //14
pinMode(SWITCH_FREEPLAY, INPUT_PULLUP); //15
pinMode(SWITCH_CREDITS, INPUT_PULLUP); //18
pinMode(CAM2_SWITCH, INPUT_PULLUP); //21 //Record player
pinMode(23, INPUT_PULLUP); //encoder - 1 position
pinMode(25, INPUT_PULLUP); //encoder - 2 position
pinMode(27, INPUT_PULLUP); //encoder - 4 position
pinMode(29, INPUT_PULLUP); //encoder - 8 position
pinMode(31, INPUT_PULLUP); //encoder - 16 position
pinMode(33, INPUT_PULLUP); //encoder - 32 position
pinMode(35, INPUT_PULLUP); //encoder - 64 position
pinMode(37, INPUT_PULLUP); //encoder - 128 position
pinMode(LED_RECORDPLAYING_LIGHT, OUTPUT); //44
pinMode(LED_MAKEANYSELECTION_LIGHT, OUTPUT); //46
pinMode(MP3PLAYER_BUSY, INPUT); //48
digitalWrite(TOGGLESHIFT_COIL, HIGH); //setup pin so that LOW triggers on
pinMode(TOGGLESHIFT_COIL, OUTPUT);  //47
digitalWrite(MAGAZINE_MOTOR, HIGH); //setup pin so that LOW triggers on
pinMode(MAGAZINE_MOTOR, OUTPUT);  //49
digitalWrite(DETENT_COIL, HIGH); //setup pin so that LOW triggers on
pinMode(DETENT_COIL, OUTPUT);  //51
digitalWrite(TRANSFER_MOTOR, HIGH); //setup pin so that LOW triggers on
pinMode(TRANSFER_MOTOR, OUTPUT);  //53
attachInterrupt(digitalPinToInterrupt(SWITCH_CREDITS), addCreditFlag, FALLING); //set up an interrupt to capture credit insert

//led display digits setup
tmiInterface.begin();
ledModule.begin();
ledModule.setBrightness(7); //full brightness?

Serial.begin(9600);
while (!Serial) {
  delay(10);
}
softSerial.begin(9600);

debugSerial("Initializing");
ledUpdateDigitsArray("rst");
delay(1000);

Button::setDebounceTime(30);
digitalWrite(LED_RECORDPLAYING_LIGHT, LOW);
digitalWrite(LED_MAKEANYSELECTION_LIGHT, LOW);


//check pcb switches
isFreePlay = digitalRead(SWITCH_FREEPLAY); 
isMp3Player = digitalRead(SWITCH_MP3PLAYER);
debugSerial("isFreePlay: " + String(isFreePlay));
debugSerial("isMp3Mplayer: " + String(isMp3Player));
if (isMp3Player == 1) {
  ledUpdateDigitsArray("mp3");
} else {
  ledUpdateDigitsArray("pho");
}
delay(1000);

//key setup
KeyReset.begin();
Key9.begin();
Key8.begin();
Key7.begin();
Key6.begin();
Key5.begin();
Key4.begin();
Key3.begin();
Key2.begin();
Key1.begin();
Key0.begin();

    


//stopPlaying(); //stop playing any song
if (isMp3Player == 1) { //if are on mp3 player, get the total songs available. //not currently used
  getTotalSongsAvailable();
  debugSerial("Total Songs on Mp3 Player: " + String(totalSongsAvailable));
}

readPreferencesFromEEPROM();  //pull volume, play mode, folder number, etc from memory
//if we using phono, do not auto start playing records from sequential mode or random mode.
//The reason for this, is that we don't know if a record is currently on the turntable from a bad shutdown
if (isMp3Player == 0) {
  playMode = 0;
}
//play startup sound on startup, if one exists.
//playcmd[5] = 99;
//playcmd[6] = 1;
//softSerial.write(playcmd,8); 
//delay(200);
#ifdef RGBLIGHTS
  lightsSetup(); //front panel rgb lights setup
#endif

} //end setup function



/* ---------------------------------------------------------------------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------------------------------------------------------------------*/
void loop() {
  displayMakeAnySelectionLight();
  getDigitsInput(); // see if the user has entered any digits
  if (inputSelectionString.length() ==3 ) { //user has entered a complete 3 digit song number or command
    processInput(); //if it's a valid song number or command, add it to the appropriate song or commmand queue
  }
  processCommandFromQueue(); //process any commands in queue
  checkForNextSong(); //check if a song is playing. If no song playing, and there are songs in queue, grab the next song from queue and play it.
  ledUpdateDisplay(); //decide what to print on the led array, either the current inputstring, or the current song playing, or blink the text for a command entered
  checkForCreditFlag(); //ISR will flag that the credit switch was triggered. Handle it here.
  if (isMp3Player == 0) { //phono mode
    scanForRecordAsync(); //check to see if we need to scan for a new record. triggered by setting scanningState to 1 in playSong()
    transferRecordAsync(); //check to see if we need to transfer a record from the magazine. triggered by setting transferRecordStage to 1 in scanForRecordAsync()
    returnRecordAsync(0); //check to see if we need to return record to magazine. triggered by transferRecordStage >=4. set from transferRecordAsync
  }  

#ifdef RGBLIGHTS
  leds.runAnimation();
#endif  

} //end loop function




/* ---------------------------------------------------------------------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------------------------------------------------------------------*/
//queue a command for the mp3 player
void addCommandToQueue(uint8_t cmd[], int size) {
  ci.cmd = cmd;
  ci.size = size;
  commandList.enqueue(ci);
  //debugSerial("Adding command to queue: " + String(cmd[0]) + " - items in queue: " + String(commandList.itemCount())); 
} //end function addCommandToQueue



//interrupt 0 pin 2 - I never want to lose a coin insert. however, you can't debounce in an ISR, so set a flag, and do the debounce elsewhere
void addCreditFlag() {
    if (isFreePlay == 0) {
      creditFlag = 1;
      creditNowMillis = millis();
      creditPrevMillis = millis();
      //debugSerial("credit flag set");
    }
} //end function addCreditFlag


//for continuous play mode 2, choose a random song from 1 to 200.
void addRandomSongFromCurrentFolderToQueue() {
  randomSeed(millis());
  addSongToQueue(currentFolderNumber, random(1,200));
} //end function addRandomSongFromCurrentFolderToQueue;


//for continuous play mode 1, we will increment the song by 1
void addSequentialSongToQueue() {
  lastSongPlayed = lastSongPlayed + 1;
  if (lastSongPlayed == 201) { 
    lastSongPlayed = 1;
  }
  addSongToQueue(currentFolderNumber, lastSongPlayed);
} //end function addSequentialSongToQueue


//add a song to the song queue. subtract a credit if not in free play mode
void addSongToQueue(int folder, int track) {
  if (isFreePlay == 0 && credits > 0) {
    credits = credits - 1;
    debugSerial("credit count: " + String(credits));
  }
  si.folder = folder;
  si.track = track;
  songList.enqueue(si);
  debugSerial("Folder: " + String(folder) + "  Track: " + String(track) + " added to song queue" +  " - items in queue: " + String(songList.itemCount())); 
} //end function addSongToQueue




//if the credit flag has been set from the ISR, we will wait 400ms before adding the credit
//therefore avoiding any noise from the credit switch press
void checkForCreditFlag() {
    if (creditFlag == 1) {
      creditNowMillis = millis();
      if ((uint16_t) (creditNowMillis - creditPrevMillis) >= 400) { 
        creditPrevMillis = creditNowMillis;
        creditFlag = 0;
        credits = credits + 1;
        //if (isSongPlaying == 1) { //play a credit sound, as long as no mp3 or record is playing
          //playcmd[5]=99;
          //playcmd[6]=2;
          //softSerial.write(playcmd,8);
        //}
        debugSerial("Credit added. Credits: " + String(credits)); 
      } //end if timer says we can trigger credit
    } //end if credit flag set
} //end function checkForCreditFlag




//check for a song playing. If no song is playing, and there is a song in queue,
//grab the next item from the song queue and play it
void checkForNextSong() {
  if (isMp3Player == 1) {
    //we need to wait at least 2 seconds since the play command to make sure the mp3 player has gone "busy"
    playCommandNowMillis = millis();
    if ((uint16_t) (playCommandNowMillis - playCommandPrevMillis) >= 2500) { 
      playCommandPrevMillis = playCommandNowMillis;
      isMP3Playing = digitalRead(MP3PLAYER_BUSY); //1 - idle //0 - song playing
      if (isMP3Playing == 1) { //if the mp3 player is idle
        if (recordPlayingLight == 1) {
          debugSerial("Record playing light turned off");
          recordPlayingLight = 0;
          digitalWrite(LED_RECORDPLAYING_LIGHT, LOW);
        }
        //if the song list is not empty, grab the next song in queue, and send the play command. otherwise, clear the led display
        if (!songList.isEmpty()) {
          processSongFromQueue();
        } else {
          //we are idle, and the song list is empty
          currentSelectionString = "";
          if (playMode ==1 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if sequential play is on, and we are on free play or have credits left, add the next incremental song to the queue
            addSequentialSongToQueue();
          } else if (playMode ==2 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if random play is on, and we are on free play or have credits left, add a random song to the queue
            addRandomSongFromCurrentFolderToQueue(); //disabling... lots of extra wear on magazine.
          }  
        } //no songs in queue
      } // no song playing
    } //timer not met
  } else { //phono stuff here

  //we need to wait at least a second since the play command to make sure the record player has gone busy
    playCommandNowMillis = millis();
    if ((uint16_t) (playCommandNowMillis - playCommandPrevMillis) >= 1500) { 
      playCommandPrevMillis = playCommandNowMillis;
      if (isRecordPlaying == 0 && transferRecordStage == 0 && scanningStage == 0) { //phono is idle
        if (recordPlayingLight == 1) {
          debugSerial("Record playing light turned off");
          recordPlayingLight = 0;
          digitalWrite(LED_RECORDPLAYING_LIGHT, LOW);
        }
        //if the song list is not empty, grab the next song in queue, and send the play command. otherwise, clear the led display
        if (!songList.isEmpty()) {
          processSongFromQueue();
        } else {
          //we are idle, and the song list is empty
          currentSelectionString = "";
          if (playMode ==1 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if sequential play is on, and we are on free play or have credits left, add the next incremental song to the queue
            addSequentialSongToQueue();
          } else if (playMode ==2 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if random play is on, and we are on free play or have credits left, add a random song to the queue
            //addRandomSongFromCurrentFolderToQueue();
          }  
        } //no songs in queue
      } // no song playing
    } //timer not met
  } //end is mp3player?
} //end function checkForNextSong



void debugSerial(String str) {
  if (debugMode == 1) {
      Serial.print(str + "\r\n");
  }    
  } //end debug_serial function




//turn the make any selection light on / off depending on credits or the freeplay switch
void displayMakeAnySelectionLight() {
  if (isFreePlay ==1 || credits > 0) {
    //turn the make any selection light on
    if (makeAnySelectionLight == 0) {
      digitalWrite(LED_MAKEANYSELECTION_LIGHT,HIGH);
      makeAnySelectionLight = 1;
    }
  } else { //turn it off if no credits or not in free play mode 
      if (makeAnySelectionLight == 1) {
        digitalWrite(LED_MAKEANYSELECTION_LIGHT,LOW);
        makeAnySelectionLight = 0;
      }  
  } //handle the display light depending on free play mode or credits
} //end function displayMakeAnySelectionLight



//display volume, current folder, current play mode, version
void displayPreferences() {
  debugSerial("Values:  volume: " + String(currentVolume) + "  currentFolder: " + String(currentFolderNumber) + "  lastSongPlayed: " + String(lastSongPlayed+99) + "  playMode: " + String(playMode));
  ledUpdateDigitsArray("Ver");
  delay(1000);
  ledUpdateDigitsArray(String(VERSION));
  delay(1000);
  ledUpdateDigitsArray("Fol");
  delay(1000);
  ledUpdateDigitsArray(String(currentFolderNumber));
  delay(1000);
  ledUpdateDigitsArray("Sng");
  delay(1000);
  ledUpdateDigitsArray(String(lastSongPlayed+99));
  delay(1000);
    ledUpdateDigitsArray("vol");
  delay(1000);
  ledUpdateDigitsArray(String(currentVolume));
  delay(1000);
  ledUpdateDigitsArray("mod");
  delay(1000);
  if (playMode == 0) {
    ledUpdateDigitsArray("normal");
  } else if (playMode == 1) {
    ledUpdateDigitsArray("seq");
  } else if (playMode == 2) {
    ledUpdateDigitsArray("ran");
  }
  delay(1000);
  //char cr[] = ":::dinapoli:2023:::";
  //for (int i=0; i< strlen(cr); i++) {
    //String x = String(cr).substring(i,i+3);
    //ledUpdateDigitsArray(x);
    //delay(200);
  //}
  
} //end function displayPreferences



void encoderAlignment() { //command 996
int x = 0;
int exitFlag = 0;
int encoderValue = 0;

debugSerial("Encoder alignment. Press any number to exit.");
ledUpdateDigitsArray("enc");
delay(400);

while (exitFlag == 0) {
//See which record is currently at the position in carousel
  encoderValue = readEncoder();
  //Serial.println(dataByte,HEX);
  ledUpdateDigitsArray(String(encoderValue, HEX));
  delay(20);
  inputSelectionString = "";
  getDigitsInput();
  if (inputSelectionString > "") {
    debugSerial("Key Pressed. Exiting encoder alignment check.");
    exitFlag = 1;
    inputSelectionString = "";
  } //if - check for keypress
} //end while

} //end function encoderAligmment




//get a digit 0 - 9 or the reset key. append it to a selection string
//if the reset key is pressed, zero out the selection string
void getDigitsInput() {
 
    Key0.handle();
    Key1.handle();
    Key2.handle();
    Key3.handle();
    Key4.handle();
    Key5.handle();
    Key6.handle();
    Key7.handle();
    Key8.handle();
    Key9.handle();
    KeyReset.handle();
    if (Key0.resetClicked()) {
      inputSelectionString = inputSelectionString + "0";
    } else if (Key1.resetClicked()) {
      inputSelectionString = inputSelectionString + "1";
    } else if (Key2.resetClicked()) {
      inputSelectionString = inputSelectionString + "2";
    } else if (Key3.resetClicked()) {
      inputSelectionString = inputSelectionString + "3";
    } else if (Key4.resetClicked()) {
      inputSelectionString = inputSelectionString + "4";
    } else if (Key5.resetClicked()) {
      inputSelectionString = inputSelectionString + "5";
    } else if (Key6.resetClicked()) {
      inputSelectionString = inputSelectionString + "6";
    } else if (Key7.resetClicked()) {
      inputSelectionString = inputSelectionString + "7";
    } else if (Key8.resetClicked()) {
      inputSelectionString = inputSelectionString + "8";
    } else if (Key9.resetClicked()) {
      inputSelectionString = inputSelectionString + "9";
    } else if (KeyReset.resetClicked()) {
      inputSelectionString = "";
      //debugSerial("Reset clicked");
    } 
  } //end function getDigitsInput



//query the mp3 player to see how many songs are available
void getTotalSongsAvailable() {
  softSerial.listen();
  static int reply[10];
  static unsigned int messagePosition = 0;
  softSerial.write(querycmd,8);
  delay(200);
    while(softSerial.available() > 0) {
    int c = softSerial.read();
    reply[messagePosition] = c;
    messagePosition++;
  }
  totalSongsAvailable  = (reply[5]<<8) + reply[6]; 
} //end function getTotalSongsAvailable;




//setup the variables to cause a blink on the led display
void ledBlinkSetup(String b, int count, int delay) {
  blinkString = b;
  blinkAmount = count;
  blinkDelay = delay;
  blinkCounter = 0; //reset the blink counter
} //end function ledBlinkSetup



void ledDisplayString(String displayString) {
//char cr[] = ":::dinapoli:2024:::";
char cr[displayString.length()];
displayString.toCharArray(cr, displayString.length() + 1);

  for (int i=0; i< strlen(cr); i++) {
    String x = String(cr).substring(i,i+3);
    ledUpdateDigitsArray(x);
    delay(200);
  }
}



  //update the led display with whatever needs to be done
  //This may be changing the display to show current input versus a song playing
  //or blinking the display with a specified string
void ledUpdateDisplay() {
   if (inputSelectionString > "") inputInProgress = 1; else inputInProgress = 0;
   if (blinkString == "") {
     //if the user is doing input, show the input from the user
    if (inputInProgress == 1) { 
      ledUpdateDigitsArray(inputSelectionString);
    } else {
      // Otherwise, show the current playing song (will be blank if no song playing)
      ledUpdateDigitsArray(currentSelectionString);
    } //endif input in progress
   } else {
     //handle the blinking of an entered command
     if (blinkCounter < blinkAmount) {
        blinkNowMillis = millis();
        if ((uint16_t) (blinkNowMillis - blinkPrevMillis) > (blinkDelay*2)) { 
          blinkPrevMillis = blinkNowMillis; 
          blinkCounter = blinkCounter + 1;
        } else if ((uint16_t) (blinkNowMillis - blinkPrevMillis) >= blinkDelay) { //blink off
          ledUpdateDigitsArray(""); //update the leddigits array, given a string
        } else if ((uint16_t) (blinkNowMillis - blinkPrevMillis) >= 0) { //blink on
          ledUpdateDigitsArray(blinkString);
        }  
      } else { //blink counter > blink amount, reset it
        blinkCounter = 0;
        blinkString = "";
      } //end counter 
   } //end if blinkString > ""
} //end function ledUpdateDisplay



//given a string containing 0 -3 digits, fill the ledDigits array with the numbers 1 - 9 for each led position. Or number 10 for a blank digit
void ledUpdateDigitsArray(String s) {
  //debugSerial("led string: " + s);
  if (s.length() == 0) {
    ledDigits[2] = 10; //off
    ledDigits[1] = 10; //off
    ledDigits[0] = 10; //off
  }else if (s.length() == 1) {
    ledDigits[2] = 10; //off
    ledDigits[1] = 10; //off
    ledDigits[0] = s[0] - 48;
  } else if (s.length() ==2) {
    ledDigits[2] = 10; //off
    ledDigits[1] = s[0]-48;  
    ledDigits[0] = s[1]-48;
  } else {
    ledDigits[2] = s[0]-48;
    ledDigits[1] = s[1]-48;
    ledDigits[0] = s[2]-48;
    //debug_serial("digit1:" + String(ledDigits[2]) + " digit2:" + String(ledDigits[1]) + " digit3:" + String(ledDigits[0]));
  }
  ledSetDigits(); //set the pattern for each led on the physical display, using the leddigits array
} //end ledUpdateDigitsArray



//get the pattern for each digit in the ledDigits array, and put it on the physical led module for each position
void ledSetDigits() {
  //debugSerial("digit1 output: " + String(ledDigits[2]) + " digit2 output :" + String(ledDigits[1]) + " digit3 output: " + String(ledDigits[0]));
  // Update the display
  uint8_t pattern = PATTERNS2[ledDigits[0]]; 
  ledModule.setPatternAt(2, pattern);
  uint8_t pattern2 = PATTERNS2[ledDigits[1]]; 
  ledModule.setPatternAt(1, pattern2);
  uint8_t pattern3 = PATTERNS2[ledDigits[2]]; 
  ledModule.setPatternAt(0, pattern3);
  ledModule.flush();
} //end ledSetDigits



int numberDecimalToHex(int recordNumber) {
//debugSerial("Decimal to hex input number: " + String(recordNumber, HEX));
char recordHexChar[5];
String recordStr = "0x"+String(recordNumber); //this is an integer, but we need to treat it as a hex number for the encoder.
recordStr.toCharArray(recordHexChar, recordStr.length() + 1);
int finalValue = strtol(recordHexChar,NULL,16);
//debugSerial("Decimal to hex output number: " + String(finalValue, HEX));
return finalValue;
} //end function numberDecimalToHex

int numberHexToDecimal(int recordNumber) {
//debugSerial("Hex to decimal input number: " + String(recordNumber, HEX));
char recordHexChar[5];
String recordStr = String(recordNumber, HEX);
recordStr.toCharArray(recordHexChar, recordStr.length() + 1);
int finalValue = strtol(recordHexChar, NULL, 10);
//debugSerial("Hex to Decimal output number: " + String(finalValue, HEX));
return finalValue;
} //end function numberHexToDecimal



//Queue a command to play a song on the mp3 player, or the phono
//in mp3 mode, songs go from 100 - 299. song 100 plays item #1 on the mp3 player. entering song 178 on the keypad stores song 79 in the playlist
//in phono mode: entering 100 (#1 stored in song queue) should play record #100. 101 (stored as #2 in playlist) should play record in position 1)
//phono mode: 199 should play record in carousel position 99
void playSong(uint8_t cmd[], int size) {
   debugSerial("Record playing light turned on");
   digitalWrite(LED_RECORDPLAYING_LIGHT, HIGH);
   recordPlayingLight = 1;
   if (isMp3Player==1) {
     debugSerial("Mp3 play song command issued");
     addCommandToQueue(cmd, size);
     playCommandNowMillis = millis(); // reset timer
     playCommandPrevMillis = millis();
   } else //phono mode
   {
    debugSerial("Phono play song command issued"); 
    //Create the proper record number to play for the phono
    int recordNumber = cmd[6]; //grab the song number from the song queue
    recordNumber = recordNumber - 1; //adjust for phono carousel. memory will have song 63, it's really record position 62
    if (recordNumber >= 100) {
      recordNumber = recordNumber - 100;
      activateToggleShiftCoil = 1;
    } else if (recordNumber == 100 ) {
      recordNumber = 0;
    }
    recordNumberOrigHex = numberDecimalToHex(recordNumber); 
    //create the proper record number to pre scan for (record number - 1)
    int recordNumberTemp = numberHexToDecimal(recordNumberOrigHex);
    recordNumberTemp = recordNumberTemp - 1; 
    if (recordNumberTemp < 0) {
      recordNumberTemp = 99;
    }
    recordNumberAdjHex = numberDecimalToHex(recordNumberTemp);
    debugSerial("Jukebox song number to play: " + String(recordNumberOrigHex, HEX));
    scanningStage = 1; //flag that we need to start scanning
    scanRecordWatchdogNowMillis = millis();
    scanRecordWatchdogPrevMillis = millis();
    transferRecordStage = 0;
   }
} //end function playSong


//pull a command from the command queue, and send it to the mp3 player
void processCommandFromQueue() {
  if (!commandList.isEmpty()) {
    //only execute a command once per second
    commandNowMillis = millis();
    if ((uint16_t) (commandNowMillis - commandPrevMillis) >= 1500) { 
      commandPrevMillis = commandNowMillis;
      ci = commandList.dequeue();
      debugSerial("Command pulled from queue - cmd: " + String(*ci.cmd) + "  items in queue: " + String(commandList.itemCount())); 
      softSerial.write(ci.cmd,ci.size);
    } //timer criteria is met
  } //a command exists in the queue
} // end function processCommandFromQueue



//take action based on our selection string, which must be 3 digits to get to this routine
void processInput() {
  inputSelectionNumber = inputSelectionString.toInt();
  if (inputSelectionNumber == 0) {
    stopPlaying();
  } else if (inputSelectionNumber >= 100 && inputSelectionNumber <= 299 ) { //add the song to queue
      if (isFreePlay == 1 || (isFreePlay == 0 && credits > 0)) { //if free play, or not free play and credits available, add song to queue
        addSongToQueue(currentFolderNumber, inputSelectionNumber - 99);
        ledBlinkSetup(inputSelectionString,5,250);
      } 
  } else if (inputSelectionNumber >= 600 && inputSelectionNumber <= 699) { //lights stuff
    //handle lights stuff here
    #ifdef RGBLIGHTS
      lightsSelector();
    #endif  
  } else if (inputSelectionNumber >= 700 && inputSelectionNumber <= 730) { //set volume
    currentVolume = inputSelectionNumber - 700;
    volcmd[6] = currentVolume;
    debugSerial("Volume level set to: " + String(inputSelectionNumber - 700));
    addCommandToQueue(volcmd, 8);
    ledBlinkSetup(inputSelectionString,4,150);
  } else if (inputSelectionNumber == 751) {
     playMode = 1;
     debugSerial("Continuous sequential play turned on");
     ledBlinkSetup(inputSelectionString,4,150);
  } else if (inputSelectionNumber == 752) {
     playMode = 2;
     debugSerial("Continuous random play for current folder turned on");
     ledBlinkSetup(inputSelectionString,4,150);
  } else if (inputSelectionNumber == 750) { //normal play mode
     playMode = 0;
     debugSerial("Normal play mode");
     ledBlinkSetup(inputSelectionString,4,150);
  } else if (isMp3Player && (inputSelectionNumber >= 801 && inputSelectionNumber <= 898)) { //change folder
     currentFolderNumber = inputSelectionNumber - 800;
     debugSerial("Folder number changed to: " + String(currentFolderNumber));
     ledBlinkSetup(inputSelectionString,4,150);
  } else if (inputSelectionNumber == 995 ) { //manual scan
    ledBlinkSetup(inputSelectionString,4,150);
    scanManual();
  } else if (inputSelectionNumber == 996 ) { //encoder alignment
     ledBlinkSetup(inputSelectionString,4,150);
     encoderAlignment(); 
  } else if (inputSelectionNumber == 997 ) { //display eeprom settings
     ledBlinkSetup(inputSelectionString,4,150);
     displayPreferences();
  } else if (inputSelectionNumber == 998 ) { //save eeprom settings
     ledBlinkSetup(inputSelectionString,4,150);
     savePreferencesToEEPROM();
  }

  inputInProgress = 0 ; //the display can go back to displaying the song in progress, if one is playing
  inputSelectionString = ""; //we've processed the input string - clear it out
} //end function processInput



//if a song exists in the song queue, grab it, and send it to be played
void processSongFromQueue() {
  si = songList.dequeue();
  debugSerial("Song pulled from queue - track: " + String(si.track) + "  folder: " + String(si.folder) + "  items in queue: " + String(songList.itemCount())); 
  char buffer[5];
  sprintf(buffer, "%03d", si.track + 99); //3 digits, left filled zeros
  currentSelectionString = String(buffer);
  if (inputInProgress ==0 ) { //if the user is not entering any data, immediately update the display with the current song number
    ledUpdateDigitsArray(currentSelectionString);
  }
  playcmd[5] = si.folder;
  playcmd[6] = si.track;
  lastSongPlayed = si.track;
  playSong(playcmd,8); 
} //end function processSongFromQueue




int readEncoder() {
int encoderValue;  
byte encoderPinArray[] = {23,25,27,29,31,33,35,37}; // MSB is the rightmost
int newBit; //used for reading encoder
int dataByte; //used for reading encoder
int x=0;

dataByte = 0;
for (x=0; x<8; x=x+1){
   newBit = digitalRead(encoderPinArray[x]); // read a pin
   dataByte = dataByte | (newBit<<x); //shift the bit into position and put in its place in the data byte
}
encoderValue = dataByte;
return encoderValue;
} //end function readEncoder




int readEncoderDouble() {
//lots of bad data when reading the encoder, so we'll read it twice with a quick compare to weed out bad data
int finalValue;
int value1 = readEncoder();
//debugSerial("Initial value1: " + String(value1,HEX));
delay(5);

int value2 = readEncoder();
//debugSerial("Initial value2: " + String(value2,HEX));
if (value2 == value1) {
  finalValue = value1; //if we read it twice, and it's the same, exit this routine
} else {
  finalValue = 999;
}

return finalValue;
} //end function readEncoderDouble





//see if we have EEPROM data stored. If so, use it to set default values on startup
void readPreferencesFromEEPROM() {
  currentVolume = readIntFromEEPROM(0); //by default, if this value was never written, it will be 255
  if (currentVolume != 255) {
    currentFolderNumber = readIntFromEEPROM(2);
    lastSongPlayed = readIntFromEEPROM(4); 
    playMode = readIntFromEEPROM(6);
    debugSerial("Values from EEPROM: volume: " + String(currentVolume) + "  currentFolder: " + String(currentFolderNumber) + "  lastSongPlayed: " + String(lastSongPlayed+99) + "  playMode: " + String(playMode));
    volcmd[6] = currentVolume;
    addCommandToQueue(volcmd, 8);
    #ifdef RGBLIGHTS
      currentLightsPattern = readIntFromEEPROM(8); //bytes 8 and 9
      customColor1R = EEPROM.read(10);
      customColor1G = EEPROM.read(11);
      customColor1B = EEPROM.read(12);
      customColor2R = EEPROM.read(13);
      customColor2G = EEPROM.read(14);
      customColor2B = EEPROM.read(15);
      customColor3R = EEPROM.read(16);
      customColor3G = EEPROM.read(17);
      customColor3B = EEPROM.read(18);
      customBrightnessR = EEPROM.read(19);
      customBrightnessG = EEPROM.read(20);
      customBrightnessB = EEPROM.read(21);
      customColorTimer = readIntFromEEPROM(22);
      debugSerial("Values from EEPROM: color1r: " + String(customColor1R) + "  color1g: " + String(customColor1G) + "  color1b: " + String(customColor1B) );
      debugSerial("Values from EEPROM: color2r: " + String(customColor2R) + "  color2g: " + String(customColor2G) + "  color2b: " + String(customColor2B) );
      debugSerial("Values from EEPROM: color3r: " + String(customColor3R) + "  color3g: " + String(customColor3G) + "  color3b: " + String(customColor3B) );
      debugSerial("Values from EEPROM: colorbrightnessr: " + String(customBrightnessR) + "  colorbrightnessg: " + String(customBrightnessG) + "  colorbrightnessb: " + String(customBrightnessB) );
      debugSerial("Values from EEPROM: color pattern: " + String(currentLightsPattern) + "  color timer: " + String(customColorTimer));
      inputSelectionString = String(currentLightsPattern);
      lightsSelector();
    #endif
  } else { //volume was set to 255... which is impossible - so we know we can't use the eeprom values}
    debugSerial("EEPROM has default values");
    currentVolume = 20; //reset the currentVolume to the default value
    addCommandToQueue(volcmd, 8);
  }  
} //end function readPreferencesFromEEPROM




void returnRecordAsync(int forceReturn) {
 //return a record from the turntable to the carousel
 //Stage 4 - check for CS2 to go LOW again
 //Stage 5 - wait for 3000ms
 //Stage 6 - wait for cam switch 2 to go low - first part of return transfer cycle is over
 //Stage 7 - wait for cam switch 2 to go high - second part of return transfer cycle is over
 //forceReturn - if set to 1, we ignore waiting for record end 
  if (isMagazineActiveFlag == 0) { //safety check. Do not allow transfer if the magazine is moving
    if (transferRecordStage == 4) {
      //debugSerial("TransferRecord Stage 4 - waiting for end of record.");
      if (forceReturn == 0) {
        cs2 = digitalRead(CAM2_SWITCH);
      } else {
        debugSerial("Force record return activated.");
        cs2 = 0;
      }  
      //Serial.println("CS2: " + String(cs2));
      if (cs2 == 0) { //end of record detected
        //start a new timer
        transferRecordNowMillis = millis();
        transferRecordPrevMillis = millis();
        debugSerial("Returning record. Transfer motor on.");
        digitalWrite(TRANSFER_MOTOR, LOW); 
        transferRecordStage = 5;
      }
    } else if (transferRecordStage == 5) {
        //debugSerial("TransferRecord Stage 5 - wait for 3000ms");
        transferRecordNowMillis = millis();
        if ((uint16_t) (transferRecordNowMillis - transferRecordPrevMillis) >= 3000) { 
          transferRecordPrevMillis = transferRecordNowMillis;
          transferRecordStage = 6;
        }
    } else if (transferRecordStage == 6) {
      //debugSerial("TransferRecord Stage 6 - wait for cs2 to go LOW");
      cs2 = digitalRead(CAM2_SWITCH);
      //Serial.println("CS2: " + String(cs2));
      if (cs2 == 0) { //end of transfer cycle
      transferRecordStage = 7;
      }
    } else if (transferRecordStage == 7) {
      //debugSerial("TransferRecord Stage 7 - wait for cs2 to go high. Transfer motor off.");
      cs2 = digitalRead(CAM2_SWITCH);
      //Serial.println("CS2: " + String(cs2));
      if (cs2 == 1) { //end of transfer cycle
        debugSerial("Record return complete.");
        digitalWrite(TRANSFER_MOTOR, HIGH); 
        transferRecordStage = 0;
        isRecordPlaying == 0;
      }
    } //endif check transfer stage
  } //magazine moving safety check  
} //end function return_record




void savePreferencesToEEPROM() {
  debugSerial("Writing data to EEPROM");
  writeIntIntoEEPROM(0,currentVolume);
  writeIntIntoEEPROM(2,currentFolderNumber);
  writeIntIntoEEPROM(4,lastSongPlayed);
  writeIntIntoEEPROM(6,playMode);
  #ifdef RGBLIGHTS
    writeIntIntoEEPROM(8,currentLightsPattern); //our pattern number; //600 - off //601 - orange, etc
    EEPROM.update(10, customColor1R);
    EEPROM.update(11, customColor1G);
    EEPROM.update(12, customColor1B);
    EEPROM.update(13, customColor2R);
    EEPROM.update(14, customColor2G);
    EEPROM.update(15, customColor2B);
    EEPROM.update(16, customColor3R);
    EEPROM.update(17, customColor3G);
    EEPROM.update(18, customColor3B);
    EEPROM.update(19, customBrightnessR);
    EEPROM.update(20, customBrightnessG);
    EEPROM.update(21, customBrightnessB);
    writeIntIntoEEPROM(22,customColorTimer);
  #endif
} //end function savePreferencesToEEPROM



//don't allow scanning if a record is playing
void scanManual() { //command 995
int exitFlag = 0;

if (isRecordPlaying == 0 && transferRecordStage ==0 ) {
  
  debugSerial("Manual Scan - Press 1 to Scan. 0 to exit.");
  isMagazineActiveFlag = 1; //consider the magazine active until we exit this routine
  ledDisplayString("   1 Scan  0 Done");
  delay(200);
  ledUpdateDigitsArray("scn");

  while (exitFlag == 0) {
    Key0.handle();
    Key1.handle();
    if (Key1.isPressed()) {
      digitalWrite(DETENT_COIL, LOW); //on
      delay(100);
      digitalWrite(MAGAZINE_MOTOR, LOW); //on
    }
    while (Key1.isPressed()) {   
      Key1.handle();
    }
    digitalWrite(DETENT_COIL, HIGH); //off
    delay(50);
    digitalWrite(MAGAZINE_MOTOR, HIGH); //off
    if (Key0.resetClicked()) {
      exitFlag = 1;
    }
  } //end while
  isMagazineActiveFlag = 0;
  }
} //end function scanManual




void scanForRecordAsync() {
//stage 1 - we'll pre-scan asynchronously for recordnumber - 1
//once we hit record - 1, we will stay in this routine and hit stage 2
//stage 2 - we'll scan for the record number, only 1 more position to move, but stay inside of this routine. too timing sensitive
int encoderValue = 0;
int recordNumberHex = 0;

//scanningStage is set to 1 in playSong to tell us that we need to scan. If scanningStage is 0, just exit the function - we aren't supposed to be scanning
if (scanningStage > 0) { 
  //precheck -- we've been asked to scan, let's check to see if we are already at the correct position
  encoderValue = readEncoderDouble();
  //debugSerial("Initial encoder value: " + String(encoderValue,HEX));
  if (encoderValue == recordNumberOrigHex) { //we are at the correct place, make sure the motor is off
    if (isMagazineActiveFlag == 1) {
      isMagazineActiveFlag = 0;
      digitalWrite(DETENT_COIL, HIGH); //off
      delay(75);
      digitalWrite(MAGAZINE_MOTOR, HIGH); //off
    }
    scanningStage = 0; //no need to scan further
    transferRecordStage = 1; //set up to transfer a record
    debugSerial("No need for scan. Carousel position: " + String(encoderValue, HEX));
  }


  //which record will we scan for: record number or record number -1 for pre-scan
  if (scanningStage == 1) { //we need to pre-scan
      recordNumberHex = recordNumberAdjHex;
    } else if (scanningStage == 2) {
      recordNumberHex = recordNumberOrigHex;
    }

  if (scanningStage == 1) {
    if (encoderValue != recordNumberHex) {  //are we at the correct magazine postion ?
      if (isMagazineActiveFlag == 0) { //turn on detent coil and magazine motor if needed
        isMagazineActiveFlag = 1;
        digitalWrite(DETENT_COIL, LOW);
        digitalWrite(MAGAZINE_MOTOR, LOW); 
      } //magazine active check  
    } else { //we found the proper record
      isMagazineActiveFlag = 0;
      digitalWrite(DETENT_COIL, HIGH); //off  
      delay(75);
      digitalWrite(MAGAZINE_MOTOR, HIGH); //off
      debugSerial("Pre-scan for record complete. Carousel position: " + String(encoderValue, HEX));
      scanningStage = 2; //set up for stage 2 of scan
    }
  } //end scanning stage 1  
 
  if (scanningStage == 2) {
    if (encoderValue != recordNumberHex) {  //are we at the correct magazine postion ?
      if (isMagazineActiveFlag == 0) { //turn on detent coil and magazine motor if needed
        isMagazineActiveFlag = 1;
        digitalWrite(DETENT_COIL, LOW);
        digitalWrite(MAGAZINE_MOTOR, LOW); 
      }
      encoderValue = readEncoderDouble();
      //debugSerial("Encoder value: " + String(encoderValue,HEX));    
      while (encoderValue != recordNumberHex) {
        encoderValue = readEncoderDouble();
        //debugSerial("Encoder value: " + String(encoderValue,HEX));    
      }
      //if we're here, we found the right record
      digitalWrite(DETENT_COIL, HIGH); //off  
      //delay(100);
      digitalWrite(MAGAZINE_MOTOR, HIGH); //off
      delay(50);
      isMagazineActiveFlag = 0;
      encoderValue = readEncoderDouble();
      debugSerial("Scan for record complete. Final magazine position: " + String(encoderValue, HEX));
      scanningStage = 0; //scan complete
      transferRecordStage = 1; //set to 1 to initiate transfer of record from the magazine to the turntable
    } //stage 2 - found correct position?
  } //end scanning stage 2

  //watchdog timer check. A full revolution should take around 16 seconds. If we are trying to scan for more than 20 seconds, something is wrong.
   scanRecordWatchdogNowMillis = millis();
   if ((uint16_t) (scanRecordWatchdogNowMillis - scanRecordWatchdogPrevMillis) >= 20000) { 
      scanRecordWatchdogPrevMillis = scanRecordWatchdogNowMillis;
      debugSerial("Scan watchdog triggered - Scan taking too long.");
      digitalWrite(DETENT_COIL, HIGH); //off  
      digitalWrite(MAGAZINE_MOTOR, HIGH); //off
      isMagazineActiveFlag = 0;
      scanningStage = 0; //scan complete
      ledUpdateDigitsArray("Err");
      delay(2000);
    } //scan timer triggered

} //scanning stage > 0
} //end function scanForRecordAsync




//stop playing the current song
void stopPlaying() {  
  if (isMp3Player==1) {
    debugSerial("Mp3 stop playing command issued");
    addCommandToQueue(stopcmd, sizeof(stopcmd) /sizeof(stopcmd[0]) ); //proper way to send the size of the command, normally hardcoding the value of 8 in most places
  } else { //physical phono
      debugSerial("Phono stop playing command issued");
      if (transferRecordStage == 0 || transferRecordStage == 4) {
        transferRecordStage = 4; //force stage 4, which is where we need to be to initiate the record return process
        returnRecordAsync(1); //1 - force return - we don't wait for cs2 to activate transfer
      } else {
        debugSerial("Transfer already in place. Try the command again shortly.");
      }
  }
} //end stop_playing




void transferRecordAsync() {
  //Stage 1 - setup Timer. Turn on transfer motor.
  //Stage 2 - wait for 2000ms
  //Stage 3 - wait for cam switch 2 to go high - first part of transfer cycle is over
  if (isMagazineActiveFlag == 0) { //safety check. Do not start any transfer if the magazine is moving
    if (transferRecordStage == 1) {
      //start a new timer
      transferRecordNowMillis = millis();
      transferRecordPrevMillis = millis();
      transferRecordStage = 2;
      debugSerial("TransferRecord Stage 1 - setting up timer and turning on transfer motor");
      //turn on transfer motor for 2000 ms. this should activate the cs2 switch which will stay LOW (0) until the record is on the turntable
      debugSerial("Turning transfer motor on.");
      digitalWrite(TRANSFER_MOTOR, LOW); 
    } else if (transferRecordStage == 2) {
      //debugSerial("TransferRecord Stage 2 - wait for 2000ms");
        transferRecordNowMillis = millis();
        if ((uint16_t) (transferRecordNowMillis - transferRecordPrevMillis) >= 2000) { 
          transferRecordPrevMillis = transferRecordNowMillis;
          transferRecordStage = 3;
        }
    } else if (transferRecordStage == 3) {
        //debugSerial("TransferRecord Stage 3 - wait for cs2 to go HIGH");
        cs2 = digitalRead(CAM2_SWITCH);
        if (activateToggleShiftCoil == 1) {
          digitalWrite(TOGGLESHIFT_COIL,LOW);
        }      
        if (cs2 == 1) { //record has finished transfer
          activateToggleShiftCoil = 0;
          digitalWrite(TOGGLESHIFT_COIL, HIGH);   //make sure toggle shift coil is off
          digitalWrite(TRANSFER_MOTOR, HIGH); //turn off transfer motor
          debugSerial("Record on turntable. Turning transfer motor off.");
          transferRecordStage = 4; //part of returnRecord
        } //end cs2 == 1
     } //end if transferRecordStage  
  } //magazine active check
} //end function transferRecordAsync


//https://roboticsbackend.com/arduino-store-int-into-eeprom/  
void writeIntIntoEEPROM(int address, int number)
{ 
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.update(address, byte1);
  EEPROM.update(address + 1, byte2);
}
int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}


//eeprom memory map - 4k to play with
//0-1, currentVolume
//2-3, currentFolderNumber
//4-5, lastSongPlayed
//6-7, playMode
//8-9, current pattern - 1 byte
//10 - custom data for lights pattern. 14 bytes each pattern
//10 color 1 - R 
//11 color 1 - G 
//12 color 1 - B 
//13 color 2 - R 
//14 color 2 - G 
//15 color 2 - B 
//16 color 3 - R 
//17 color 3 - G 
//18 color 3 - B 
//19 brightness- R
//20 brightness - G
//21 brightness - B
//22-23 (int) 2 bytes - color timer - 0 - 32767
  

