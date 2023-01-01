// Can Display
// https://www.designer2k2.at
// Target: Teensy 4.0 with Cut VUSB trace
// Arduino 1.8.19 with Teensyduino 1.56, Teensy 4.0 with 396MHZ seems enough (and runs cooler)

// ToDo: (* = open,  1-9 completeness where 9 = testet)
// 9 Teensy Test with USB Power (TPS2113A selector)
// 9 Teensy Test with 12V Power (TPS2113A selector)
// 9 Teensy RTC Test  (300ms offset from millis to second)
// 9 Display Test ILI9341
// 9 Display Test Alt Lib with frame buffer: https://github.com/KurtE/ILI9341_t3n Font: https://forum.pjrc.com/threads/65840-ILI9341-Ethnocentric-Font?p=266794&viewfull=1
// 9 Touch Test
// 9 SD Card Test (write test.txt, read test.txt)
// 9 On Board LED test (IO 33, PWM)
// 9 WS1812 RGB LED Interface test (IO 20)
// 9 GPS Module Test (Serial 2)  https://github.com/mikalhart/TinyGPSPlus  or switch to https://github.com/SlashDevin/NeoGPS ?
// 9 Temp Sensor TMP75AIDR Test (A0 to 3.3V = 49)  https://github.com/jeremycole/Temperature_LM75_Derived
// 9 MPU MPU-6050 Test (AD0 to GND = b1101000 68) https://github.com/rfetick/MPU6050_light
// 7 ADC ADS1015IDGS Test (ADDR to GND 48) https://github.com/RobTillaart/ADS1X15
// 9 CAN Bus Test
// * RS232 Test (Serial 1)
// 2 SD File Dump, a method to transfer the file over serial, with python receiver.  Does not work for large files now.
// 8 EEPROM to save some things (like the last shown screen)
// 9 SD LOG Filename every day
// 9 Screenshot transfer & recovery, 450kb transmit, compressed to 65kb
// 9 Send CAN Switch Status (8 of them)
// 7 Send GPS Speed over CAN
// 2 Board (Trip) computer -> finally as standalone lib


// Implement more: Lambda Target (multiply by 128), RPM Limit (in steps of 50), Fuel consumed (16 bit), VE ( 16 bit)

// needed to switch ti "Serial + Keyboard + Mouse + Joystick" as Serial was not working anymore...

// CAN Failures?
// Looks like have Receive and Transmit Error Counters, check them?
// Teensy: https://forum.pjrc.com/threads/24720-Teensy-3-1-and-CAN-Bus/page12
// Teensy 4: https://www.pjrc.com/teensy/IMXRT1060RM_rev2.pdf
// There is a "SYNCH" Bit, that should show if the Speed is ok

// Onboard LED:
#define ONBOARD_LED 33

// Image test:
#include "res/Dlogominiature.c"

// Threading:
#include <Arduino.h>
#include "TeensyThreads.h"
Threads::Mutex mylock;
int blinkythreadID;

// Lock for Serial:
ThreadWrap(Serial, SerialX);
#define Serial ThreadClone(SerialX)

// CAN Bus:
#include <EMUcanT4.h>
EMUcan emucan;
CAN_message_t canMsg1;  //Message to be send

// Display:
#include <Adafruit_GFX.h>
#include <ILI9341_t3n.h>
#include <font_Arial.h>  // from mjs513/ILI9341_t3n
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#include <font_AwesomeF000.h>  // from mjs513/ILI9341_fonts

//Font test:
#include "res/7segment20pt7b.h"


DMAMEM uint16_t fb1[320 * 240];  //Framebuffer
#define TFT_DC 9
#define TFT_CS 10
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC);

// Touchscreen:
#define CS_PIN 8
#define TIRQ_PIN 2
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

// EEPROM:
#include <EEPROM.h>

// Teensy 4.0 Onboard RTC:
#include <TimeLib.h>

