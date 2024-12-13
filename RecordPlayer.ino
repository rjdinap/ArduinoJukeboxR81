//functions for handling the record player and encoder are here


void encoderAlignment() { //command 996
int x = 0;
int exitFlag = 0;
int encoderValue = 0;

debugSerial("Encoder alignment. Press any number to exit.");
ledDisplayDigits("enc");
delay(400);

while (exitFlag == 0) {
//See which record is currently at the position in carousel
  encoderValue = encoderRead();
  //Serial.println(dataByte,HEX);
  ledDisplayDigits(String(encoderValue, HEX));
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




int encoderRead() {
int encoderValue;  
//byte encoderPinArray[] = {23,25,27,29,31,33,35,37}; // MSB is the rightmost
//const byte encoderPinArray[] = {E1, E2, E3, E4, E5, E6, E7, E8}; // MSB is the rightmost
int newBit; //used for reading encoder
int dataByte; //used for reading encoder
int x=0;

dataByte = 0;

//for (x=0; x<8; x=x+1){
//   newBit = digitalRead(encoderPinArray[x]); // read a pin - this doesn't work with digitalReadFast
//   dataByte = dataByte | (newBit<<x); //shift the bit into position and put in its place in the data byte
//}
   newBit = digitalReadFast(ENCODER1); // read a pin
   dataByte = dataByte | (newBit<<0); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER2); // read a pin
   dataByte = dataByte | (newBit<<1); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER4); // read a pin
   dataByte = dataByte | (newBit<<2); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER8); // read a pin
   dataByte = dataByte | (newBit<<3); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER16); // read a pin
   dataByte = dataByte | (newBit<<4); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER32); // read a pin
   dataByte = dataByte | (newBit<<5); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER64); // read a pin
   dataByte = dataByte | (newBit<<6); //shift the bit into position and put in its place in the data byte
   newBit = digitalReadFast(ENCODER128); // read a pin
   dataByte = dataByte | (newBit<<7); //shift the bit into position and put in its place in the data byte

encoderValue = dataByte;
return encoderValue;
} //end function readEncoder



int encoderReadDouble() {
//lots of bad data when reading the encoder, so we'll read it twice with a quick compare to weed out bad data
int finalValue;
int value1 = encoderRead();
//debugSerial("Initial value1: " + String(value1,HEX));
  delay(5);

  int value2 = encoderRead();
  //debugSerial("Initial value2: " + String(value2,HEX));
  if (value2 == value1) {
    finalValue = value1; //if we read it twice, and it's the same, exit this routine
  } else {
    finalValue = 999;
  }
return finalValue;
} //end function readEncoderDouble



//we bring a pin LOW to turn it on
//or table: 0 and 0 = 0, 0 and 1 = 1, 1 and 1 = 1
//with !    1 and 1 = 1, 1 and 0 = 1, 0 and 0 = 0
//if eithther pin is LOW, we will return true that a scan is active: 
int isScanActive() {
  return (!digitalReadFast(MAGAZINE_MOTOR) || !digitalReadFast(DETENT_COIL));
}


//if the transfer pin is low (ON), return true
int istransferActive() {
  return !(digitalReadFast(TRANSFER_MOTOR));
}




//trigger the play counter to increment
void playCounter() {
  digitalWriteFast(PLAYCOUNTER, LOW);
  delay(25);
  digitalWriteFast(PLAYCOUNTER, HIGH);
} //end fuction playCounter



void recordReturnAsync(int forceReturn) {
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
        cs2 = digitalReadFast(CAM2_SWITCH);
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
        digitalWriteFast(TRANSFER_MOTOR, LOW); 
        transferRecordStage = 5;
        transferRecordWatchdogNowMillis = millis();
        transferRecordWatchdogPrevMillis = millis();
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
      cs2 = digitalReadFast(CAM2_SWITCH);
      //Serial.println("CS2: " + String(cs2));
      if (cs2 == 0) { //end of transfer cycle
      transferRecordStage = 7;
      }
    } else if (transferRecordStage == 7) {
      //debugSerial("TransferRecord Stage 7 - wait for cs2 to go high. Transfer motor off.");
      cs2 = digitalReadFast(CAM2_SWITCH);
      //Serial.println("CS2: " + String(cs2));
      if (cs2 == 1) { //end of transfer cycle
        debugSerial("Record return complete.");
        digitalWriteFast(TRANSFER_MOTOR, HIGH); 
        transferRecordStage = 0;
        isRecordPlaying == 0;
      }
    } //endif check transfer stage

 if (transferRecordStage >=5 && transferRecordStage <=7 ) {  
    //watchdog timer check. A record transfer should take about 6 seconds. 
     transferRecordWatchdogNowMillis = millis();
     if ((uint16_t) (transferRecordWatchdogNowMillis - transferRecordWatchdogPrevMillis) >= 9000) { 
        transferRecordWatchdogPrevMillis = transferRecordWatchdogNowMillis;
        debugSerial("Record transfer watchdog triggered - more than 9 seconds to transfer.");
        activateToggleShiftCoil = 0;
        digitalWriteFast(TOGGLESHIFT_COIL, HIGH);   //make sure toggle shift coil is off
        digitalWriteFast(TRANSFER_MOTOR, HIGH); //turn off transfer motor
        transferRecordStage = 0;
        ledDisplayError(3); //scan error
      } //transfer watchdog timer triggered
  } //transferrecordstage > 4 and < 7  
  } //magazine moving safety check  
} //end function return_record




