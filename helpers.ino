// Helper for the RTC Setup:
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

// Helper for the Teensy 4.0 onboard Temp:
extern float tempmonGetTemp(void);

// I2C Scanner:
byte start_address = 0;
byte end_address = 127;
void i2cscanner()
{
  byte rc;

  Serial.println("\nI2C Scanner");

  Serial.print("Scanning I2C bus from ");
  Serial.print(start_address, DEC);  Serial.print(" to ");  Serial.print(end_address, DEC);
  Serial.println("...");

  for ( byte addr  = start_address;
        addr <= end_address;
        addr++ ) {
    Wire.beginTransmission(addr);
    rc = Wire.endTransmission();

    if (addr < 16) Serial.print("0");
    Serial.print(addr, HEX);
    if (rc == 0) {
      Serial.print(" found!");
    } else {
      Serial.print(" "); Serial.print(rc); Serial.print("     ");
    }
    Serial.print( (addr % 8) == 7 ? "\n" : " ");
  }
  Serial.println("\ndone");
}

//Helper for timeprint

void digitalClockDisplay() {
  // digital clock display of the time
  char sz[32];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d.%03ld ",
          year(), month(), day(), hour(), minute(), second(), (millis() - 300) % 1000);
  tft.println(sz);
}


void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  tft.print(":");
  if (digits < 10)
    tft.print('0');
  tft.print(digits);
}

//Thread for fancy colors:
void blinkythread() {
  while (1) {
    // Switch to something usefull if data is received:
    if (emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
      // Blank out all
      if (rgb_status == 0) {
        for (int i = 0; i < leds.numPixels(); i++) {
          // Light brightness low if night time:
          if ((hour() < 7) && (hour() > 19)) {
            leds.setBrightness(5);
          } else {
            leds.setBrightness(50);
          }
          leds.setPixel(i, 0x00000000);
        }
        leds.show();
        rgb_status = 1;
      } //else {
      // First (Bottom) LED, is the NOS, Blue = Armed, Green = Active
      // Top 2 Shiftlight
      if (emucan.emu_data.RPM > 6500) {
        leds.setPixel(3, 0x00FF0000);
        leds.setPixel(4, 0x00FF0000);
        leds.show();
        rgb_status = 0;
      }
      // CEL, second from below.
      if (emucan.decodeCel()) {
        leds.setPixel(2, 0x00FF0000);
        leds.show();
        rgb_status = 0;
      }
      // WAES active, BLUE, parametric 1, bottom:
      if (emucan.emu_data.outflags1 & emucan.F_PO1 ) {
        leds.setPixel(1, 0x000000FF);
        leds.show();
        rgb_status = 0;
      } else {
        // WAES Enabled, GREEN, Switch #3
        if (emucan.emu_data.outflags3 & emucan.F_SW3  ) {
          leds.setPixel(1, 0x00001000);
          leds.show();
          rgb_status = 0;
        }
      }
      threads.delay(100);
      threads.yield();
    } else {
      // Fancy Color Bars Demo Mode
      leds.setBrightness(3);
      rgb_status = 0;
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixel(i, 0x00FF0000);
        leds.show();
        threads.delay(100);
        threads.yield();
      }
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixel(i, 0x0000FF00);
        leds.show();
        threads.delay(100);
        threads.yield();
      }
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixel(i, 0x000000FF);
        leds.show();
        threads.delay(100);
        threads.yield();
      }
      for (int i = 0; i < leds.numPixels(); i++) {
        leds.setPixel(i, 0x008F8F8F);
        leds.show();
        threads.delay(100);
        threads.yield();
      }
    }
  }
}

//Onboard LED Toggle:
void debugLEDtoggle() {
  //Led toggle: 33 = FlexPWM2.0 (Pin 4 and 33) for
  ledshine = !ledshine;
  if (ledshine) {
    analogWrite(ONBOARD_LED, 10); //5 is enough, very bright led...
  } else {
    analogWrite(ONBOARD_LED, 0);
  }
}

