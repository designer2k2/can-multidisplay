// CAN Debug
// Shows all over CAN received things:

// BIN print: https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')


void screen4press(int X, int Y, int Z) {
  if (millis() > lasttouch + 500) {
    lasttouch = millis();
    if (X > 160) {
      if (Y > 120) {
        //switch screens
        ScreenSwitch();
      }
    }
  }
}

void screen4setup() {
  intervals = 500;  // Update every 500ms
}

void screen4run() {
  intervals = 500;  // Update every 500ms
  //Led toggle:
  debugLEDtoggle();


  // Temp Text on Display:
  tft.fillScreen(ILI9341_BLACK);
  tft.writeRect(280, 200, 40, 39, (uint16_t*)Dlogominiature);

  tft.setCursor(1, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_12);
  tft.setTextSize(1);

  digitalClockDisplay();
  tft.println("");

  if (emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
    tft.setTextColor(ILI9341_GREEN);
    tft.println("Communication with EMU working");
    tft.drawRect(0, 0, 320, 240, ILI9341_GREEN);
  } else {
    tft.setTextColor(ILI9341_RED);
    tft.println("No Data from EMU received");
    tft.drawRect(0, 0, 320, 240, ILI9341_RED);
  }

  tft.setTextColor(ILI9341_WHITE);

  char sz[50];
  sprintf(sz, "RPM: %5u MAP %3u TPS %3u BAT %2.1f",
          emucan.emu_data.RPM, emucan.emu_data.MAP, emucan.emu_data.TPS, emucan.emu_data.Batt);
  tft.println(sz);
  sprintf(sz, "VSS %3u IAT: %2i CLT %3i WBO %+.1f",
          emucan.emu_data.vssSpeed, emucan.emu_data.IAT, emucan.emu_data.CLT, emucan.emu_data.wboLambda);
  tft.println(sz);
  sprintf(sz, "EGT1 %4u EGT2 %4u INJ: %+.2f",
          emucan.emu_data.Egt1, emucan.emu_data.Egt2, emucan.emu_data.pulseWidth);
  tft.println(sz);
  sprintf(sz, "IgnA %+.1f LCorr %+.1f EMUT %2i",
          emucan.emu_data.IgnAngle, emucan.emu_data.LambdaCorrection, emucan.emu_data.emuTemp);
  tft.println(sz);
  sprintf(sz, "OilP %.1f OilT %3i FuelP %.1f",
          emucan.emu_data.oilPressure, emucan.emu_data.oilTemperature, emucan.emu_data.fuelPressure);
  tft.println(sz);
  sprintf(sz, "E Flag: " BYTE_TO_BINARY_PATTERN " Flags1 " BYTE_TO_BINARY_PATTERN "",
          BYTE_TO_BINARY(emucan.emu_data.cel), BYTE_TO_BINARY(emucan.emu_data.flags1));
  tft.println(sz);

  if (emucan.emu_data.flags1 & emucan.F_IDLE) {
    tft.println("Engine Idle active");
  }

  if (emucan.can_error_flag) {
    //CAN Error gets true on any warning / error from the CAN controller
    //The can_error_data contains detailed information.
    tft.println("CAN Error!");
    tft.print("FlexCAN State: "); tft.print((char*)emucan.can_error_data.state);
    if ( emucan.can_error_data.BIT1_ERR ) tft.print(", BIT1_ERR");
    if ( emucan.can_error_data.BIT0_ERR ) tft.print(", BIT0_ERR");
    if ( emucan.can_error_data.ACK_ERR ) tft.print(", ACK_ERR");
    if ( emucan.can_error_data.CRC_ERR ) tft.print(", CRC_ERR");
    if ( emucan.can_error_data.FRM_ERR ) tft.print(", FRM_ERR");
    if ( emucan.can_error_data.STF_ERR ) tft.print(", STF_ERR");
    if ( emucan.can_error_data.RX_WRN ) tft.printf(", RX_WRN: %d", emucan.can_error_data.RX_ERR_COUNTER);
    if ( emucan.can_error_data.TX_WRN ) tft.printf(", TX_WRN: %d", emucan.can_error_data.TX_ERR_COUNTER);
    tft.printf(", FLT_CONF: %s\n", (char*)emucan.can_error_data.FLT_CONF);
  } else {
    tft.println("No Can Error");
    tft.print("FlexCAN State: "); tft.print((char*)emucan.can_error_data.state);
  }


  if (emucan.over_run) {
    tft.println("Overrun on CAN Bus! Loosing Data!");
    tft.drawRect(0, 0, 320, 240, ILI9341_BLUE);
  }

  tft.updateScreen();
}
