void screen1press(int X, int Y, int Z) {
  if (millis() > lasttouch + 500) {
    lasttouch = millis();
    if (X > 160) {
      if (Y < 120) {
        datalogactive = !datalogactive;
      }
    }
    if (Y > 120) {
      //switch screens
      ScreenSwitch();
    }
  }
}


void screen1setup() {
  intervals = 500;  // Update every 500ms
}

void screen1run() {
  //Led toggle:
  intervals = 500;  // Update every 500ms
  debugLEDtoggle();

  //Fancy Temps:
  //    Serial.print("TMP75 C: = ");
  //    Serial.print(temperature.readTemperatureC());
  //    Serial.print(F(" MPU C: ")); Serial.print(mpu.getTemp());
  //    Serial.print(F(" Teensy C: ")); Serial.println(tempmonGetTemp());

  lm75temp = temperature.readTemperatureC();
  int16_t val_0 = ADS.readADC(0);
  int16_t val_1 = ADS.readADC(1);
  int16_t val_2 = ADS.readADC(2);
  int16_t val_3 = ADS.readADC(3);

  // Temp Text on Display:
  tft.fillScreen(ILI9341_BLACK);

  tft.writeRect(280, 200, 40, 39, (uint16_t*)Dlogominiature);

  // progressbar:
  int v = map(mpu.getAccX(), -1, 1, 0, 100);
  pbar(v, 180, 230, 100, 10, true, ILI9341_DARKCYAN, ILI9341_LIGHTGREY);
  v = map(mpu.getAccZ(), -1, 1, 0, 100);
  pbar(v, 310, 100, 10, 100, false, ILI9341_DARKCYAN, ILI9341_RED);

  tft.setCursor(1, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.setTextSize(1);

  tft.print("TMP75 C: = ");
  tft.print(lm75temp);
  tft.print(F(" MPU C: "));
  tft.print(mpu.getTemp());
  tft.print(F(" Teensy C: "));
  tft.println(tempmonGetTemp());

  // Indicate the datalogger
  if (datalogactive) {
    tft.fillCircle(290, 30, 10, ILI9341_RED);
  }

  tft.setCursor(1, 20);
  tft.print(F("TEMPERATURE: "));
  tft.println(mpu.getTemp());
  char sz[32];
  sprintf(sz, "Acel: X %+.2f Y %+.2f Z %+.2f",
          mpu.getAccX(), mpu.getAccY(), mpu.getAccZ());
  tft.println(sz);
  sprintf(sz, "Gyro: X %+.2f Y %+.2f Z %+.2f",
          mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ());
  tft.println(sz);

  tft.print(F("ACC ANGLE X: "));
  tft.print(mpu.getAccAngleX());
  tft.print("  Y: ");
  tft.println(mpu.getAccAngleY());

  tft.print(F("ANGLE     X: "));
  tft.print(mpu.getAngleX());
  tft.print("  Y: ");
  tft.print(mpu.getAngleY());
  tft.print("  Z: ");
  tft.println(mpu.getAngleZ());

  tft.println("");

  tft.print("Analog0: ");
  tft.print(val_0);
  tft.print(" Analog1: ");
  tft.println(val_1);
  tft.print("Analog2: ");
  tft.print(val_2);
  tft.print(" Analog3: ");
  tft.println(val_3);
  tft.print("ADS Error: ");
  tft.println(ADS.getError());

  tft.println("");

  tft.print("RTC: ");
  digitalClockDisplay();

  tft.println("");

  //GPS:
  tft.print("GPS: ");
  tft.print("char: ");
  tft.print(gps.charsProcessed());
  tft.print(" sntn: ");
  tft.print(gps.sentencesWithFix());
  tft.print(" fail: ");
  tft.print(gps.failedChecksum());
  tft.print(" valid: ");
  tft.println(gps.satellites.isValid());
  tft.print("sat: ");
  tft.print(gps.satellites.value());
  tft.print(" hdop: ");
  tft.print(gps.hdop.hdop());
  tft.print(" age: ");
  tft.print(gps.location.age());
  tft.print(" course: ");
  tft.println(gps.course.deg());
  tft.print("lat: ");
  tft.print(gps.location.lat(), 4);
  tft.print(" lon: ");
  tft.print(gps.location.lng(), 4);
  tft.print(" height: ");
  tft.print(gps.altitude.meters());
  tft.print(" speed: ");
  tft.println(gps.speed.kmph());

  if (gps.date.age() == gps.date.isValid()) {
    tft.print("********** ******** ");
  } else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d",
            gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    tft.print(sz);
  }
  tft.println("");

  tft.print("EMU RPM: ");
  tft.println(emucan.emu_data.RPM);
  if (emucan.can_error_flag) {
    //CAN Error gets true on any warning / error from the CAN controller
    //The can_error_data contains detailed information.
    tft.println("CAN Error!");
    tft.print("FlexCAN State: ");
    tft.print((char*)emucan.can_error_data.state);
    if (emucan.can_error_data.BIT1_ERR) tft.print(", BIT1_ERR");
    if (emucan.can_error_data.BIT0_ERR) tft.print(", BIT0_ERR");
    if (emucan.can_error_data.ACK_ERR) tft.print(", ACK_ERR");
    if (emucan.can_error_data.CRC_ERR) tft.print(", CRC_ERR");
    if (emucan.can_error_data.FRM_ERR) tft.print(", FRM_ERR");
    if (emucan.can_error_data.STF_ERR) tft.print(", STF_ERR");
    if (emucan.can_error_data.RX_WRN) tft.printf(", RX_WRN: %d", emucan.can_error_data.RX_ERR_COUNTER);
    if (emucan.can_error_data.TX_WRN) tft.printf(", TX_WRN: %d", emucan.can_error_data.TX_ERR_COUNTER);
    tft.printf(", FLT_CONF: %s\n", (char*)emucan.can_error_data.FLT_CONF);
  } else {
    tft.println("No Can Error");
    tft.print("FlexCAN State: ");
    tft.print((char*)emucan.can_error_data.state);
  }

  //lets try an arc:
  //drawArc(160, 120, 50, 10, 0, 180, ILI9341_WHITE);

  // Clock lines:
  for (int i = 0; i <= 3; i++) {
    drawArc(290, 30, 30, 5, i * 90 - 3, i * 90 + 3, ILI9341_DARKGREY);
  }
  for (int i = 0; i <= 12; i++) {
    drawArc(290, 30, 25, 3, i * 30 - 2, i * 30 + 2, ILI9341_DARKGREY);
  }
  int arcs = ((second() * 6) + 270);
  drawArc(290, 30, 30, 10, arcs - 4, arcs + 4, ILI9341_LIGHTGREY);
  drawArc(290, 30, 20, 2, 0, 360, ILI9341_BLUE);
  drawArc(290, 30, 30, 1, 135, 45, ILI9341_GREEN);

  //7 segment font test:
  tft.setTextColor(ILI9341_YELLOW);
  tft.setFont(&segment20pt7b);
  tft.setCursor(220, 70);
  tft.println("1234");

  //Update the screen:
  tft.updateScreen();
}
