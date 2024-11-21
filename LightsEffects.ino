#ifdef RGBLIGHTS

//built in colors
typedef enum {
  //reds
  Red = 0xff0000,
  DarkRed = 0x8b0000,
  Coral = 0xff7f50, //orangish
  LightCoral = 0xf08080, //orangish
  //oranges
  DarkOrange = 0xff8c00,
  Orange = 0xffa500,
  //Yellows
  Gold = 0xffd700, //orangish
  Yellow = 0xffff00, //orangish
  Khaki = 0xf0e68c, //orangish
  //greens
  DarkGreen = 0x0006400,
  LightGreen = 0x7fc000,
  Green = 0x00ff00,
  //blues
  Teal = 0x008080,
  Aqua = 0x00ffff,
  SkyBlue = 0x00bf00,
  Blue = 0x0000ff,
  DarkBlue = 0x00008b,
  //purples -- all of these purples look like white or pink when filtered through the orange lens
  BlueViolet = 0x8a2be2,
  Indigo = 0x4b0082,
  MediumOrchid = 0xba55d3,
  Purple = 0x800080, 
  //pinks -- all of the pinks look white white or orange when filtered through the orange lens, except fuscia
  Fuscia = 0xff00ff,
  Violet = 0xee82ee,
  HotPink = 0xff69b4,
  //other
  White = 0xffffff,
  Grey = 0x606060,
  Black = 0x000000
} ColorCodes;

  //there is another memory management bug trying to define these globally. I'm too lazy to try and track down another one...
  //AlaColor colorsArray[3] = { Orange, Orange, Orange }; 
  //AlaPalette colorsPalette = { 3, colorsArray };
  

int getNumberInput(int numberOfDigits, int maxValue) {
int returnValue = 32767;
inputSelectionString = "";

while (returnValue > maxValue) {
  while (inputSelectionString.length() < numberOfDigits) {
    getDigitsInput();
    ledUpdateDisplay();
  }
  returnValue = inputSelectionString.toInt();
  //Serial.println("getNumberInput returning: " + String(returnValue));
  inputSelectionString = "";
}
return returnValue;
} //end function getNumberInput


//697 - input custom brightness only
void inputCustomBrightnessData() {
  ledUpdateDigitsArray("br");
  delay(1000);
  customBrightnessR = getNumberInput(3, 255);
  ledUpdateDigitsArray("bg");
  delay(1000);
  customBrightnessG = getNumberInput(3, 255);
  ledUpdateDigitsArray("bb");
  delay(1000);
  customBrightnessB = getNumberInput(3, 255);
}

//698 - input custom timer only
void inputCustomTimerData() {
  ledUpdateDigitsArray("tmr");
  delay(1000);
  customColorTimer = getNumberInput(5, 32000);
} //end function inputCustomTimerData

//TODO - 699 - this routine may take a while for someone to enter all data. only allow entry when no song playing
void inputCustomLightsData() {
  if (isRecordPlaying == 0 && transferRecordStage == 0) {
    ledUpdateDigitsArray("c1r");
    delay(1000);
    customColor1R = getNumberInput(3, 255); //yes, the color is a byte. and the function is returning an int. But with a controlled value of 0 - 255
    Serial.println("color1r: " + String(customColor1R));
    ledUpdateDigitsArray("c1g");
    delay(1000);
    customColor1G = getNumberInput(3, 255);
    ledUpdateDigitsArray("c1b");
    delay(1000);
    customColor1B = getNumberInput(3, 255);
    ledUpdateDigitsArray("c2r");
    delay(1000);
    customColor2R = getNumberInput(3, 255);
    ledUpdateDigitsArray("c2g");
    delay(1000);
    customColor2G = getNumberInput(3, 255);
    ledUpdateDigitsArray("c2b");
    delay(1000);
    customColor2B = getNumberInput(3, 255);
    ledUpdateDigitsArray("c3r");
    delay(1000);
    customColor3R = getNumberInput(3, 255);
    ledUpdateDigitsArray("c3g");
    delay(1000);
    customColor3G = getNumberInput(3, 255);
    ledUpdateDigitsArray("c3b");
    delay(1000);
    customColor3B = getNumberInput(3, 255);
    ledUpdateDigitsArray("br");
    delay(1000);
    customBrightnessR = getNumberInput(3, 255);
    ledUpdateDigitsArray("bg");
    delay(1000);
    customBrightnessG = getNumberInput(3, 255);
    ledUpdateDigitsArray("bb");
    delay(1000);
    customBrightnessB = getNumberInput(3, 255);
    ledUpdateDigitsArray("tmr");
    delay(1000);
    customColorTimer = getNumberInput(5, 32000);
  }
} //end function inputCustomLightsData();




