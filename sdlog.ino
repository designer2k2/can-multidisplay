//SD Logging, run this in a intervall to get daily logfiles.

//SD log entry point:
void datalogtask() {
  if (DebugPrint) {
    char timeStamp[25];
    sprintf(timeStamp, "[%02d/%02d/%02d][%02d:%02d:%02d.%03ld],",
            (year() - 2000), month(), day(), hour(), minute(), second(), (millis() - 300) % 1000);
    Serial.print(timeStamp);
    Serial.println("Log will Run");
  }
  datalogtask2();
}

void datalogtask2() {

  //This here builds the string to log, then write it to the SD

  // The millis rollover not in sync with the seconds,  so substract 300ms:
  char timeStamp[25];
  sprintf(timeStamp, "[%02d/%02d/%02d],%05d.%03ld,",
          (year() - 2000), month(), day(), hour() * minute() * second(), (millis() - 300) % 1000);

  String dataString = "";

  dataString += timeStamp;

  //Temperature Sensor Data:
  //dataString += String(lm75temp, 1);  //28.08.2021 remove LM75, mpu is enough
  dataString += "0,";
  dataString += String(mpu.getTemp(), 1);
  dataString += ",";
  dataString += String(tempmonGetTemp(), 1);
  dataString += ",";

  //MPU Data:
  dataString += String(mpu.getAccX());
  dataString += ",";
  dataString += String(mpu.getAccY());
  dataString += ",";
  dataString += String(mpu.getAccZ());
  dataString += ",";

  dataString += String(mpu.getGyroX());
  dataString += ",";
  dataString += String(mpu.getGyroY());
  dataString += ",";
  dataString += String(mpu.getGyroZ());
  dataString += ",";

  //GPS Data:
  dataString += String(gps.satellites.value());
  dataString += ",";
  dataString += String(gps.location.lat(), 5);
  dataString += ",";
  dataString += String(gps.location.lng(), 5);
  dataString += ",";
  dataString += String(gps.altitude.meters(), 1);
  dataString += ",";
  dataString += String(gps.speed.kmph(), 1);
  dataString += ",";
  dataString += String(gps.course.deg(), 0);
  dataString += ",";
  dataString += String(gps.hdop.hdop());
  dataString += ",";
  dataString += String(gps.time.age());

  //Emu Data:
  dataString += ",";
  dataString += String(emucan.emu_data.RPM);
  dataString += ",";
  dataString += String(emucan.emu_data.MAP);
  dataString += ",";
  dataString += String(emucan.emu_data.TPS);
  dataString += ",";
  dataString += String(emucan.emu_data.IAT);
  dataString += ",";
  dataString += String(emucan.emu_data.CLT);
  dataString += ",";
  dataString += String(emucan.emu_data.wboLambda);
  dataString += ",";
  dataString += String(emucan.emu_data.pulseWidth);
  dataString += ",";
  dataString += String(emucan.emu_data.Egt1);
  dataString += ",";
  dataString += String(emucan.emu_data.Egt2);
  dataString += ",";
  dataString += String(emucan.emu_data.Batt);
  dataString += ",";
  dataString += String(emucan.emu_data.Baro);
  dataString += ",";
  dataString += String(emucan.emu_data.IgnAngle, 0);
  dataString += ",";
  dataString += String(emucan.emu_data.LambdaCorrection, 0);
  dataString += ",";
  dataString += String(emucan.emu_data.gear);
  dataString += ",";
  dataString += String(emucan.emu_data.pwm1);
  dataString += ",";
  dataString += String(emucan.emu_data.vssSpeed);
  dataString += ",";
  dataString += String(emucan.emu_data.emuTemp);
  dataString += ",";
  dataString += String(emucan.emu_data.cel);
  dataString += ",";
  dataString += String(emucan.emu_data.flags1);
  dataString += ",";
  dataString += String(emucan.emu_data.lambdaTarget);
  dataString += ",";
  dataString += String(emucan.emu_data.oilTemperature);
  dataString += ",";
  dataString += String(emucan.emu_data.oilPressure);
  dataString += ",";
  dataString += String(fuel_used);
  dataString += ",";
  dataString += String(emucan.can_error_data.state[1]); //Only first character

  //Create the filename: its LYYMMDD.TXT
  char filename[15];
  sprintf(filename, "L%02d%02d%02d.TXT",
          (year() - 2000), month(), day());

  SdFile::dateTimeCallback(dateTime);
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    if (dataFile.size() < 1) {
      // Filesize is below 1, its new, so write a header instead:
      Serial.print("Generate Header for New Logfile:");
      Serial.println(filename);
      String headString = "Date,Time,BoardTemp,MPUTemp,TeensyTemp,AccX,AccY,AccZ,GyroX,GyroY,GyroZ,";
      dataFile.print(headString);
      headString = "Sats,Lat,Long,Meter,Kmh,Heading,Hdop,Age,RPM,MAP,TPS,IAT,CLT,Lambda,";
      dataFile.print(headString);
      headString = "PulseWidth,EGT1,EGT2,Batt,Baro,IgnAngle,LambdaCorr,Gear,Pwm1,Vss,EmuTemp,";
      dataFile.print(headString);
      headString = "CEL,FLAGS1,LambdaTarget,OilTemp,OilPress,FuelUsed,CanState";
      dataFile.println(headString);
    }
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    //Serial.println(dataString);
    //b = millis();
    //Serial.print("Write took: [ms] "); Serial.println(b - a);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

}

void dateTime(uint16_t* date, uint16_t* time) {
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year(), month(), day());
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(), minute(), second());
}