void recordScanAsync() {
//stage 1 - we'll pre-scan asynchronously for recordnumber - 1
//once we hit record - 1, we will stay in this routine and hit stage 2
//stage 2 - we'll scan for the record number, only 1 more position to move, but stay inside of this routine. too timing sensitive
int encoderValue = 0;
int recordNumberHex = 0;

//scanningStage is set to 1 in playSong to tell us that we need to scan. If scanningStage is 0, just exit the function - we aren't supposed to be scanning
if (scanningStage > 0) { 
  //precheck -- we've been asked to scan, let's check to see if we are already at the correct position
  encoderValue = encoderReadDouble();
  //debugSerial("Initial encoder value: " + String(encoderValue,HEX));
  if (encoderValue == recordNumberOrigHex) { //we are at the correct place, make sure the motor is off
    if (isMagazineActiveFlag == 1) {
      isMagazineActiveFlag = 0;
      digitalWriteFast(DETENT_COIL, HIGH); //off
      //delay(75);
      digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
    }
    scanningStage = 0; //no need to scan further
    transferRecordStage = 1; //set up to transfer a record
    transferRecordWatchdogNowMillis = millis(); //set up the timer to watch record transfer
    transferRecordWatchdogPrevMillis = millis();

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
        digitalWriteFast(DETENT_COIL, LOW);
        digitalWriteFast(MAGAZINE_MOTOR, LOW); 
      } //magazine active check  
    } else { //we found the proper record
      isMagazineActiveFlag = 0;
      digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
      digitalWriteFast(DETENT_COIL, HIGH); //off  
      debugSerial("Pre-scan for record complete. Carousel position: " + String(encoderValue, HEX));
      scanningStage = 2; //set up for stage 2 of scan
      recordNumberHex = recordNumberOrigHex;
    }
  } //end scanning stage 1  
 
  if (scanningStage == 2) {
    transferRecordWatchdogNowMillis = millis(); //we're not doing a transfer yet; just reusing this timer as a safeguard in stage 2
    transferRecordWatchdogPrevMillis = millis();
    //debugSerial("Entering scanning stage 2");
    if (encoderValue != recordNumberHex) {  //if we aren't at the correct position
      if (isMagazineActiveFlag == 0) { //turn on detent coil and magazine motor if needed
        isMagazineActiveFlag = 1;
        digitalWriteFast(DETENT_COIL, LOW); //on
        digitalWriteFast(MAGAZINE_MOTOR, LOW); //on
      }
      //it would seem to make sense to leave the detent coil active until we are at the correct encoder position
      //in reality though, it doesn't seem to "catch" in time, and my magazine is just a fraction of an inch past where it should be
      //adding this 50ms delay and then turning it off seems to work - at least on my unit.. and then we turn the motor off a 
      //fraction of a second later when the encoder shows the correct position. If you have any issues, try commenting out the next 2 lines
      delay(50); 
      digitalWriteFast(DETENT_COIL, HIGH); //off
      digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
      encoderValue = encoderReadDouble();
      //debugSerial("Encoder value: " + String(encoderValue,HEX));    
      while (encoderValue != recordNumberHex) {
        encoderValue = encoderReadDouble();
        digitalWriteFast(DETENT_COIL, LOW); //on
        digitalWriteFast(MAGAZINE_MOTOR, LOW); //on
        //debugSerial("Encoder value: " + String(encoderValue,HEX));    
        //Scan 2 should take less than 1 second.
        transferRecordWatchdogNowMillis = millis();
        if ((uint16_t) (transferRecordWatchdogNowMillis - transferRecordWatchdogPrevMillis) >= 1000) { 
          transferRecordWatchdogPrevMillis = transferRecordWatchdogNowMillis;
          debugSerial("Scan watchdog triggered - Stage 2 taking too long.");
          digitalWriteFast(DETENT_COIL, HIGH); //off  
          digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
          isMagazineActiveFlag = 0;
          scanningStage = 0; //scan complete
          ledDisplayError(4); //scan error
        } //scan timer triggered
      }
      
      //if we're here, we found the right record
      digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
      digitalWriteFast(DETENT_COIL, HIGH); //off  
      delay(50);
      isMagazineActiveFlag = 0;
      encoderValue = encoderReadDouble();
      debugSerial("Scan for record complete. Final magazine position: " + String(encoderValue, HEX));
      scanningStage = 0; //scan complete
      transferRecordStage = 1; //set to 1 to initiate transfer of record from the magazine to the turntable
      transferRecordWatchdogNowMillis = millis(); //set up the timer to watch record transfer
      transferRecordWatchdogPrevMillis = millis();
  } //end scanning stage 2

  //watchdog timer check. A full revolution should take around 16 seconds. If we are trying to scan for more than 20 seconds, something is wrong.
   scanRecordWatchdogNowMillis = millis();
   if ((uint16_t) (scanRecordWatchdogNowMillis - scanRecordWatchdogPrevMillis) >= 20000) { 
      scanRecordWatchdogPrevMillis = scanRecordWatchdogNowMillis;
      debugSerial("Scan watchdog triggered - Scan taking too long.");
      digitalWriteFast(DETENT_COIL, HIGH); //off  
      digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
      isMagazineActiveFlag = 0;
      scanningStage = 0; //scan complete
      ledDisplayError(1); //scan error
    } //scan timer triggered
  } //scanning stage = 2
} //scanning stage > 0
} //end function scanForRecordAsync




