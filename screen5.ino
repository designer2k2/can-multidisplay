// (Trip) Board Computer
// Follow ~ the VAG style with a Single journey memory, and a Total journey memory: http://www.audihelp.com/auda-11-on_board_computer.html

// Consumption:  Below 5kmh  = L/H,  else L/100km
// Distance: oldvalue + (time now - time old) * (speed / 3.6)

void screen5press(int X, int Y, int Z) {
  if (millis() > lasttouch + 500) {
    lasttouch = millis();
    if (X > 160) {
      if (Y > 120) {
        //switch screens  lower right
        ScreenSwitch();
      } else {
        // Uper right:
        board_computer_reset(&trip1);
      }
    } else {
      if (Y > 120) {
        //lower left:
        board_computer_restore(&trip1, 1);
      } else {
        // Uper left:
        board_computer_save(&trip1, 1);
      }
    }
  }
}

void screen5setup() {
  intervals = 500;  // Update every 500ms
}

void screen5run() {
  intervals = 500;  // Update every 500ms
  //Led toggle:
  debugLEDtoggle();


  // Logo and Time:
  tft.fillScreen(ILI9341_BLACK);
  tft.writeRect(280, 200, 40, 39, (uint16_t *)Dlogominiature);
  //Date and Time from RTC:
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_8);
  tft.setTextSize(1);
  tft.setCursor(220, 230);
  char sz[50];
  sprintf(sz, "%02d.%02d.%02d",
          day(), month(), year());
  tft.print(sz);
  sprintf(sz, "%02d:%02d:%02d",
          hour(), minute(), second());
  tft.setFont(Arial_16);
  tft.setTextSize(1);
  tft.setCursor(190, 210);
  tft.print(sz);


  // Header:
  tft.setCursor(1, 10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(Arial_12);
  tft.setTextSize(1);
  tft.println("Board Computer:");



  tft.setTextColor(ILI9341_WHITE);


  sprintf(sz, "Fuel consumed: %2.2fL, Distance: %3.2fkm\nTime: %s",
          trip1.trip_fuel_used, trip1.trip_distance, TimeToString(trip1.trip_time / 1000));
  tft.println(sz);

  sprintf(sz, "Fuel now: %3.2fL, Average: %3.2f, Gear: %1u",
          trip1.trip_fuel_now, trip1.trip_fuel_average, emucan.emu_data.gear);
  tft.println(sz);

  sprintf(sz, "Last Store: [%02d/%02d/%02d][%02d:%02d:%02d]",
          (year(trip1.trip_last_saved) - 2000), month(trip1.trip_last_saved), day(trip1.trip_last_saved), hour(trip1.trip_last_saved), minute(trip1.trip_last_saved), second(trip1.trip_last_saved));
  tft.println(sz);





  // Show CEL Infos if present:
  if (emucan.decodeCel()) {
    //Show CEL Details:
    tft.setCursor(1, 180);
    //The can_error_data contains detailed information.
    tft.println("CEL!");
    if (emucan.emu_data.cel & emucan.ERR_CLT) tft.print("CLT ");
    if (emucan.emu_data.cel & emucan.ERR_IAT) tft.print("IAT ");
    if (emucan.emu_data.cel & emucan.ERR_MAP) tft.print("MAP ");
    if (emucan.emu_data.cel & emucan.ERR_WBO) tft.print("WBO ");
    if (emucan.emu_data.cel & emucan.ERR_EGT1) tft.print("EGT1 ");
    if (emucan.emu_data.cel & emucan.ERR_EGT2) tft.print("EGT2 ");
    if (emucan.emu_data.cel & emucan.EGT_ALARM) tft.print("KNOCK");
  }


  tft.updateScreen();
}

// t is time in seconds = millis()/1000;
char *TimeToString(unsigned long t) {
  static char str[12];
  long h = t / 3600;
  t = t % 3600;
  int m = t / 60;
  int s = t % 60;
  sprintf(str, "%04ld:%02d:%02d", h, m, s);
  return str;
}

// Board Computer Thread runs ~ 500ms
void board_computer_thread() {
  while (1) {
    board_computer_calc(&trip1);
    board_computer_autosave(&trip1, 1);
    //board_computer_calc(&trip2);
    threads.delay(500);
    threads.yield();
  }
}