//Command handler:
void CommandHandler(String commandtext) {
  Serial.println("Command received: " + commandtext);

  if (commandtext.substring(0, 1) == "?") {
    //Help Text Send:
    Serial.println("CAN Display by designer2k2");
    Serial.println("Commands:");
    Serial.println("?   for this help");
    Serial.println("c:v for software version");
    Serial.println("c:p for next screen");
    Serial.println("c:l for a List of all Files on the SD Card");
    Serial.println("c:s:TEST.TXT  to send TEST.TXT");
    Serial.println("c:d:i for a I2C Scann Result");
    Serial.println("c:d:d toggle Debug Infos");
    Serial.println("c:d:s Serial bridge to the GPS Module");
    Serial.println("c:d:w:1000  Wait with Delay 1000");
    Serial.println("c:d:h:200 converts heading to Text (200=NW)");
    Serial.println("c:d:r:123023 send pixel color from 123 023");
    Serial.println("c:d:p Send Screenshot (print screen)");
    Serial.println("c:d:x:DATALOG.TXT removes DATALOG.TXT");
  }
  else if (commandtext.substring(0, 3) == "c:s") {
    SendFile(commandtext.substring(4));
  }
  else if (commandtext.substring(0, 3) == "c:v") {
    Serial.println("Version: 1.01");
  }
  else if (commandtext.substring(0, 3) == "c:l") {
    ListFile();
  }
  else if (commandtext.substring(0, 5) == "c:d:i") {
    i2cscanner();
  }
  else if (commandtext.substring(0, 5) == "c:d:d") {
    //Toggle Debug Infos
    DebugPrint = !DebugPrint;
  }
  else if (commandtext.substring(0, 5) == "c:d:s") {
    //Serial Bridge to GPS
    SerialBridge = !SerialBridge;
  }
  else if (commandtext.substring(0, 5) == "c:d:w") {
    //Delay with given time (c:d:w:1000 = 1Sec)
    String rt = commandtext.substring(6);
    rt.trim();
    int wait = rt.toInt();
    Serial.println("Wait for (ms): " + String(wait));
    delay(wait);
  }
  else if (commandtext.substring(0, 5) == "c:d:h") {
    //Heading Return (c:d:h:200 = NW)
    String rt = commandtext.substring(6);
    rt.trim();
    int heading = rt.toInt();
    Serial.println("Heading: " + String(heading) + " String: " + headingToText(heading));
  }
  else if (commandtext.substring(0, 3) == "c:p") {
    // Switch to the next screen
    ScreenSwitch();
  }
  else if (commandtext.substring(0, 5) == "c:d:r") {
    // Send the Pixel Color from the requested Pixel: 001002 = X1 Y2
    String rt = commandtext.substring(6);
    rt.trim();
    int x = rt.substring(0, 3).toInt();
    int y = rt.substring(3).toInt();

    Serial.print("Pixel X:" + String(x) + ":Y:" + String(y) + ":");
    uint8_t r, g, b;
    uint16_t coltodecode = fb1[x + y * 320];
    tft.color565toRGB(coltodecode, r, g, b);
    Serial.println(String(r) + ":" + String(g) + ":" + String(b));
  }
  else if (commandtext.substring(0, 5) == "c:d:p") {
    // Send a Screenshot:
    screenshotToConsole();
  }
  else if (commandtext.substring(0, 5) == "c:d:x") {
    // remove the given file
    String rt = commandtext.substring(6);
    rt.trim();
    Serial.print("Remove:" + rt + ":");
    Serial.println(String(SD.remove(rt.c_str())));
  }
  else {
    Serial.print("Command not recognized");
  }
}

void SendFile(String FileName) {
  FileName.trim();  // this is needed to get the filename to work properly...
  Serial.println("Sending:" + FileName + ".");
  Serial.flush();
  //Check if Datalog is active! Do not send with active log

  File dataFile = SD.open(FileName.c_str(), FILE_READ);

  Serial.println("Filesize:" + String((unsigned long)dataFile.size()) + ".");

  //send the header:  ;FILENAME;SIZE;CREATED;LASTMODIFIED
  //Start: STX (2)
  Serial.write(char(2));
  Serial.flush();

  // From: https://www.arduino.cc/en/Tutorial/LibraryExamples/DumpFile
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      //Serial.flush();
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening:" + FileName + ".");
  }

  //End: EOT (4)
  Serial.write(char(4));
}

void ListFile() {
  Serial.println("Sending Filelist on SD Card");
  File rootdir;
  rootdir = SD.open("/");
  printDirectory(rootdir, 0);
}

// From https://www.arduino.cc/en/Tutorial/LibraryExamples/Listfiles
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}


// Freeram on Teensy 4 https://forum.pjrc.com/threads/33443-How-to-display-free-ram?p=275013&viewfull=1#post275013
extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;

int freeram() {
  return (char *)&_heap_end - __brkval;
}

// Heading to Text (N-NE-E)
String headingToText(int angle) {
  // Valid angle: 0-360
  String directions[] = {"N", "NO", "O", "SO", "S", "SW", "W", "NW", "N"};
  int index = round(angle / 45.0);
  return directions[index];
}

//Screen switcher:
void ScreenSwitch() {

  screenactive += 1;

  switch (screenactive) {
    case 1:
      screen1setup();
      break;
    case 2:
      screen2setup();
      break;
    case 3:
      screen3setup();
      break;
    case 4:
      screen4setup();
      break;
    case 5:
      screen5setup();
      break;
    default:
      screen1setup();
      screenactive = 1;
      break;
  }
  // and store it:
  EEPROM.write(1, screenactive);   //First Setting = active Screen
}