// TMP75 Temperature Sensor:
#include <Temperature_LM75_Derived.h>
TI_TMP75 temperature;
volatile float lm75temp;

// MPU6050 IMU:
#include <MPU6050_light.h>
MPU6050 mpu(Wire);

// ADS1015 ADC:
#include "ADS1X15.h"
ADS1115 ADS(0x48);

// SD Card:
#include <SD.h>
const int chipSelect = BUILTIN_SDCARD;
bool datalogactive = false;  //This flag show the datalog is working ok

// WS2812 LED
#include <WS2812Serial.h>
const int numled = 4;
const int pin = 20;
byte drawingMemory[numled * 3];          //  3 bytes per LED
DMAMEM byte displayMemory[numled * 12];  // 12 bytes per LED
WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);
unsigned int rgb_status = 0;

// GPS on Serial2:
#include <TinyGPS++.h>
TinyGPSPlus gps;
#define gpsPort Serial2
int gps_loop_count = 0;

// Variables for the loop:
unsigned long previousMillisS = 0;  // Last Time the Screen was shown
unsigned long previousMillisL = 0;  // Last Time the Log was written
unsigned long intervals = 50;       // Screen Update Interval
boolean ledshine = true;
int emptycycles = 0;

//touchscreen variables:
unsigned long lasttouch = 0;
int screenactive = 1;

// Global Debug Flag:
bool DebugPrint = false;
bool SerialBridge = false;
bool CanDebugPrint = false;

// CAN Switch States:
bool CAN_Switch_1 = false;
bool CAN_Switch_2 = false;
unsigned long CAN_Switch_turnoff = 0;

// Extra CAN Infos:
unsigned int rev_limiter = 6500;  //Pre load with 6500
float fuel_usage = 0;

// Board computer things:
struct trip_data {
  unsigned long trip_distance_last = 0;
  float trip_distance = 0;
  float distance_offset = 0;
  float fuel_offset = 0;
  float trip_fuel_average = 0;
  float trip_fuel_now = 0;
  unsigned long trip_time = 0;
  long trip_time_offset = 0;
  float trip_fuel_used = 0;
  boolean trip_fuel_stationary = true;
  unsigned long trip_last_saved = 0;
};
struct trip_data trip1;
//struct trip_data trip2;

struct trip_data_runtime {
  unsigned long trip_save_time = 0;
  float trip_save_fuel = 0;
  boolean aboveVSS = false;
};
struct trip_data_runtime trip_runtime1;

// Storage for MAX events
struct max_event {
  struct emu_data_t emu_data_store;
  char max_event_time[32];
};
struct max_event emu_max_rpm;
struct max_event emu_max_iat;
struct max_event emu_max_clt;
struct max_event emu_max_map;
struct max_event emu_max_oilp;
struct max_event emu_max_oilt;
struct max_event emu_max_egt;