// Board Computer Autosave
// * VSS goes below 2 kmh, but only if VSS was above 5 before (hysteresis)
// * Last save is more than 5 minutes ago
void board_computer_autosave(struct trip_data *tripdata, int storage_slot) {

  //If speed goes below 2kmh and was above 5 before:
  if (emucan.EMUcan_Status == EMUcan_RECEIVED_WITHIN_LAST_SECOND) {
    if (emucan.emu_data.vssSpeed > 5) {
      trip_runtime1.aboveVSS = true;
    } else {
      if (emucan.emu_data.vssSpeed < 2) {
        if (trip_runtime1.aboveVSS = true) {
          trip_runtime1.aboveVSS = false;
          //Below threshold and hysteresis ok:
          board_computer_save(tripdata, storage_slot);
          trip_runtime1.trip_save_time = millis();
        }
      }
    }
  }

  //If last save is more than 5 minutes ago:
  if ((millis() - trip_runtime1.trip_save_time) >= (5 * 60 * 1000)) {
    trip_runtime1.trip_save_time = millis();
    board_computer_save(tripdata, storage_slot);
  }
}

// Run this at a static interval:
void board_computer_calc(struct trip_data *tripdata) {
  //Timestamp:
  unsigned long this_time = millis();
  uint16_t this_speed = emucan.emu_data.vssSpeed;

  //Timediff since last run:
  unsigned long time_diff;
  time_diff = this_time - tripdata->trip_distance_last;
  tripdata->trip_distance_last = this_time;

  //Distance with speed:
  float dist;
  dist = ((time_diff * this_speed) / 3600.0) / 1000.0;  //Meter since last update
  tripdata->trip_distance += dist;

  //Handle fuel used reset:
  tripdata->trip_fuel_used = tripdata->fuel_offset + fuel_used - trip_runtime1.trip_save_fuel;

  //Trip time:
  tripdata->trip_time = tripdata->trip_time_offset + this_time - trip_runtime1.trip_save_time;

  //Trip Fuel usage average:
  tripdata->trip_fuel_average = (100 / tripdata->trip_distance) * tripdata->trip_fuel_used;

  //Fuel current average:
  if (this_speed > 5) {
    if (tripdata->trip_fuel_stationary == true) {
      tripdata->trip_fuel_now = 0;
    }
    tripdata->trip_fuel_stationary = false;
    tripdata->trip_fuel_now = tripdata->trip_fuel_now * 0.9 + (100.0 / this_speed * fuel_usage) * 0.1;
  } else {
    if (tripdata->trip_fuel_stationary == false) {
      tripdata->trip_fuel_now = 0;
    }
    tripdata->trip_fuel_stationary = true;
    tripdata->trip_fuel_now = tripdata->trip_fuel_now * 0.9 + fuel_usage * 0.1;
  }
}

// Call this to reset a trip:
void board_computer_reset(struct trip_data *tripdata) {
  Serial.println("Trip wiped");
  tripdata->trip_distance_last = 0;
  tripdata->trip_distance = 0;
  tripdata->fuel_offset = 0;
  tripdata->trip_fuel_average = 0;
  tripdata->trip_fuel_now = 0;
  tripdata->trip_time = 0;
  tripdata->distance_offset = 0;
  tripdata->trip_fuel_used = 0;
  tripdata->trip_fuel_stationary = true;
  tripdata->trip_last_saved = 0;
  tripdata->trip_time_offset = millis() * -1;
  trip_runtime1.trip_save_time = 0;
  trip_runtime1.trip_save_fuel = 0;
}

// Call this to save a trip to a given slot:
void board_computer_save(struct trip_data *tripdata, int storage_slot) {
  tripdata->trip_last_saved = now();
  tripdata->trip_time_offset = tripdata->trip_time;
  tripdata->fuel_offset = tripdata->trip_fuel_used;
  tripdata->distance_offset = tripdata->trip_distance;

  //and for this run:
  trip_runtime1.trip_save_time = millis();
  trip_runtime1.trip_save_fuel = tripdata->trip_fuel_used;

  //Starting with position 100 (keep the lower ones for other things
  unsigned int eeAddress = 100 + 50 * (storage_slot - 1);
  EEPROM.put(eeAddress, *tripdata);
  Serial.print("Trip stored, Size of Tripdata:");
  Serial.println(sizeof(*tripdata));
  Serial.print("Time:");
  Serial.println(tripdata->trip_time_offset);
  Serial.println(tripdata->trip_last_saved);
}

// Call this to restore a trip from a given slot:
void board_computer_restore(struct trip_data *tripdata, int storage_slot) {
  // Read it:
  unsigned int eeAddress = 100 + 50 * (storage_slot - 1);
  EEPROM.get(eeAddress, *tripdata);
  //Restore the distance from the offset:
  tripdata->trip_distance = tripdata->distance_offset;
  Serial.print("Trip restored:");
  Serial.println(tripdata->trip_time_offset);
}
