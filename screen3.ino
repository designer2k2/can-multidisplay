// Scope with Circular Buffer:

/* ToDo
  Switchable data display
  Data labels
*/


#include <CircularBuffer.h>     //https://github.com/rlogiacco/CircularBuffer/

CircularBuffer<byte, 320> buffer;  //Framebuffer1 for the Scope Data
CircularBuffer<byte, 320> buffer2; //Framebuffer2 for the Scope Data
CircularBuffer<byte, 320> buffer3; //Framebuffer3 for the Scope Data
int sincount = 0;

boolean scoperun = true;

void screen3press(int X, int Y, int Z) {
  if (millis() > lasttouch + 500) {
    lasttouch = millis();
    if (X > 160) {
      if (Y > 120) {
        //switch screens
        ScreenSwitch();
      }
    } else {
      //toggle scope update
      scoperun = !scoperun;
    }
  }
}

void screen3setup() {
  intervals = 50;  // Update every 50ms
}

void screen3run() {
  intervals = 50;  // Update every 50ms
  //Led toggle:
  debugLEDtoggle();

  // If Emu Data is present, no demo mode:
  bool DemoMode = true;
  if (emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
    DemoMode = false;
  }

  // Temp Text on Display:
  tft.fillScreen(ILI9341_BLACK);


  //Store new value into buffer if enabled:
  if (scoperun) {

    if (DemoMode) {
      //buffer.push(millis() % 200);

      //Sinewave dummy:
      for (int i = 0; i < 3; i++) {
        int restostore = int(sin(sincount * DEG_TO_RAD) * 100) + 100;
        int restostore2 = int(cos(sincount * DEG_TO_RAD) * 100) + 100;
        int restostore3 = int(sincount / 1.8);

        buffer.push(restostore);
        buffer2.push(restostore2);
        buffer3.push(restostore3);

        sincount += 1;
        if (sincount >= 361) {
          sincount = 0;
        }
      }
    } else {
      buffer.push(240 - (emucan.emu_data.RPM / 30));
      buffer2.push(240 - (emucan.emu_data.MAP * 1.3));
      buffer3.push(240 - (emucan.emu_data.TPS * 2.4));
    }
  }


  //Plot buffer:

  // the following ensures using the right type for the index variable
  using index_t = decltype(buffer)::index_t;
  for (index_t i = 0; i < buffer.size(); i++) {
    tft.drawPixel(i, buffer[i], ILI9341_RED);
  }

  using index_t = decltype(buffer2)::index_t;
  for (index_t i = 0; i < buffer2.size(); i++) {
    tft.fillCircle(i, buffer2[i], 2, ILI9341_GREEN);
  }

  using index_t = decltype(buffer3)::index_t;
  for (index_t i = 0; i < buffer3.size(); i++) {
    tft.fillCircle(i, buffer3[i], 1, tft.color565(buffer3[i], buffer3[i], 256 - buffer3[i]));
  }

  // Logo
  tft.writeRect(280, 200, 40, 39, (uint16_t*)Dlogominiature);
  //Date and Time from RTC:
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.setTextSize(1);
  tft.setCursor(220, 230);
  char sz[32];
  sprintf(sz, "%02d.%02d.%02d",
          day(), month(), year());
  tft.print(sz);
  sprintf(sz, "%02d:%02d:%02d",
          hour(), minute(), second());
  tft.setFont(Arial_16);
  tft.setTextSize(1);
  tft.setCursor(190, 210);
  tft.print(sz);

  //Fontawesome play/pause button:
  tft.setFont(AwesomeF000_18);
  tft.setCursor(0, 215);
  if (scoperun) {
    tft.setTextColor(ILI9341_GREEN);
    tft.print(char(75));  // 75 = play, 76 = pause https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128
    intervals = 50;  // Update every 50ms
  } else {
    tft.setTextColor(ILI9341_YELLOW);
    tft.print(char(76));  // 75 = play, 76 = pause https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128
    intervals = 500;  // Update every 500ms
  }

  //Update the screen:
  tft.updateScreen();
}