// Setup: ------------------------------------------------------------------------------------------------
void setup() {

  // This can be used to set the Frequency:
  // set_arm_clock(396000000);  // 396MHZ

  Serial.begin(115200);

  // LED on PCB:
  pinMode(ONBOARD_LED, OUTPUT);
  //Led toggle: 33 = FlexPWM2.0 (Pin 4 and 33) for
  analogWriteFrequency(ONBOARD_LED, 515625);  //515625 for 396mhz 8 bit: https://www.pjrc.com/teensy/td_pulse.html
  analogWrite(ONBOARD_LED, 255);

  Wire.begin();
  //Wire.setClock(400000);

  analogWrite(ONBOARD_LED, 0);

  // set the Time library to use Teensy 4.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  analogWrite(ONBOARD_LED, 255);

  // Wait for Arduino Serial Monitor to open
  //while (!Serial);
  delay(100);
  Serial.print("Can Display Hardware Check Rev 0");

  // Check Clock Speed:
  Serial.print("F_CPU_ACTUAL=");
  Serial.println(F_CPU_ACTUAL);

  analogWrite(ONBOARD_LED, 0);

  //I2C Scanner:
  i2cscanner();


  // Display and Touch:
  tft.begin(60000000);
  //tft.setClock(60000000);     //over 70m breaks the touch, over 100m breaks the tft
  tft.setRotation(1);
  tft.setFrameBuffer(fb1);
  tft.useFrameBuffer(true);
  tft.fillScreen(ILI9341_BLACK);
  tft.updateScreen();
  ts.begin();
  ts.setRotation(3);

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x");
  Serial.println(x, HEX);

  // Initial Text on Display:
  tft.setCursor(1, 10);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(Arial_12);
  tft.setTextSize(2);
  tft.println("Can Display Rev 0");

  //Also print the Build Info:
  tft.print(__DATE__);
  tft.print(" ");
  tft.println(__TIME__);
  tft.setFont(Arial_8);
  tft.println(__FILE__);

  tft.updateScreen();

  // RTC Check,
  if (timeStatus() != timeSet) {
    Serial.println("RTC Test: Unable to sync with the RTC");
  } else {
    Serial.println("RTC Test: RTC has set the system time");
  }

  // TMP75 Sensor, start Wire:
  lm75temp = temperature.readTemperatureC();
  Serial.print("TMP75 Temperature = ");
  Serial.print(lm75temp);
  Serial.println(" C");

  // MPU6050 IMU Check:
  byte statusmpu = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(statusmpu);
  Serial.println(F("will now wait for MPU6050"));
  while (statusmpu != 0) {}  // stop everything if could not connect to MPU6050
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets(true, false);   // gyro and accelero
  mpu.setAccOffsets(0, -1.0, 0);  //Static offset for upright mounting
  Serial.println("Done!\n");
  Serial.print(F("MPU TEMPERATURE: "));
  Serial.println(mpu.getTemp());

  // ADS1015 Setup:
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);
  ADS.begin();
  ADS.setGain(0);
  int16_t val_0 = ADS.readADC(0);
  float f = ADS.toVoltage(1);  // voltage factor
  Serial.print("\tAnalog0: ");
  Serial.print(val_0);
  Serial.print('\t');
  Serial.println(val_0 * f, 3);

  //CAN Bus:
  Serial.print("EMUCANT4_LIB_VERSION: ");
  Serial.println(EMUCANT4_LIB_VERSION);
  //emucan.setClock(CLK_60MHz);   // Experimental, default is 24MHz
  emucan.begin(500000);
  //Check mailbox status:
  emucan.mailboxStatus();

  //Setup the Callback to receive every CAN Message:
  ReturnAllFramesFunction LetMeHaveIt = specialframefunction;
  emucan.ReturnAllFrames(LetMeHaveIt);


  // SD Card:
  Sd2Card card;
  SdVolume volume;
  SdFile root;
  Serial.print("\nInitializing SD card...");
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }
  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }
  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();  // clusters are collections of blocks
  volumesize *= volume.clusterCount();     // we'll have a lot of clusters
  if (volumesize < 8388608ul) {
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize * 512);  // SD card blocks are always 512 bytes
  }
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 2;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  //root.openRoot(volume); //This line stopped working, no idea why.

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
  //finished with the check, open the SD normally:
  SD.begin(chipSelect);



  //GPS Start: https://sixfab.com/wp-content/uploads/2018/10/Quectel_GNSS_SDK_Commands_Manual_V1.4.pdf
  //NMEA Checksum calc: https://nmeachecksum.eqth.net/
  //  Serial.print("Using TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  //  gpsPort.begin(9600);
  //  // Remove not needed things: this keeps only RMC and GGA alive:
  //  gpsPort.print(F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"));
  //  gpsPort.flush();
  //  gpsPort.print(F("$PMTK220,200*2C\r\n"));
  //  gpsPort.flush();
  //  //Switch to 115k
  //  gpsPort.print(F("$PMTK251,115200*1F\r\n"));  //Specific for L80-R module
  //  gpsPort.flush();
  //  delay(100);
  //  gpsPort.end();
  //  gpsPort.begin(115200); //from now on we use 115200k
  //  delay(100);
  //to ease the load, unwanted messages should be turned off: https://www.element14.com/community/community/raspberry-pi/raspberrypi_projects/geocaching/blog/2015/08/14/setting-up-the-raspberry-pi-for-high-speed-gps-5hz-10hz-etc
  // http://www.dragino.com/downloads/downloads/datasheet/other_vendors/L80-R/01%20Software/Quectel_L80-R_GPS_Protocol_Specification_V1.1.pdf
  //TinyGPS needs: GPRMC, GPGGA
  // doc: The default output message of L80-Rhas the followingsixsentences: RMC,VTG, GGA, GSA, GSV and GLL.
  //L80 has: GPGGA GPGSA GPGSV GPGSV GPGSV GPGSV GPGLL GPTXT GPRMC GPVTG
  //Turn of: GPGSA GPGSV GPGSV GPGSV GPGSV GPGLL GPTXT GPVTG
  //Switch to Standby (approx 1mA, any other send = wakeup:
  //gpsPort.print(F("$PMTK161,0*28\r\n"));
  //L80-R Backup Battery: https://www.quectel.com/wp-content/uploads/2021/03/Quectel_L80-R_Hardware_Design_V1.3-1.pdf

  //Switch to GN-801 GPS Module:
  //https://content.arduino.cc/assets/Arduino-MKR-GPS-Shield_u-blox8-M8_ReceiverDescrProtSpec_UBX-13003221_Public.pdf
  //  Serial.print("Using TinyGPS library v. "); Serial.println(TinyGPSPlus::libraryVersion());
  //  gpsPort.begin(115200);

  // Prints  (and a lot of trash)
  //$GNRMC,111513.00,A,4716.81732,N,01125.80000,E,1.167,195.73,280321,,,A*70
  //$GNVTG,195.73,T,,M,1.167,N,2.162,K,A*2C
  //$GNGGA,111513.00,4716.81732,N,01125.80000,E,1,09,1.28,612.5,M,46.2,M,,*42
  //$GNGSA,A,3,12,25,24,32,02,29,,,,,,,2.43,1.28,2.06*1C
  //$GNGSA,A,3,85,66,76,,,,,,,,,,2.43,1.28,2.06*1A
  //$GPGSV,3,1,11,02,29,107,28,06,25,066,16,12,76,034,44,19,16,042,*74
  //$GPGSV,3,2,11,24,44,141,33,25,53,276,39,29,19,206,37,31,01,307,*76
  //$GPGSV,3,3,11,32,36,287,43,36,32,154,42,49,35,189,43*49
  //$GLGSV,2,1,05,65,14,243,,66,25,307,32,76,20,298,32,85,44,148,35*65
  //$GLGSV,2,2,05,,,,36*65
  //$GNGLL,4716.81732,N,01125.80000,E,111513.00,A,A*75

  // Now:
  //$GNRMC,122936.00,A,4716.82541,N,01125.80566,E,0.624,,280321,,,A*6D
  //$GNGGA,122936.00,4716.82541,N,01125.80566,E,1,06,1.11,591.9,M,46.2,M,,*48


  //Switch to BN-280 GPS Module:
  Serial.print("Using TinyGPS library v. ");
  Serial.println(TinyGPSPlus::libraryVersion());
  //gpsPort.print(F("$PUBX,41,1,0003,0003,115200,0*1C\r\n"));  //Specific for Ublox M8, only NMEA+UBX
  // use u-center to setup the GPS module (with serial pass through mode)
  gpsPort.begin(230400);  //from now on we use 230400


  //Restore Settings:
  screenactive = EEPROM.read(1);

  //Restore board computer:
  board_computer_restore(&trip1, 1);

  //WS2812 LED Thread:
  leds.begin();
  blinkythreadID = threads.addThread(blinkythread);

  Serial.println("Thread Blink:" + String(blinkythreadID));

  //Board Computer Thread:
  Serial.println("Board Computer Thread:" + String(threads.addThread(board_computer_thread)));

  Serial.print("EMUCANT4_LIB_VERSION: ");
  Serial.println(EMUCANT4_LIB_VERSION);
}