//allows you to enter 3 rgb values and immediately see the light display change
//enter 000 000 000 to exit this routine
//you can't play around with this routine when a record is playing
void lightsColorTest() { //690 command
int c1 = 255;
int c2 = 255;
int c3 = 255;

if (isRecordPlaying == 0 && transferRecordStage ==0 ) {
    while (c1 != 0 || c2 != 0 || c3 != 0) {
      ledUpdateDigitsArray("c1r");
      delay(1000);
      c1 = getNumberInput(3, 255); //yes, the color is a byte. and the function is returning an int. But with a controlled value of 0 - 255
      Serial.println("color1r: " + String(customColor1R));
      ledUpdateDigitsArray("c1g");
      delay(1000);
      c2 = getNumberInput(3, 255);
      ledUpdateDigitsArray("c1b");
      delay(1000);
      c3 = getNumberInput(3, 255);
      AlaColor colorsArray[1] = { ((unsigned long)c1 << 16 | (unsigned long)c2 << 8 | (unsigned long)c3) }; 
      AlaPalette colorsPalette = { 1, colorsArray };
      leds.setAnimation(ALA_ON, 1000, colorsPalette); 
      leds.setBrightness(0xffffff);
      leds.runAnimation();
    } //while loop
    inputSelectionNumber = currentLightsPattern;
    lightsSelector();
} //end if is record playing
} //end function lightsColorTest



