//Robert DiNapoli 2024
//rjgee@hotmail.com
#define VERSION 113
#define RGBLIGHTS  //comment out this line if you aren't going to use RGB lights in your front panel

// 000 - stop current song. next song in queue should play. In record player mode, will return the record from the turntable to the magazine.
// 100 to 299 - queue a song to play 
// 600 to 699 - lights commands - see below
// 700 to 730 - set the volume - mp3 player only
// 750 - back to normal play mode, mode 0 - non continuous 
// 751 - continuous play, mode 1 - sequential play - current folder. must be free play, or have credits inserted. available on phono, but not for startup.
// 752 - continuous play, mode 2 - random play - current folder. must be free play, or have credits inserted. not available on phono
// 753 - continuous play, mode 3 - random play - song from any mp3 folder. must be free play, or have credits inserted. not available on phono
// 801 to 898 - if on mp3player, switch folder / directory ( 1 - 98)
// 995 - manual magazine scan - phono only
// 996 - encoder alignment test - phono only
// 997 - display preferences: version, folder, volume, play mode: 0 normal(nor), 1 sequential(seq), 2 random (ran), 3 randomall (ral)
// 998 - save preferences to the eeprom
// 999 - reset arduino

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
//653 - glow - red, custom color with custom timer
//654 - glow - random color - 7 second timer

//690 - color test - enter 000 000 000 to exit
//697 - input custom brightness values
//698 - input customer timer value
//699 - create custom pattern - 3 colors, timer and brightness
//jukebox selection - 100 - 299
//in the mp3 player, there is no position 0. entering 100 on the keypad stores song number 1 in the queue
//for the phono player, magazine starts at postion 0 and go to 99. (uses the same queue for mp3 player, so need to adjust song number and subtract.)

//lights: make any selection, reset and reselect, your selection, record playing. we only handle make any selection or record playing
//interrupt 0(2), interrupt 1(3), interrupt 2(21), interrupt 3(20), interrupt 4(19), interrupt 5(18)
//interrupt 5 - pin 18 - coin insert


#include <avr/wdt.h> //for watchdog timer - arduino reset
#include <SoftwareSerial.h>
#include <AbleButtons.h> //John Scott
#include <QList.h> //Martin Dagarin
#include <Arduino.h>
#include <AceTMI.h> //Brian T Park
#include <AceSegment.h> // Tm1637Module //Brian T Park
#include <EEPROM.h> 
//#define THROW_ERROR_IF_NOT_FAST // only for compile test... Activate this to detect where ...Fast() functions are called with NON constant parameters and are therefore still slow.
#include <digitalWriteFast.h> //Watterrott
#include <AlmostRandom.h> //rubbish52
#ifdef RGBLIGHTS
//this code uses the ALA library by bportaluri to make rgb light effects. Great concepts and code - but some pointer bugs that make the code non functional
//in the aspect we need to use it.. It's been over 20 years since I've touched C/C++ and my memory mangagement / pointer skills are pretty rusty. I think I've patched
//everything enough to have it working properly, and also ripped out the code we don't need. Copy the ALA-FIXED directory to the arduino libraries folder.
//restart the arduino ide. you can then go sketch -> include library and select the ala-fixed library. 
  #include <Ala.h> //bportaluri
  #include "AlaLedRgb.h"
  AlaLedRgb leds;
#endif


//library for led digit display
using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;

SoftwareSerial softSerial(12,13); //rx, tx

//setup song queue - as songs are entered, we throw them into this queue
struct songinfo
{
    int folder;
    int track;
};
typedef struct songinfo SongInfo;