//don't allow scanning if a record is playing
void recordScanManual() { //command 995
int exitFlag = 0;
if (isMp3Player == 0) {
if (isRecordPlaying == 0 && transferRecordStage ==0 ) {
  
  debugSerial("Manual Scan - Press 1 to Scan. 0 to exit.");
  isMagazineActiveFlag = 1; //consider the magazine active until we exit this routine
  ledDisplayString(":::1:Scan::0: Done:::");
  delay(200);
  ledDisplayDigits("scn");

  while (exitFlag == 0) {
    Key0.handle();
    Key1.handle();
    if (Key1.isPressed()) {
      digitalWriteFast(DETENT_COIL, LOW); //on
      delay(100);
      digitalWriteFast(MAGAZINE_MOTOR, LOW); //on
    }
    while (Key1.isPressed()) {   
      Key1.handle();
    }
    digitalWriteFast(DETENT_COIL, HIGH); //off
    delay(50);
    digitalWriteFast(MAGAZINE_MOTOR, HIGH); //off
    if (Key0.resetClicked()) {
      exitFlag = 1;
    }
  } //end while
  isMagazineActiveFlag = 0;
  }
} //phono mode activated  
} //end function scanManual








void recordTransferAsync() {
  //Stage 1 - setup Timer. Turn on transfer motor.
  //Stage 2 - wait for 2000ms
  //Stage 3 - wait for cam switch 2 to go high - first part of transfer cycle is over
  if (isMagazineActiveFlag == 0) { //safety check. Do not start any transfer if the magazine is moving
  if (transferRecordStage >= 1 && transferRecordStage <=3) {
    if (transferRecordStage == 1) {
      //start a new timer
      transferRecordNowMillis = millis();
      transferRecordPrevMillis = millis();
      transferRecordStage = 2;
      debugSerial("TransferRecord Stage 1 - setting up timer and turning on transfer motor");
      //turn on transfer motor for 2000 ms. this should activate the cs2 switch which will stay LOW (0) until the record is on the turntable
      debugSerial("Turning transfer motor on.");
      digitalWriteFast(TRANSFER_MOTOR, LOW); 
    } else if (transferRecordStage == 2) {
      //debugSerial("TransferRecord Stage 2 - wait for 2000ms");
        transferRecordNowMillis = millis();
        if ((uint16_t) (transferRecordNowMillis - transferRecordPrevMillis) >= 2000) { 
          transferRecordPrevMillis = transferRecordNowMillis;
          transferRecordStage = 3;
        }
    } else if (transferRecordStage == 3) {
        //debugSerial("TransferRecord Stage 3 - wait for cs2 to go HIGH");
        cs2 = digitalReadFast(CAM2_SWITCH);
        if (activateToggleShiftCoil == 1) {
          digitalWriteFast(TOGGLESHIFT_COIL,LOW); // turn on coil - play flip side of record
        }      
        if (cs2 == 1) { //record has finished transfer
          activateToggleShiftCoil = 0;
          digitalWriteFast(TOGGLESHIFT_COIL, HIGH);   //make sure toggle shift coil is off
          digitalWriteFast(TRANSFER_MOTOR, HIGH); //turn off transfer motor
          debugSerial("Record on turntable. Turning transfer motor off.");
          transferRecordWatchdogNowMillis = millis(); //reset the watchdog   
          transferRecordWatchdogPrevMillis = millis(); //reset the watchdog   
          transferRecordStage = 4; //part of returnRecord
        } //end cs2 == 1
     } //end if transferRecordStage  

       //watchdog timer check. A record transfer should take about 6 seconds. 
       transferRecordWatchdogNowMillis = millis();
     if ((uint16_t) (transferRecordWatchdogNowMillis - transferRecordWatchdogPrevMillis) >= 9000) { 
        transferRecordWatchdogPrevMillis = transferRecordWatchdogNowMillis;
        debugSerial("Record transfer watchdog triggered - more than 9 seconds to transfer.");
        activateToggleShiftCoil = 0;
        digitalWriteFast(TOGGLESHIFT_COIL, HIGH);   //make sure toggle shift coil is off
        digitalWriteFast(TRANSFER_MOTOR, HIGH); //turn off transfer motor
        transferRecordStage = 0;
        ledDisplayError(2); //scan error
      } //transfer watchdog timer triggered
  } //transfer record stage > 0
  } //magazine active check
} //end function transferRecordAsync