// Loop: ------------------------------------------------------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:

  // Touch Test, Green dot when touched:
  boolean istouched = ts.touched();
  if (istouched) {
    TS_Point p = ts.getPoint();
    //Only trigger on strong press:
    int z = p.z;
    if (z > 1200) {
      int Xmap, Ymap;
      Xmap = map(p.x, 240, 3800, 0, 320);
      Ymap = map(p.y, 390, 3800, 0, 240);
      tft.fillCircle(Xmap, Ymap, 10, ILI9341_GREEN);
      tft.updateScreen();
      //Serial.print(F("Touch: ")); Serial.println(p.z);

      // call the individual actions
      switch (screenactive) {
        case 1:
          screen1press(Xmap, Ymap, z);
          break;
        case 2:
          screen2press(Xmap, Ymap, z);
          break;
        case 3:
          screen3press(Xmap, Ymap, z);
          break;
        case 4:
          screen4press(Xmap, Ymap, z);
          break;
        case 5:
          screen5press(Xmap, Ymap, z);
          break;
        default:
          //Screen ouf of bounds:
          screenactive = 1;
          break;
      }
    }
  }

  // CAN Loop:
  emucan.checkEMUcan();

  // Max Event check:
  max_event_checker();

  //Mpu fetch:
  mpu.update();

  //GPS Update:
  do {
    int incomingByte = gpsPort.read();
    if (gps.encode(incomingByte)) {
      //TinyGPSPlus returns true if a valid sentence is received = Update every 5 to reduce load.
      gps_loop_count = gps_loop_count + 1;
      if (gps_loop_count >= 5) {
        gps_loop_count = 0;
        Send_CAN_GPS_Speed();
      }
    }
    if (SerialBridge) {
      Serial.write(incomingByte);  //Debug only
    }
  } while (gpsPort.available());

  //Reverse Pass for GPS:
  if (SerialBridge) {
    do {
      gpsPort.write(Serial.read());
    } while (Serial.available());
  }

  //Serial Command receiver:
  if (Serial.available()) {
    String stringcommand;
    stringcommand = Serial.readStringUntil('\n');
    CommandHandler(stringcommand);
  }


  // Loop and only every given interall do something:
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisS >= intervals) {
    previousMillisS = currentMillis;

    switch (screenactive) {
      case 1:
        screen1run();
        break;
      case 2:
        screen2run();
        break;
      case 3:
        screen3run();
        break;
      case 4:
        screen4run();
        break;
      case 5:
        screen5run();
        break;
      default:
        screenactive = 1;
        break;
    }

    //lm75temp = temperature.readTemperatureC();

    //CAN Switch OFF after the time has passed:
    if ((millis() > CAN_Switch_turnoff) && (CAN_Switch_2 == true)) {
      CAN_Switch_2 = false;
      Send_CAN_Switch_States();
      Serial.print("Switch 2: ");
      Serial.println(CAN_Switch_2);
    }

    if (DebugPrint) {
      Serial.print("Empty cycles: ");
      Serial.print(emptycycles);
      Serial.print(" freeram = ");
      Serial.println(freeram());
    }
    emptycycles = 0;

  } else {
    emptycycles += 1;
  }
  if (datalogactive) {
    if (currentMillis - previousMillisL >= 250) {
      previousMillisL = currentMillis;
      datalogtask();
    }
  }
}

// ------------------------------------------------------------------------------------------------