//commands to the mp3 player need a delay between them, sometimes up to 1500ms for the mp3 player busy to register after playing a song
//we will enter all mp3 player commands into a queue, and then pick them off one at a time after an appropriate delay
struct commandinfo
{
    uint8_t *cmd;
    int size;
};
typedef struct commandinfo CommandInfo;


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
#define ENCODER1 23
#define KEY_9 24
#define ENCODER2 25
#define KEY_8 26
#define ENCODER4 27
#define KEY_7 28
#define ENCODER8 29
#define KEY_6 30
#define ENCODER16 31
#define KEY_5 32
#define ENCODER32 33
#define KEY_4 34
#define ENCODER64 35
#define KEY_3 36
#define ENCODER128 37
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
#define PLAYCOUNTER A14


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
int r81queuing = 0; // set to 0 to play next song in the queue first. set to 1 to emulate r81 queuing mode - closest record in the queue plays first. - only active in phono mode
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
QList<SongInfo> songList; //queue of songs
QList<CommandInfo> commandList; //queue of commands 
//timers
uint16_t blinkNowMillis = millis();
uint16_t blinkPrevMillis = millis();
uint16_t commandNowMillis = millis();
uint16_t commandPrevMillis = millis();
uint16_t playCommandNowMillis = millis();
uint16_t playCommandPrevMillis = millis();
uint16_t creditNowMillis = millis();
uint16_t creditPrevMillis = millis();
uint8_t  nextcmd[] = {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF};
uint8_t stopcmd[] = {0x7E, 0xFF, 0x06, 0x0e, 0x00, 0x00, 0x00, 0xEF};
uint8_t playcmd[] = {0x7E, 0xFF, 0x06, 0x0f, 0x00, 0x02, 0x01, 0xEF};
uint8_t eqcmd[] = {0x7E, 0xFF, 0x06, 0x07, 0x00, 0x02, 0x01, 0xEF}; //built in equalizer // I don't notice too much difference using these, so didn't code for it.
uint8_t volcmd[] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 0x1e, 0xEF}; //max volume of 30
/*a query with 0x48 will show you the number of songs on the whole flash card - in all folders. so if have 2 songs in folder 01 and 4 songs in
/folder 2, the 48 query will return 6 songs. Using the 0x03 command, you can play the absolute song number - no matter which folder the song is in*/
uint8_t querycmd[] = {0x7E, 0xFF, 0x06, 0x48, 0x00, 0x00, 0x00, 0xEF}; 
uint8_t playabsolutesongnumbercmd[] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0xca, 0xEF};
//record player stuff
boolean cs2; //cam switch 2
int isRecordPlaying = 0; //0 - no record playing  1 - record playing
int scanningStage = 0; //0 - no scanning   1 - pre scan    2 - scan
int recordNumberOrigHex = 0; //the record number to play 0 - 99 - does not get modified
int recordNumberAdjHex = 0; //the record number to play 0 - 99 - will be adjusted for pre scan
int activateToggleShiftCoil = 0; // 1 - activate toggle shift and play opposite side of the record
int isMagazineActiveFlag = 0; //0 - magazine off. 1 - detent coil is engaged and magazine turning
int transferRecordStage = 0;  //0 - no action. 1 - we need to transfer a record to the turntable  2 - record on turntable and will need to be returned
uint16_t transferRecordNowMillis = millis(); //used to set a 2 second timer when starting transfer
uint16_t transferRecordPrevMillis = millis();
uint16_t scanRecordWatchdogNowMillis = millis(); //more than 20 seconds on a scan - flag an error
uint16_t scanRecordWatchdogPrevMillis = millis();
uint16_t transferRecordWatchdogNowMillis = millis(); //more than 5 seconds on a transfer - flag an error
uint16_t transferRecordWatchdogPrevMillis = millis();
//randomness
AlmostRandom ar;
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
  int currentLightsPattern = 601; //default lights off
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
digitalWriteFast(TOGGLESHIFT_COIL, HIGH); //setup pin so that LOW triggers on - relay 4
pinMode(TOGGLESHIFT_COIL, OUTPUT);  //47
digitalWriteFast(MAGAZINE_MOTOR, HIGH); //setup pin so that LOW triggers on - relay 3
pinMode(MAGAZINE_MOTOR, OUTPUT);  //49
digitalWriteFast(DETENT_COIL, HIGH); //setup pin so that LOW triggers on - relay 2
pinMode(DETENT_COIL, OUTPUT);  //51
digitalWriteFast(TRANSFER_MOTOR, HIGH); //setup pin so that LOW triggers on - relay 1
pinMode(TRANSFER_MOTOR, OUTPUT);  //53
digitalWriteFast(PLAYCOUNTER, HIGH); //setup pin so that LOW triggers on - relay 5
pinMode(PLAYCOUNTER, OUTPUT); //a 14
attachInterrupt(digitalPinToInterrupt(SWITCH_CREDITS), addCreditFlag, FALLING); //set up an interrupt to capture credit insert


