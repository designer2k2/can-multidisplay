// Bord Computer
// Follow ~ the VAG style with a Single journey memory, and a Total journey memory: http://www.audihelp.com/auda-11-on_board_computer.html

// Consumption:  Below 5kmh  = L/H,  else L/100km
// Distance: oldvalue + (time now - time old) * (speed / 3.6)

void screen5press(int X, int Y, int Z) {
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

void screen5setup() {
  intervals = 500;  // Update every 500ms
}

void screen5run() {
  intervals = 500;  // Update every 500ms
  //Led toggle:
  debugLEDtoggle();


  // Logo and Time:
  tft.fillScreen(ILI9341_BLACK);
  tft.writeRect(280, 200, 40, 39, (uint16_t*)Dlogominiature);
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
          trip_fuel_used, trip_distance, TimeToString(trip_time / 1000));
  tft.println(sz);

  sprintf(sz, "Fuel now: %3.2fL, Average: %3.2f, Gear: %1u",
          trip_fuel_now, trip_fuel_average, emucan.emu_data.gear);
  tft.println(sz);


  tft.updateScreen();
}

// t is time in seconds = millis()/1000;
char * TimeToString(unsigned long t)
{
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
    board_computer_calc();
    threads.delay(500);
    threads.yield();
  }
}

// Run this at a static intervall:
void board_computer_calc() {
  //Timestamp:
  unsigned long this_time = millis();
  uint16_t this_speed = emucan.emu_data.vssSpeed;

  //Timediff since last run:
  unsigned long time_diff;
  time_diff = this_time - trip_distance_last;
  trip_distance_last = this_time;

  //Distance with speed:
  float dist;
  dist = (time_diff * this_speed) / 3600.0; //Meter since last update
  trip_distance += (dist / 1000.0); //adding KM

  //Handle fuel used reset:
  trip_fuel_used =  fuel_used;

  //Trip Fuel usage average:
  trip_fuel_average = (100 / trip_distance) * trip_fuel_used;

  //Fuel current average:
  if (this_speed > 5) {
    if (trip_fuel_stationary == true) {
      trip_fuel_now = 0;
    }
    trip_fuel_stationary = false;
    trip_fuel_now = trip_fuel_now * 0.9 + (100.0 / this_speed  * fuel_usage) * 0.1;
  } else {
    if (trip_fuel_stationary == false) {
      trip_fuel_now = 0;
    }
    trip_fuel_stationary = true;
    trip_fuel_now = trip_fuel_now * 0.9 + fuel_usage * 0.1;
  }

  //Trip time:
  trip_time = this_time;
}