// CAN Bus send Switch States:
void Send_CAN_Switch_States() {
  if (emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
    byte CAN_Switch_State = 0;
    bitWrite(CAN_Switch_State, 0, CAN_Switch_1);
    bitWrite(CAN_Switch_State, 1, CAN_Switch_2);
    if (DebugPrint) {
      Serial.print("Send Switch State over CAN: "); Serial.println(CAN_Switch_State, BIN);
    }
    // Frame to be send:
    canMsg1.id  = 0x0F6;
    canMsg1.len = 1;
    canMsg1.buf[0] = CAN_Switch_State;
    //Sends the frame;
    emucan.sendFrame(canMsg1);
  } else {
    if (DebugPrint) {
      Serial.println("Did not send Switch State over CAN due to no communication.");
    }
  }
}

// CAN Bus send GPS:
void Send_CAN_GPS_Speed() {
  if ((emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) && (gps.location.isValid() == true)) {
    //gps.speed.kmph());  //double
    // GPS Speed = 8 Bit Unsigned
    byte gps_speed = (byte) gps.speed.kmph();
    if (DebugPrint) {
      Serial.print("Send GPS Speed over CAN: "); Serial.println(gps_speed);
    }
    // Frame to be send:
    canMsg1.id  = 0x0F7;
    canMsg1.len = 1;
    canMsg1.buf[0] = gps_speed;
    //Sends the frame;
    emucan.sendFrame(canMsg1);
  } else {
    if (DebugPrint) {
      Serial.println("Did not send GPS Speed over CAN due to no communication or no GPS lock");
    }
  }
}

// self defined function to handle received frames with additional content
void specialframefunction(const CAN_message_t *frame) {
  //Magic things can happen here, but dont block!

  if (frame->id == 0x520) {
    //Byte 0 = Lambda Target * 128
    //emucan.emu_data.lambdaTarget = frame->buf[0] / 128.0;
  }

  if (frame->id == 0x521) {
    //Byte 0 = Rev Limiter / 50
    //Byte 1/2 = Fuel used * 100
    //Byte 3/4 = Fuel usage * 100
    //rev_limiter = frame->buf[0] * 50; thats only 0/1 not the actual value
    //fuel_used = ((frame->buf[2] << 8) + frame->buf[1]) / 100.0;  // Send 16bit unsigned little endian
    fuel_usage = ((frame->buf[4] << 8) + frame->buf[3]) / 100.0; // Send 16bit unsigned little endian
  }


}

//Max Checker:
void max_event_checker() {

  char sz[32];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d.%03ld ",
          year(), month(), day(), hour(), minute(), second(), (millis() - 300) % 1000);

  // Max RPM:
  if (emucan.emu_data.RPM > emu_max_rpm.emu_data_store.RPM) {
    emu_max_rpm.emu_data_store = emucan.emu_data;
    strcpy(emu_max_rpm.max_event_time, sz);
  }

  // Max IAT:
  if (emucan.emu_data.IAT > emu_max_iat.emu_data_store.IAT) {
    emu_max_iat.emu_data_store = emucan.emu_data;
    strcpy(emu_max_iat.max_event_time, sz);
  }

  // Max CLT:
  if (emucan.emu_data.CLT > emu_max_clt.emu_data_store.CLT) {
    emu_max_clt.emu_data_store = emucan.emu_data;
    strcpy(emu_max_clt.max_event_time, sz);
  }

  // Max MAP:
  if (emucan.emu_data.MAP > emu_max_map.emu_data_store.MAP) {
    emu_max_map.emu_data_store = emucan.emu_data;
    strcpy(emu_max_map.max_event_time, sz);
  }

  // Max oilp:
  if (emucan.emu_data.oilPressure > emu_max_oilp.emu_data_store.oilPressure) {
    emu_max_oilp.emu_data_store = emucan.emu_data;
    strcpy(emu_max_oilp.max_event_time, sz);
  }

  // Max oilt:
  if (emucan.emu_data.oilTemperature  > emu_max_oilt.emu_data_store.oilTemperature) {
    emu_max_oilt.emu_data_store = emucan.emu_data;
    strcpy(emu_max_oilt.max_event_time, sz);
  }

  // Max egt:
  if (emucan.emu_data.Egt1 > emu_max_egt.emu_data_store.Egt1) {
    emu_max_egt.emu_data_store = emucan.emu_data;
    strcpy(emu_max_egt.max_event_time, sz);
  }
  // Max egt2:
  if (emucan.emu_data.Egt2 > emu_max_egt.emu_data_store.Egt2) {
    emu_max_egt.emu_data_store = emucan.emu_data;
    strcpy(emu_max_egt.max_event_time, sz);
  }

}