//led display digits setup
tmiInterface.begin();
ledModule.begin();
ledModule.setBrightness(7); //full brightness

//debugging serial
Serial.begin(38400);
while (!Serial) {
  delay(10);
}
//mp3 serial
softSerial.begin(9600);

debugSerial("Initializing");
ledDisplayDigits("rst");
delay(1000);

Button::setDebounceTime(30);
digitalWriteFast(LED_RECORDPLAYING_LIGHT, LOW);
digitalWriteFast(LED_MAKEANYSELECTION_LIGHT, LOW);


//check pcb switches
isFreePlay = digitalReadFast(SWITCH_FREEPLAY); 
isMp3Player = digitalReadFast(SWITCH_MP3PLAYER);
debugSerial("isFreePlay: " + String(isFreePlay));
debugSerial("isMp3Mplayer: " + String(isMp3Player));
if (isMp3Player == 1) {
  ledDisplayDigits("mp3");
} else {
  ledDisplayDigits("pho");
  delay(1000);
  //display encoder position on startup
  int encoderPosition = encoderRead();
  ledDisplayDigits(String(encoderRead(), HEX));
}
delay(1500);

//seed random
long randomL = ar.getRandomLong();
randomSeed(randomL); 
debugSerial("Random seed: " + String(randomL));
//debugSerial("Random methods: " + String(ar.getLastRunCode())); //quick test shows the arduino mega is using all 4 randomization methods

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

    

//the mp3 player kind of does its own thing. make it stop playing any songs after a system reset.
//yes I want to issue this command even if currently in phono mode.
addCommandToQueue(stopcmd, sizeof(stopcmd) /sizeof(stopcmd[0]) ) ; 
if (isMp3Player == 1) { //if are on mp3 player, get the total songs available. //used for playmode 3
  getTotalSongsAvailable();
  debugSerial("Total Songs on Mp3 Player: " + String(totalSongsAvailable));
}

readPreferencesFromEEPROM();  //pull volume, play mode, folder number, etc from memory
//if we using phono, reset the play mode to zero - do not auto start playing records from sequential mode or random mode.
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
  checkForCreditFlag(); //ISR will flag that the cr----------edit switch was triggered. Handle it here.
  if (isMp3Player == 0) { //phono mode
    recordScanAsync(); //check to see if we need to scan for a new record. triggered by setting scanningState to 1 in playSong()
    recordTransferAsync(); //check to see if we need to transfer a record from the magazine. triggered by setting transferRecordStage to 1 in scanForRecordAsync()
    recordReturnAsync(0); //check to see if we need to return record to magazine. triggered by transferRecordStage >=4. set from transferRecordAsync
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
  commandList.push_back(ci);
  //debugSerial("Adding command to queue: " + String(cmd[0]) + " - items in queue: " + String(commandList.itemCount())); 
} //end function addCommandToQueue



//interrupt 5 pin 18 - I never want to lose a coin insert. however, you can't debounce in an ISR, so set a flag, and do the debounce elsewhere
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
  addSongToQueue(currentFolderNumber, random(1,200));
} //end function addRandomSongFromCurrentFolderToQueue;


