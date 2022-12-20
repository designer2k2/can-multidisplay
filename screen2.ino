//Screen 2  Arc Dash with little Scope
// The "MAIN" Screen

/*To Do:
   IgnAngle & Injector PW weg
   Exhaust average + diff
   Anstelle von E2 den Öldruck/Öltemp
   Warndreieck großes Overlay, Transparent
   Color Code the Temperatures? EGT -400 Blue, 401-850 G, 851- R,  C -70 R, 71-99 G, 99- R
   Gelber Balken zwischen Lambda soll und target
   rgb lights for button and nos and check engine and shiftlight
   Öldruck Anzeigen
*/

#include <CircularBuffer.h>     //https://github.com/rlogiacco/CircularBuffer/

#include "res/icons.c"    //Contains fancy icons

int screen2var = 0;
unsigned int arcsHigh = 0;
unsigned long arcsHighTime = 0;
int mapHigh = 0;
unsigned long mapHighTime = 0;

CircularBuffer<byte, 150> bufferS;  //Framebuffer1 for the MAP Scope Data, or TPS
CircularBuffer<byte, 150> bufferS2;  //Framebuffer2 for the RPM Scope Data, or Lambda

void screen2press(int X, int Y, int Z) {
  if (millis() > lasttouch + 1000) {
    lasttouch = millis();
    if (X > 160) {
      if (Y > 120) {
        //Lower Right:
        //switch screens
        ScreenSwitch();
      } else {
        //Upper Right:
        //Toggle CAN Switch
        CAN_Switch_1 = !CAN_Switch_1;
        Serial.print("Switch 1: "); Serial.println(CAN_Switch_1);
        Send_CAN_Switch_States();
      }
    } else
    {
      if (Y < 120) {
        //Upper Left:
        //Log toggle:
        datalogactive = !datalogactive;
        //And store Bord Computer if logging stops:
        if (!datalogactive) {
          board_computer_save(&trip1, 1);
        }
      } else {
        //Lower Left:
        //CAN Switch High with turnoff time
        CAN_Switch_2 = true;
        CAN_Switch_turnoff = millis() + 1000;
        Serial.print("Switch 2: "); Serial.println(CAN_Switch_2);
        Send_CAN_Switch_States();
      }
    }
  }
}

void screen2setup() {
  intervals = 75;  // Update every 200ms
}