// ON, OFF, BLINKALT, SPARKLE, SPARKLE2, PLASMA, CYCLECOLORS, FADEIN, FADEOUT, FADEINOUT, GLOW, FADECOLORS, FADECOLORSLOOP
//based on the 3 digit number input from the keypad, setup the lights routine to display
void lightsSelector() {
  if (inputSelectionNumber == 600) {
    AlaColor colorsArray[1] = { Black }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_OFF, 4000, colorsPalette); 
  } else if (inputSelectionNumber == 601) { //all on orange
    AlaColor colorsArray[1] = { Orange }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_ON, 1000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 602) { //all on red
    AlaColor colorsArray[1] = { Red }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_ON, 1000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 603) { //all on green
    AlaColor colorsArray[1] = { Green }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_ON, 1000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 604) { //all on blue
    AlaColor colorsArray[1] = { Blue }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_ON, 1000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 605) { //all on custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_ON, 1000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 606) { //blink red
    AlaColor colorsArray[1] = { Red }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_BLINKALT, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 607) { //blink green
    AlaColor colorsArray[1] = { Green }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_BLINKALT, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 608) { //blink blue
    AlaColor colorsArray[1] = { Blue }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_BLINKALT, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 609) { //blink custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_BLINKALT, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 610) { //sparkle red
    AlaColor colorsArray[1] = { Red }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 611) { //sparkle green
    AlaColor colorsArray[1] = { Green }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 612) { //sparkle blue
    AlaColor colorsArray[1] = { Blue }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 613) { //sparkle custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 614) { //sparkle2 red, green, blue
    AlaColor colorsArray[3] = { Red, Green, Blue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE2, 100, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 615) { //sparkle2 greens
    AlaColor colorsArray[3] = { LightGreen, Teal, SkyBlue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE2, 100, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 616) { //sparkle2 blue
    AlaColor colorsArray[3] = { Blue, White, Aqua }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE2, 100, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 617) { //sparkle2 blue
    AlaColor colorsArray[3] = { Blue, White, Red }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE2, 100, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 618) { //sparkle2 custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE2, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 619) { //sparkle2 random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_SPARKLE2, 100, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 620) { //plasma red, green, blue
    AlaColor colorsArray[3] = { Red, Green, Blue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_PLASMA, 5000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 621) { //plasma greens
    AlaColor colorsArray[3] = { LightGreen, Teal, SkyBlue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_PLASMA, 5000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 622) { //plasma blue
    AlaColor colorsArray[3] = { Blue, White, Aqua }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_PLASMA, 5000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 623) { //plasma blue
    AlaColor colorsArray[3] = { Blue, White, Red }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_PLASMA, 5000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 624) { //plasma custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_PLASMA, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 625) { //plasma random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_PLASMA, 5000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 626) { //cycle red, green, blue
    AlaColor colorsArray[3] = { Red, Green, Blue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_CYCLECOLORS, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 627) { //cycle greens
    AlaColor colorsArray[3] = { LightGreen, Teal, SkyBlue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_CYCLECOLORS, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 628) { //cycle blue white aqua
    AlaColor colorsArray[3] = { Blue, White, Aqua }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_CYCLECOLORS, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 629) { //cycle blue white red
    AlaColor colorsArray[3] = { Blue, White, Red }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_CYCLECOLORS, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 630) { //cycle custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_CYCLECOLORS, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 631) { //cycle random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_CYCLECOLORS, 4000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 632) { //moving gradient red, green, blue
    AlaColor colorsArray[3] = { Red, Green, Blue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGGRADIENT, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 633) { //moving gradient lightgreen teal skyblue
    AlaColor colorsArray[3] = { LightGreen, Teal, SkyBlue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGGRADIENT, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 634) { //moving gradient blue white aqua
    AlaColor colorsArray[3] = { Blue, White, Aqua }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGGRADIENT, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 635) { //moving gradient blue white red
    AlaColor colorsArray[3] = { Blue, White, Red }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGGRADIENT, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 636) { //moving gradient custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGGRADIENT, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 637) { //moving gradient random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGGRADIENT, 4000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 638) { //fade colors loop red, green, blue
    AlaColor colorsArray[3] = { Red, Green, Blue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_FADECOLORSLOOP, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 639) { //fade colors loop lightgreen teal skyblue
    AlaColor colorsArray[3] = { LightGreen, Teal, SkyBlue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_FADECOLORSLOOP, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 640) { //fade colors loop blue white aqua
    AlaColor colorsArray[3] = { Blue, White, Aqua }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_FADECOLORSLOOP, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 641) { //fade colors loop blue white red
    AlaColor colorsArray[3] = { Blue, White, Red }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_FADECOLORSLOOP, 4000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 642) { //fade colors loop custom
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_FADECOLORSLOOP, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 643) { //fade colors loop random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_FADECOLORSLOOP, 4000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 644) { //moving bars red, green, blue
    AlaColor colorsArray[3] = { Red, Green, Blue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGBARS, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 645) { //moving bars lightgreen teal skyblue
    AlaColor colorsArray[3] = { LightGreen, Teal, SkyBlue }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGBARS, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 646) { //moving bars blue white aqua
    AlaColor colorsArray[3] = { Blue, White, Aqua }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGBARS, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 647) { //moving bars blue white red
    AlaColor colorsArray[3] = { Blue, White, Red }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGBARS, 2000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 648) { //moving bars custom colors
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGBARS, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 649) { //moving bars random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_MOVINGBARS, 2000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 650) { //glow red
    AlaColor colorsArray[1] = { Red }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_GLOW, 7000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 651) { //glow green
    AlaColor colorsArray[1] = { Green }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_GLOW, 7000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 652) { //glow blue
    AlaColor colorsArray[1] = { Blue }; 
    AlaPalette colorsPalette = { 1, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_GLOW, 7000, colorsPalette); 
    leds.setBrightness(0xffffff);
  } else if (inputSelectionNumber == 653) { //glow custom colors
    AlaColor colorsArray[3] = { ((unsigned long)customColor1R << 16 | (unsigned long)customColor1G << 8 | (unsigned long)customColor1B), ((unsigned long)customColor2R << 16 | (unsigned long)customColor2G << 8 | (unsigned long)customColor2B), ((unsigned long)customColor3R << 16 | (unsigned long)customColor3G << 8 | (unsigned long)customColor3B) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_GLOW, customColorTimer, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if (inputSelectionNumber == 654) { //glow random colors
    AlaColor colorsArray[3] = { ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)), ((unsigned long)random(0,255) << 16 | (unsigned long)random(0,255) << 8 | (unsigned long)random(0,255)) }; 
    AlaPalette colorsPalette = { 3, colorsArray };
    currentLightsPattern = inputSelectionNumber;
    leds.setAnimation(ALA_GLOW, 7000, colorsPalette); 
    leds.setBrightness( ((unsigned long)customBrightnessR << 16 | (unsigned long)customBrightnessG << 8 | (unsigned long)customBrightnessB) );
  } else if(inputSelectionNumber == 690) { //color test
    lightsColorTest();
  } else if(inputSelectionNumber == 697) { //custom brightness
    inputCustomBrightnessData();
  } else if (inputSelectionNumber == 698){ //custom timer
    inputCustomTimerData();
  } else if (inputSelectionNumber == 699) { //enter complete custom pattern
    inputCustomLightsData();
  }

} //end function lightsSelector



void lightsSetup() {
  byte pins[] = { LED1RED, LED1GREEN, LED1BLUE, LED2RED, LED2GREEN, LED2BLUE, LED3RED, LED3GREEN, LED3BLUE };
  leds.initPWM(3, pins);
  leds.setBrightness(0xffffff);
}  //end function lightsSetup



#endif  //if RGBLIGHTS is defined