//for continuous play mode 3, choose a random song from anywhere on the mp3 player
void addRandomSongFromAnyFolderToQueue() {
  addSongToQueue(currentFolderNumber, random(1,totalSongsAvailable));
} //end function addRandomSongFromAnyFolderToQueue;



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
  songList.push_back(si);
  debugSerial("Folder: " + String(folder) + "  Track: " + String(track) + " added to song queue" +  " - items in queue: " + String(songList.size())); 
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
        debugSerial("Credit added. Credits: " + String(credits)); 
      } //end if timer says we can trigger credit
    } //end if credit flag set
} //end function checkForCreditFlag




//check for a song playing. If no song is playing, and there is a song in queue,
//grab the next item from the song queue and play it
void checkForNextSong() {
  if (isMp3Player == 1) {
    //we need to wait at least 3.5 seconds since the play command to make sure the mp3 player has gone "busy"
    //might be my imagination, but this seems to take longer with more songs on the sdcard...
    playCommandNowMillis = millis();
    if ((uint16_t) (playCommandNowMillis - playCommandPrevMillis) >= 3500) { 
      playCommandPrevMillis = playCommandNowMillis;
      isMP3Playing = digitalReadFast(MP3PLAYER_BUSY); //1 - idle //0 - song playing
      if (isMP3Playing == 1) { //if the mp3 player is idle
        if (recordPlayingLight == 1) {
          debugSerial("Record playing light turned off");
          recordPlayingLight = 0;
          digitalWriteFast(LED_RECORDPLAYING_LIGHT, LOW);
        }
        //if the song list is not empty, grab the next song in queue, and send the play command. otherwise, clear the led display
        if (songList.size() > 0) {
          processSongFromQueue();
        } else {
          //we are idle, and the song list is empty
          currentSelectionString = "";
          if (playMode ==1 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if sequential play is on, and we are on free play or have credits left, add the next incremental song to the queue
            addSequentialSongToQueue();
          } else if (playMode ==2 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if random play is on, and we are on free play or have credits left, add a random song to the queue
            addRandomSongFromCurrentFolderToQueue(); 
          } else if (playMode ==3 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if random play for all folders is on, and we are on free play or have credits left, add a random song to the queue
            addRandomSongFromAnyFolderToQueue(); 
          }  
        } //no songs in queue
      } // no song playing
    } //timer not met
  } else { //phono stuff here

  //we need to wait at least 1.5 seconds since the play command to make sure the record player has gone busy
    playCommandNowMillis = millis();
    if ((uint16_t) (playCommandNowMillis - playCommandPrevMillis) >= 1500) { 
      playCommandPrevMillis = playCommandNowMillis;
      if (isRecordPlaying == 0 && transferRecordStage == 0 && scanningStage == 0) { //phono is idle
        if (recordPlayingLight == 1) {
          debugSerial("Record playing light turned off");
          recordPlayingLight = 0;
          digitalWriteFast(LED_RECORDPLAYING_LIGHT, LOW);
        }
        //if the song list is not empty, grab the next song in queue, and send the play command. otherwise, clear the led display
        if (songList.size() > 0) {
          processSongFromQueue();
        } else {
          //we are idle, and the song list is empty
          currentSelectionString = "";
          if (playMode ==1 && (isFreePlay ==1 || (isFreePlay == 0 && credits > 0)) ) {
            //if sequential play is on, and we are on free play or have credits left, add the next incremental song to the queue
            addSequentialSongToQueue();
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
      digitalWriteFast(LED_MAKEANYSELECTION_LIGHT,HIGH);
      makeAnySelectionLight = 1;
    }
  } else { //turn it off if no credits or not in free play mode 
      if (makeAnySelectionLight == 1) {
        digitalWriteFast(LED_MAKEANYSELECTION_LIGHT,LOW);
        makeAnySelectionLight = 0;
      }  
  } //handle the display light depending on free play mode or credits
} //end function displayMakeAnySelectionLight



//display volume, current folder, current play mode, version
void displayPreferences() {
  debugSerial("Values:  volume: " + String(currentVolume) + "  currentFolder: " + String(currentFolderNumber) + "  lastSongPlayed: " + String(lastSongPlayed+99) + "  playMode: " + String(playMode));
  ledDisplayDigits("Ver");
  delay(1000);
  ledDisplayDigits(String(VERSION));
  delay(1000);
  ledDisplayDigits("Fol");
  delay(1000);
  ledDisplayDigits(String(currentFolderNumber));
  delay(1000);
  ledDisplayDigits("Sng");
  delay(1000);
  ledDisplayDigits(String(lastSongPlayed+99));
  delay(1000);
    ledDisplayDigits("vol");
  delay(1000);
  ledDisplayDigits(String(currentVolume));
  delay(1000);
  ledDisplayDigits("mod");
  delay(1000);
  if (playMode == 0) {
    ledDisplayDigits("normal");
  } else if (playMode == 1) {
    ledDisplayDigits("seq");
  } else if (playMode == 2) {
    ledDisplayDigits("ran");
  } else if (playMode == 3) {
    ledDisplayDigits("ral");
  }
  delay(1000);
   
} //end function displayPreferences




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


//display an error code wait for a keypress for acknowledgement
void ledDisplayError(int errorNumber) {
  int exitFlag = 0;
  songList.clear(); //clear the song list. we don't want the unit trying to scan or transfer the next record if the user hasn't cleared the current problem
  if (errorNumber == 1) {
    debugSerial("Error: Scan took more than 20 seconds. Press any key to acknowledge.");
    ledDisplayDigits("scn");
    scanRecordWatchdogNowMillis = millis;
    scanRecordWatchdogPrevMillis = millis;
  } else if (errorNumber == 2) {
    debugSerial("Error: Transfer took more than 9 seconds. Press any key to acknowledge.");
    ledDisplayDigits("trf");
    transferRecordWatchdogNowMillis = millis;
    transferRecordWatchdogPrevMillis = millis;
  } else if (errorNumber == 3) {
    debugSerial("Error: Record return took more than 9 seconds. Press any key to acknowledge.");
    ledDisplayDigits("rtn");
    transferRecordWatchdogNowMillis = millis;
    transferRecordWatchdogPrevMillis = millis;
  } else if (errorNumber == 4) {
    debugSerial("Error: Scan Stage 2 took more than 1 second. Press any key to acknowledge.");
    ledDisplayDigits("sc2");
    transferRecordWatchdogNowMillis = millis;
    transferRecordWatchdogPrevMillis = millis;
  }

  while (exitFlag == 0) {
    getDigitsInput();
    if (inputSelectionString > "") {
      debugSerial("Key Pressed. Error acknowledged.");
      exitFlag = 1;
      inputSelectionString = "";
  } //if - check for keypress
} //end while
} //end function errorDisplay





//given a string containing 0 -3 digits, fill the ledDigits array with the numbers 1 - 9 for each led position. Or number 10 for a blank digit
void ledDisplayDigits(String s) {
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

  //get the pattern for each digit in the ledDigits array, and put it on the physical led module for each position
  //debugSerial("digit1 output: " + String(ledDigits[2]) + " digit2 output :" + String(ledDigits[1]) + " digit3 output: " + String(ledDigits[0]));
  // Update the display
  uint8_t pattern = PATTERNS2[ledDigits[0]]; 
  ledModule.setPatternAt(2, pattern);
  uint8_t pattern2 = PATTERNS2[ledDigits[1]]; 
  ledModule.setPatternAt(1, pattern2);
  uint8_t pattern3 = PATTERNS2[ledDigits[2]]; 
  ledModule.setPatternAt(0, pattern3);
  ledModule.flush();
} //end ledDisplayDigits




void ledDisplayString(String displayString) {
//char cr[] = ":::dinapoli:2024:::";
char cr[displayString.length()+1];
displayString.toCharArray(cr, displayString.length() + 1);

  for (int i=0; i< displayString.length(); i++) {
    String x = String(cr).substring(i,i+3);
    ledDisplayDigits(x);
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
      ledDisplayDigits(inputSelectionString);
    } else {
      // Otherwise, show the current playing song (will be blank if no song playing)
      ledDisplayDigits(currentSelectionString);
    } //endif input in progress
   } else {
     //handle the blinking of an entered command
     if (blinkCounter < blinkAmount) {
        blinkNowMillis = millis();
        if ((uint16_t) (blinkNowMillis - blinkPrevMillis) > (blinkDelay*2)) { 
          blinkPrevMillis = blinkNowMillis; 
          blinkCounter = blinkCounter + 1;
        } else if ((uint16_t) (blinkNowMillis - blinkPrevMillis) >= blinkDelay) { //blink off
          ledDisplayDigits(""); //update the leddigits array, given a string
        } else if ((uint16_t) (blinkNowMillis - blinkPrevMillis) >= 0) { //blink on
          ledDisplayDigits(blinkString);
        }  
      } else { //blink counter > blink amount, reset it
        blinkCounter = 0;
        blinkString = "";
      } //end counter 
   } //end if blinkString > ""
} //end function ledUpdateDisplay




//used for help translating the encoder position. there is probably a much better way to do this
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
//in phono mode: entering 100 (#1 stored in song queue) should play record in magazine position 0. 101 (stored as #2 in playlist) should play record in position 1)
//phono mode: 199 should play record in carousel position 99
void playSong(uint8_t cmd[], int size) {
   debugSerial("Record playing light turned on");
   digitalWriteFast(LED_RECORDPLAYING_LIGHT, HIGH);
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
    playCounter(); //trigger the play counter to trigger
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
  if (commandList.size() > 0) {
    //only execute a command once every 2 seconds
    commandNowMillis = millis();
    if ((uint16_t) (commandNowMillis - commandPrevMillis) >= 2000) { 
      commandPrevMillis = commandNowMillis;
      ci = commandList.front();
      commandList.pop_front();
      debugSerial("Command pulled from queue - cmd: " + String(*ci.cmd) + "  items in queue: " + String(commandList.size())); 
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
  } else if (inputSelectionNumber == 753) {
     playMode = 3;
     debugSerial("Continuous random play for all folders turned on");
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
    recordScanManual();
  } else if (inputSelectionNumber == 996 ) { //encoder alignment
     ledBlinkSetup(inputSelectionString,4,150);
     encoderAlignment(); 
  } else if (inputSelectionNumber == 997 ) { //display eeprom settings
     ledBlinkSetup(inputSelectionString,4,150);
     displayPreferences();
  } else if (inputSelectionNumber == 998 ) { //save eeprom settings
     ledBlinkSetup(inputSelectionString,4,150);
     savePreferencesToEEPROM();
  } else if (inputSelectionNumber == 999 ) { //reset
     ledBlinkSetup(inputSelectionString,4,150);
     resetArduino();
  }
  inputInProgress = 0 ; //the display can go back to displaying the song in progress, if one is playing
  inputSelectionString = ""; //we've processed the input string - clear it out
} //end function processInput



//if a song exists in the song queue, grab it, and send it to be played
void processSongFromQueue() {
  
if (isMp3Player == 0 && r81queuing == 1) { //emulate R81 queuing mode for phonograph mode
  int encoderPosition = numberHexToDecimal(encoderRead());
  int queuePosition = 0;
  int currentTrack = 0;
  //debugSerial("Encoder position: " + String(encoderPosition));
  int distance = 300;
  for (int x = 0; x<songList.size(); x++) {
    //1 - 200 - song # stored in queue. 
    if (songList.get(x).track > 100) {
      currentTrack = songList.get(x).track - 101; //b side - 101 - 200. then subtract one because first magazine position is zero.
    } else {
      currentTrack = songList.get(x).track - 1; //first song number is 1. but first magazine position is zero.
    }  
    //debugSerial("track in queue: " + String(currentTrack));
    if (currentTrack - encoderPosition < 0) {
      currentTrack = currentTrack + 100; //make the distance much further away
    }
    if (currentTrack - encoderPosition < distance) {
      distance = currentTrack - encoderPosition;
      //debugSerial("new next track to play: " + String(currentTrack));
      queuePosition = x;
    }
  } //for loop
    si = songList.get(queuePosition);
    songList.clear(queuePosition);
    debugSerial("R81 queue emulation mode: next song to play: " + String(si.track));
  } else {
    si = songList.front();
    songList.pop_front();
  }  
  debugSerial("Song pulled from queue - track: " + String(si.track) + "  folder: " + String(si.folder) + "  items in queue: " + String(songList.size())); 
  char buffer[5];
  
  playcmd[5] = si.folder;
  playcmd[6] = si.track;
  lastSongPlayed = si.track;
  if (playMode ==0 || playMode ==1 || playMode ==2) { //normal play modes use folder and song #
    sprintf(buffer, "%03d", si.track + 99); //3 digits, left filled zeros
    currentSelectionString = String(buffer);
    if (inputInProgress ==0 ) { //if the user is not entering any data, immediately update the display with the current song number
      ledDisplayDigits(currentSelectionString);
    }
    playcmd[5] = si.folder;
    playcmd[6] = si.track;
    lastSongPlayed = si.track;
    playSong(playcmd,8); 
  } else if (playMode == 3) { //special play mode, no folder. song number uses 2 positions instead of folder
    sprintf(buffer, "%03d", si.track); //3 digits, left filled zeros
    currentSelectionString = String(buffer);
    if (inputInProgress ==0 ) { //we'll display a number here, but it won't really mean anything.
      //the best I can tell is that this is the order that files were written to on the sd card. So if you write a file (let's say the first file)
      //and then renumber it to 070 - Filename, it's now the 70th file on the sd card, numerically. But it was still written first.
      //so playing song 070 in normal mode will play the song expected. But in absolute play mode, it's still song #1.
      //anyway, that's my best guess.
      ledDisplayDigits(currentSelectionString);
    }
    playabsolutesongnumbercmd[5] = si.track >> 8; //absolute song number on the mp3 player. 1 - xxxxx. 
    playabsolutesongnumbercmd[6] = si.track & 0xff; 
    playSong(playabsolutesongnumbercmd,8); 
  }
} //end function processSongFromQueue



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
    currentFolderNumber = 1;
    playMode = 0;
    lastSongPlayed = 1;
    #ifdef RGBLIGHTS
      currentLightsPattern = 601;
      inputSelectionString = String(currentLightsPattern);
      lightsSelector();
    #endif  
    addCommandToQueue(volcmd, 8);
  }  
} //end function readPreferencesFromEEPROM



//I added this mainly so I could switch between phono and mp3 mode since the switches are only read on startup.
void resetArduino() {
ledDisplayString(":::::Reset:::1:Yes:::0:No:::");

inputSelectionString = "";
while (inputSelectionString == "") {
  getDigitsInput();
  if (inputSelectionString == "1") {
    debugSerial("Reset Requuested via command 999.");
    wdt_disable();
    wdt_enable(20); //20ms
    while (1) {}
  } 
} //end while
} //end function resetArduino




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




//stop playing the current song
void stopPlaying() {  
  if (isMp3Player==1) {
    debugSerial("Mp3 stop playing command issued");
    addCommandToQueue(stopcmd, sizeof(stopcmd) /sizeof(stopcmd[0]) ); //proper way to send the size of the command, normally hardcoding the value of 8 in most places
  } else { //physical phono
    debugSerial("Phono stop playing command issued");
    if (transferRecordStage == 0 || transferRecordStage == 4) {
      transferRecordStage = 4; //force stage 4, which is where we need to be to initiate the record return process
      transferRecordWatchdogNowMillis = millis(); //set up the timer to watch record transfer
      transferRecordWatchdogPrevMillis = millis();
      recordReturnAsync(1); //1 - force return - we don't wait for cs2 to activate transfer
    } else {
      debugSerial("Transfer already in place. Try the command again shortly.");
    }
  }
} //end stop_playing



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
  