void screen2run() {
  intervals = 75;  // Update every 75ms
  //Led toggle:
  debugLEDtoggle();

  // If Emu Data is present, no demo mode:
  bool DemoMode = true;
  if (emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
    DemoMode = false;
  }

  // Temp Text on Display:
  tft.fillScreen(ILI9341_BLACK);
  // Logo
  tft.writeRect(280, 200, 40, 39, (uint16_t*)Dlogominiature);

  // Tach: (320/4 = 80, 240 = 120)
  drawArc(80, 120, 75, 1, 135, 45, ILI9341_LIGHTGREY);
  drawArc(80, 120, 76, 1, 135, 45, ILI9341_DARKGREY);
  //tft.fillCircle(80, 120, 10, ILI9341_BLUE);

  // RPM, for now its the seconds * 120 = max 7200. 7k / 270 = 26, and then 135 offset
  unsigned int rpms;
  if (DemoMode) {
    rpms = (second() + ((millis() - 300) % 1000) / 1000.0) * 120.0;
    rev_limiter = 6500;
  } else {
    rpms = emucan.emu_data.RPM;
    if (emucan.emu_data.oilTemperature > 40) {
      rev_limiter = 6500;
    } else {
      rev_limiter = 3000;
    }
  }

  //Save RPM/TPS to the buffer:
  if (DemoMode) {
    bufferS2.push(second() * 4);
  } else {
    //bufferS2.push(rpms / 28); //Scale max 7000rpm to fit inside 0-256
    bufferS2.push(emucan.emu_data.TPS * 2.55); //Scale max 100 to fit inside 0-256

  }

  unsigned int arcs = rpms / 26.0;
  arcs += 135;  //Start Offset
  if (rpms < rev_limiter) {
    drawArc(80, 120, 74, 13, 135, arcs, tft.color565(0, 255, 0));
    drawArc(80, 120, 61, 2, 135, arcs, tft.color565(0, 192, 0));  // slight 3D effect
    drawArc(80, 120, 59, 2, 135, arcs, tft.color565(0, 128, 0));  // slight 3D effect
    drawArc(80, 120, 57, 2, 135, arcs, tft.color565(0, 64, 0));  // slight 3D effect
    drawArc(80, 120, 55, 2, 135, arcs, tft.color565(0, 32, 0));  // slight 3D effect
  } else {
    drawArc(80, 120, 74, 13, 135, arcs, tft.color565(255, 0, 0));
    drawArc(80, 120, 61, 2, 135, arcs, tft.color565(192, 0, 0));  // slight 3D effect
    drawArc(80, 120, 59, 2, 135, arcs, tft.color565(128, 0, 0));  // slight 3D effect
    drawArc(80, 120, 57, 2, 135, arcs, tft.color565(64, 0, 0));  // slight 3D effect
    drawArc(80, 120, 55, 2, 135, arcs, tft.color565(32, 0, 0));  // slight 3D effect
  }

  // RPM warn area:
  drawArc(80, 120, 74, 5, rev_limiter / 26 + 135, 45, tft.color565(255, 0, 0)); // slight 3D effect
  drawArc(80, 120, 69, 5, rev_limiter / 26 + 135, 45, tft.color565(128, 0, 0)); // slight 3D effect

  //RPM Store high, and drop after 10 sec by 5deg each round
  uint16_t rpmcolor = ILI9341_RED;
  if ((millis() - arcsHighTime) > 10000) {
    if (arcsHigh > 5) {
      arcsHigh -= 5;
    }
    rpmcolor = ILI9341_BLUE;
  }
  if (arcs >= arcsHigh) {
    arcsHigh = arcs;
    arcsHighTime = millis();
  }
  drawArc(80, 120, 74, 15, arcsHigh - 2, arcsHigh + 2, rpmcolor);

  //RPM Markings
  for (int i = 1; i <= 7; i++) {
    int drawpos = i * 38.57;
    drawpos = drawpos + 135;
    drawArc(80, 120, 75, 10, drawpos - 2, drawpos + 2, ILI9341_DARKGREY);
  }

  //MAP, inner ring, from 0 to 180, colorswitch at 100,  270/180=1,5
  if (DemoMode) {
    screen2var += 3;
    if (screen2var > 180) {
      screen2var = 0;
    }
    bufferS.push(screen2var * 1.4);
  } else {
    screen2var = emucan.emu_data.MAP;
  }

  //Save the MAP to the buffer too:
  //bufferS.push(screen2var * 1.4);
  // or Lambda:
  int LambBuf = (emucan.emu_data.wboLambda * 250) - 125; //(0-255 for lambda 0.5 to 1.5)
  LambBuf = constrain(LambBuf , 0, 250);
  bufferS.push(LambBuf);

  int MapArc = screen2var * 1.5;
  MapArc += 135;
  drawArc(80, 120, 44, 5, 135, MapArc, ILI9341_YELLOW);
  drawArc(80, 120, 39, 2, 135, MapArc, tft.color565(192, 192, 0));
  drawArc(80, 120, 37, 2, 135, MapArc, tft.color565(128, 128, 0));
  drawArc(80, 120, 35, 2, 135, MapArc, tft.color565(64, 64, 0));
  drawArc(80, 120, 33, 2, 135, MapArc, tft.color565(32, 32, 0));

  //MAP Store high, and drop after 10 sec by 3deg each round
  rpmcolor = ILI9341_RED;
  if ((millis() - mapHighTime) > 10000) {
    if (mapHigh > 3) {
      mapHigh -= 3;
    }
    rpmcolor = ILI9341_BLUE;
  }
  if (MapArc > mapHigh) {
    mapHigh = MapArc;
    mapHighTime = millis();
  }
  drawArc(80, 120, 44, 9, mapHigh - 4, mapHigh + 4, rpmcolor);

  //MAP 100 mark (100*1,5+135):
  drawArc(80, 120, 44, 7, 285 - 3, 285 + 3, ILI9341_DARKGREY);
  drawArc(80, 120, 45, 1, 135, 45, ILI9341_LIGHTGREY);
  drawArc(80, 120, 46, 1, 135, 45, ILI9341_DARKGREY);



  // Lambda Bar 150 to 8 sections = 18, so 144/2=72 to both sides
  // 0,6 - 1,4
  tft.fillRect(11, 211, 16, 5, tft.color565(0, 193, 200));
  tft.fillRect(29, 211, 16, 5, tft.color565(0, 168, 201));
  tft.fillRect(47, 211, 16, 5, tft.color565(0, 134, 201));
  tft.fillRect(65, 211, 16, 5, tft.color565(0, 101, 201));
  tft.fillRect(83, 211, 16, 5, tft.color565(0, 67, 201));
  tft.fillRect(101, 211, 16, 5, tft.color565(0, 34, 201));
  tft.fillRect(119, 211, 16, 5, tft.color565(0, 0, 201));
  tft.fillRect(137, 211, 16, 5, tft.color565(64, 0, 201));


  tft.drawRect(10, 210, 144, 7, ILI9341_DARKGREY);
  //Lambda Bar Markings
  for (int i = 0; i <= 8; i++) {
    int drawpos = i * 18;
    drawpos = drawpos + 9;
    tft.fillRect(drawpos, 211, 2, 5, ILI9341_LIGHTGREY);
  }
  tft.fillRect(81, 205, 2, 16, ILI9341_WHITE);

  //Top the actual, bottom the target Lambda:  0.5 - 1.5 shown:
  int actLamb;
  int tarLamb;
  if (DemoMode) {
    actLamb = (1.0 - (screen2var / 100.0)) * -360.0;
    tarLamb = (1.0 - (2.0 - (screen2var / 100.0))) * -360.0;
  } else {
    actLamb = (1.0 - emucan.emu_data.wboLambda) * -360.0;
    tarLamb = (1.0 - emucan.emu_data.lambdaTarget) * -360.0;
  }
  actLamb = constrain(actLamb, -72, 72) + 82;
  tft.fillTriangle(actLamb - 3, 195, actLamb + 3, 195, actLamb, 210, ILI9341_GREEN);
  tarLamb = constrain(tarLamb, -72, 72) + 82;
  tft.fillTriangle(tarLamb - 3, 230, tarLamb + 3, 230, tarLamb, 215, ILI9341_YELLOW);


  //MAP segment font:
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(&segment20pt7b);
  String num2show = String(screen2var);
  int numoffset;
  numoffset = 17.5 * (4 - num2show.length());
  tft.setCursor(35 + numoffset, 120);
  tft.print(num2show);

  //RPM
  num2show = String(rpms);
  numoffset = 17.5 * (4 - num2show.length());
  tft.setCursor(45 + numoffset, 165);
  tft.print(num2show);

  //EGT1
  if (DemoMode) {
    num2show = String(second() * 20);
  } else {
    unsigned int avgegt;
    avgegt = (emucan.emu_data.Egt1 + emucan.emu_data.Egt2) / 2;
    num2show = String(avgegt);
  }
  numoffset = 17.5 * (4 - num2show.length());
  tft.setCursor(240 + numoffset, 10);
  if (num2show.toInt() < 400) {
    //Below 400°C
    tft.setTextColor(ILI9341_CYAN);
  } else {
    if (num2show.toInt() < 900) {
      //Between 401-900
      tft.setTextColor(ILI9341_GREENYELLOW);
    } else {
      //Above 901
      tft.setTextColor(ILI9341_RED);
    }
  }
  tft.print(num2show);


  //CLT
  if (DemoMode) {
    num2show = num2show = String((second() * 2) - 10);
  } else {
    num2show = String(emucan.emu_data.CLT);
  }
  numoffset = 17.5 * (3 - num2show.length());
  tft.setCursor(255 + numoffset, 80);
  if (num2show.toInt() < 60) {
    //Below 60°C
    tft.setTextColor(ILI9341_CYAN);
  } else {
    if (num2show.toInt() < 100) {
      //Between 61-100
      tft.setTextColor(ILI9341_GREENYELLOW);
    } else {
      //Above 101
      tft.setTextColor(ILI9341_RED);
    }
  }
  tft.print(num2show);

  //IAT
  if (DemoMode) {
    num2show = num2show = String((second() * 2) - 10);
  } else {
    num2show = String(emucan.emu_data.IAT);
  }
  numoffset = 17.5 * (3 - num2show.length());
  tft.setCursor(180 + numoffset, 80);
  if (num2show.toInt() < 10) {
    //Below 10°C
    tft.setTextColor(ILI9341_CYAN);
  } else {
    if (num2show.toInt() < 50) {
      //Between 11-50
      tft.setTextColor(ILI9341_GREENYELLOW);
    } else {
      //Above 51
      tft.setTextColor(ILI9341_RED);
    }
  }
  tft.print(num2show);

  //Oil Temp
  if (DemoMode) {
    num2show = num2show = String((second() * 2));
  } else {
    num2show = String(emucan.emu_data.oilTemperature);
  }
  numoffset = 17.5 * (3 - num2show.length());
  tft.setCursor(255 + numoffset, 45);
  if (num2show.toInt() < 70) {
    //Below 70°C
    tft.setTextColor(ILI9341_CYAN);
  } else {
    if (num2show.toInt() < 110) {
      //Between 71-110
      tft.setTextColor(ILI9341_GREENYELLOW);
    } else {
      //Above 110
      tft.setTextColor(ILI9341_RED);
    }
  }
  tft.print(num2show);


  //Oil Pressure
  if (DemoMode) {
    num2show = num2show = String(second() / 10.0, 1);
  } else {
    num2show = String(emucan.emu_data.oilPressure, 1);
  }
  numoffset = 17.5 * (3 - num2show.length());
  tft.setCursor(200 + numoffset, 45);
  if (num2show.toInt() < 1) {
    //Below 15 (1,5Bar)
    tft.setTextColor(ILI9341_RED);
  } else {
    if (num2show.toInt() < 8) {
      //Between 15-80
      tft.setTextColor(ILI9341_GREENYELLOW);
    } else {
      //Above 80
      tft.setTextColor(ILI9341_RED);
    }
  }
  tft.print(num2show);

  // EGT Text:
  tft.setFont(Arial_16);
  tft.setTextColor(ILI9341_LIGHTGREY);

  // EGT Icon:
  tft.writeRect(220, 2, 27, 25, (uint16_t*)egt_27x25);

  // EGT Difference:
  int diffegt;
  diffegt = max(emucan.emu_data.Egt1, emucan.emu_data.Egt2) - min(emucan.emu_data.Egt1, emucan.emu_data.Egt2);
  tft.setCursor(160, 10);
  tft.print("Ed: ");
  tft.print(diffegt);

  //CLT / IAT:
  //tft.setCursor(240, 80);
  //tft.print("C:");
  tft.writeRect(240, 75, 26, 22, (uint16_t*)clt26x22);
  //tft.setCursor(175, 80);
  //tft.print("I:");
  tft.writeRect(170, 75, 26, 25, (uint16_t*)iat_26x25);

  //OILP / OILT:
  //tft.setCursor(240, 45);
  //tft.print("O:");
  tft.writeRect(240, 40, 28, 21, (uint16_t*)oiltemp_28x21);
  //tft.setCursor(170, 45);
  //tft.print("O:");
  tft.writeRect(170, 40, 30, 24, (uint16_t*)oilp_30x24);

  //Ign Angle and Fuel:
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(175, 107);
  tft.print("S: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(emucan.emu_data.IgnAngle, 0);
  tft.setCursor(250, 107);
  tft.setTextColor(ILI9341_BLUE);
  tft.print("F: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(emucan.emu_data.pulseWidth, 1);

  //Pbar:
  int v;
  if (DemoMode) {
    v = map(mpu.getAccZ(), -1, 1, 0, 100);
  } else {
    v = emucan.emu_data.TPS;
  }
  pbar(v, 157, 70, 10, 100, false, ILI9341_DARKCYAN, ILI9341_CYAN);

  //Small Plot: (64px height, 256bit buffer/4)
  using index_t = decltype(bufferS)::index_t;
  for (index_t i = 0; i < bufferS.size(); i++) {
    tft.fillCircle(i + 170, 194 - (bufferS[i]) / 4, 1, ILI9341_GREENYELLOW);
  }
  using index_t = decltype(bufferS2)::index_t;
  for (index_t i = 0; i < bufferS2.size(); i++) {
    tft.fillCircle(i + 170, 194 - (bufferS2[i]) / 4, 1, ILI9341_MAGENTA);
  }
  // Box around:
  tft.drawRect(170, 130, 150, 65, ILI9341_DARKGREY);
  tft.drawRect(169, 129, 151, 67, ILI9341_LIGHTGREY);


  // Warning Sign
  if (DemoMode) {
    if (second() % 2) {
      tft.setFont(AwesomeF000_96);
      tft.setCursor(15, 30);
      tft.setTextColor(ILI9341_RED);
      tft.print(char(113));  // 113 = Warning https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128
    }
  } else {
    if (emucan.decodeCel()) {
      //Check Engine Light is on!
      if (second() % 2) {
        tft.setFont(AwesomeF000_96);
        tft.setCursor(15, 30);
        tft.setTextColor(ILI9341_RED);
        tft.print(char(113));  // 113 = Warning https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128
      }
    }
  }

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

  //GPS Info:
  tft.setFont(AwesomeF000_18);
  tft.setCursor(0, 0);
  if (gps.hdop.hdop() < 3.0) {
    tft.setTextColor(ILI9341_GREEN);
  }
  else {
    tft.setTextColor(ILI9341_ORANGE);
  }
  tft.print(char(18));  // 18 = Signal Bar https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128

  tft.setCursor(0, 30);
  tft.setFont(Arial_12);
  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.print("sat: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(gps.satellites.value());

  // Log Active:
  tft.setFont(AwesomeF000_12);
  tft.setCursor(30, 5);
  if (datalogactive) {
    tft.setTextColor(ILI9341_GREEN);
    tft.print(char(75));  // 75 = play, 76 = pause https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128
  }
  else {
    tft.setTextColor(ILI9341_DARKGREY);
    tft.print(char(76));  //  75 = play, 76 = pause https://hackaday.io/project/7330/gallery#db8ed33fc293ae63248b1d5737a3f128
  }

  //Heading:
  tft.setFont(Arial_16);
  tft.setTextColor(ILI9341_LIGHTGREY);
  String shead = String(round(gps.course.deg()));
  tft.setCursor(50 + (3 - shead.length()) * 10, (25 - 16));
  tft.print(shead);
  tft.setCursor(90, (25 - 16));
  tft.setTextColor(ILI9341_WHITE);
  tft.print(headingToText(gps.course.deg()));

  // Battery Volt from EMU on Bottom:
  //  tft.setFont(Arial_16);
  //  tft.setCursor(115, (25 - 16));
  //  tft.setCursor(0, 215);
  //  tft.setTextColor(ILI9341_LIGHTGREY);
  //  tft.print(emucan.emu_data.Batt, 1);
  //  tft.print("V");

  // Mini Scope from ringbuffer
  // MAP/RPM/TPS


  //Update the screen:
  tft.updateScreen();
}
