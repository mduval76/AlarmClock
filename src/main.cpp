#include <Arduino.h>
#include <LiquidCrystal.h>

/////////////////////////////////////////////////////////////////////////
// ENUMS ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// MENU
enum MenuState {
  CLOSED,
  TIME,
  ALARM,
  TOGGLE_ALARM
};

MenuState menuState = CLOSED;

/////////////////////////////////////////////////////////////////////////
// VARIABLES ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// TIME
unsigned int time_hours = 12;
unsigned int time_minutes = 0;
unsigned int time_seconds = 0;

// TEMPERATURE
bool firstReading = true;
double tempCelsius;
unsigned long lastUpdate = 0;

// ALARM
bool alarmStatus = true;
unsigned int alarm_hourse = 6;
unsigned int alarm_minutes = 0;

// DISPLAY
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/////////////////////////////////////////////////////////////////////////
// FUNCTIONS ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// TIME
String getTime() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%2d:%02d:%02d", time_hours, time_minutes, time_seconds);
  return String(buffer);
}

void updateTime() {
  time_seconds = (millis() / 1000);
  time_minutes = (time_seconds / 60);
  time_hours += (time_minutes / 60);

  time_seconds %= 60;
  time_minutes %= 60;
}

// TEMPERATURE
void updateTemperature() {
  if (millis() - lastUpdate < 1000 && !firstReading) {
    return;
  }

  int value = analogRead(A0);
  double voltage = value * (5.0 / 1023.0);
  double resistance = 10000.0 * voltage / (5.0 - voltage);
  double tempKelvin = 1.0 / (1.0 / 298.15 + 1.0 / 3950.0 * log(resistance / 10000.0));
  tempCelsius = tempKelvin - 273.15;
  lastUpdate = millis();
}

double getTemperature() {
    return tempCelsius;
}

void beginTemperature() {
    updateTemperature();
    firstReading = false;
}

// ALARM
void updateAlarm() {
  if (time_hours == alarm_hourse && time_minutes == alarm_minutes && alarmStatus) {
    // START BUZZER
  }
}

String getAlarm() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%2d:%02d", alarm_hourse, alarm_minutes);
  return String(buffer);
}

void setAlarm(unsigned int hours, unsigned int minutes) {
  alarm_hourse = hours;
  alarm_minutes = minutes;
}

void toggleAlarm() {
  alarmStatus = !alarmStatus;
}

// DISPLAY

void displayBasicInfo() {
  lcd.print(getTime());
  lcd.setCursor(10, 0);
  lcd.print(getTemperature(), 1);
  lcd.setCursor(0, 1);
  lcd.print(getAlarm());
  lcd.setCursor(6, 1);
  lcd.print(alarmStatus ? "(ON)" : "(OFF)");
}

void displayTimeMenu() {
  lcd.print("Menu: Heure");
  lcd.setCursor(0, 1);
  lcd.print(getTime());
}

void displayAlarmMenu() {
  lcd.print("Menu: Alarme");
    lcd.setCursor(0, 1);

  if (menuState == ALARM){
    lcd.print(getAlarm());
  } 
  else {
    lcd.setCursor(1, 1);
    lcd.print(alarmStatus ? "(ON)" : "(OFF)");
  }
}

void displaySelectedMenu(MenuState menuState) {
  switch (menuState) {
    case CLOSED:
      displayBasicInfo();
      break;
    case TIME:
      displayTimeMenu();
      break;
    case ALARM:
      displayAlarmMenu();
      break;
    case TOGGLE_ALARM:
      displayAlarmMenu();
      break;
    default:
      displayBasicInfo();
      break;
  }
}

// HELPER FUNCTIONS
void waitRelease(int pin) {
  while (digitalRead(pin) == LOW) {}
}

/////////////////////////////////////////////////////////////////////////
// MAIN /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  
  lcd.begin(16, 2);
  beginTemperature();
}

void update() {
  updateTime();
  updateTemperature();
}

void loop() {
  int menuBtn = digitalRead(A2);
  int setBtn = digitalRead(A3);
  int plusBtn = digitalRead(A4);
  int minusBtn = digitalRead(A5);

  if (menuBtn == LOW) {
    if (menuState != TOGGLE_ALARM) {
      menuState = static_cast<MenuState>(static_cast<int>(menuState) + 1);
    }
    else {
      menuState = CLOSED;
    }
    lcd.clear();
  }

  displaySelectedMenu(menuState);
  lcd.home();
  update();

  if (menuBtn == LOW) { waitRelease(A2); }
  if (setBtn == LOW) { waitRelease(A3); }
  if (plusBtn == LOW) { waitRelease(A4); }
  if (minusBtn == LOW) { waitRelease(A5); }
}