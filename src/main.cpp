#include <Arduino.h>
#include <LiquidCrystal.h>

/////////////////////////////////////////////////////////////////////////
// ENUMS ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// TIME
enum SetTimeState {
  TIME_NONE,
  TIME_HOURS,
  TIME_MINUTES,
  TIME_SECONDS
};

// ALARM
enum SetAlarmState {
  ALARM_NONE,
  ALARM_HOURS,
  ALARM_MINUTES
};

// TOGGLE
enum SetToggleState {
  TOGGLE_NONE,
  TOGGLE_ALARM
};

// MENU
enum MenuState {
  MENU_NONE,
  MENU_TIME,
  MENU_ALARM,
  MENU_TOGGLE_ALARM
};

MenuState menuState = MENU_NONE;
SetTimeState setTimeState = TIME_NONE;
SetAlarmState setAlarmState = ALARM_NONE;
SetToggleState setToggleState = TOGGLE_NONE;

/////////////////////////////////////////////////////////////////////////
// VARIABLES ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// TIME
unsigned int time_hours = 12;
unsigned int time_minutes = 0;
unsigned int time_seconds = 0;
unsigned int temp_time_hours = 12;
unsigned int temp_time_minutes = 0;
unsigned int temp_time_seconds = 0;

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

String getTempTime() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%2d:%02d:%02d", temp_time_hours, temp_time_minutes, time_seconds);
  return String(buffer);
}

void updateTime() {
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastMillis >= 1000) {
    lastMillis = currentMillis;
    
    time_seconds++;
    if (time_seconds >= 60) {
      time_seconds = 0;
      time_minutes++;
      
      if (time_minutes >= 60) {
        time_minutes = 0;
        time_hours++;
        
        if (time_hours >= 24) {
          time_hours = 0;
        }
      }
    }
  }
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
  lcd.clear();
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
  lcd.print(getTempTime());
}

void displayAlarmMenu() {
  lcd.print("Menu: Alarme");
    lcd.setCursor(0, 1);

  if (menuState == MENU_ALARM){
    lcd.print(getAlarm());
  } 
  else {
    lcd.setCursor(1, 1);
    lcd.print("(");
    lcd.print(alarmStatus ? "ON" : "OFF");
    lcd.print(")");
  }
}

void displaySelectedMenu(MenuState menuState) {
  switch (menuState) {
    case MENU_NONE:
      displayBasicInfo();
      break;
    case MENU_TIME:
      displayTimeMenu();
      break;
    case MENU_ALARM:
      displayAlarmMenu();
      break;
    case MENU_TOGGLE_ALARM:
      displayAlarmMenu();
      break;
    default:
      break;
  }
}

// HELPER FUNCTIONS
void waitRelease(int pin) {
  while (digitalRead(pin) == LOW) {}
}

int wrapValues(int value, int limit, bool isIncrement) {
  if (isIncrement) {
    if (value < limit) {
      value++;
    } 
    else {
      value = 0;
    }
  } 
  else {
    if (value > 0) {
      value--;
    } 
    else {
      value = limit;
    }
  }
  return value;
}

// BUTTONS
void handleMenuButtonPress() {
  if (menuState != MENU_TOGGLE_ALARM) {
    menuState = static_cast<MenuState>(static_cast<int>(menuState) + 1);
  }
  else {
    menuState = MENU_NONE;
    setTimeState = TIME_NONE;
  }
  Serial.println("MenuState = " + String(menuState));
  lcd.clear();
  waitRelease(A2);
}

void handleSetButtonPress() {
  if (menuState == MENU_TIME) {
    if (setTimeState != TIME_SECONDS) {
        setTimeState = static_cast<SetTimeState>(static_cast<int>(setTimeState) + 1);
    } 
    else {
      time_hours = temp_time_hours;
      time_minutes = temp_time_minutes;

      setTimeState = TIME_NONE;
      menuState = MENU_NONE;
      
      lcd.clear();
      Serial.println("SET Menu > Time = " + String(menuState));
    }
    Serial.println("SET Time = " + String(setTimeState));
  }
  else if (menuState == MENU_ALARM) {
    if (setAlarmState != ALARM_MINUTES) {
      setAlarmState = static_cast<SetAlarmState>(static_cast<int>(setAlarmState) + 1);
    } 
    else {
      setAlarmState = ALARM_NONE;
      menuState = MENU_NONE;

      lcd.clear();
      Serial.println("SET Menu > Alarm = " + String(menuState));
    }
    Serial.println("SET Alarm = " + String(setAlarmState));
  }
  else if (menuState == MENU_TOGGLE_ALARM) {
    if (setToggleState != TOGGLE_ALARM) {
      setToggleState = static_cast<SetToggleState>(static_cast<int>(setToggleState) + 1);
    } 
    else {
      setToggleState = TOGGLE_NONE;
      menuState = MENU_NONE;
      Serial.println("SET Menu > Toggle = " + String(menuState));
    }
    lcd.clear();
    Serial.println("SET Toggle = " + String(setToggleState));
  }
  
  waitRelease(A3);
}

void handlePlusButtonPress() {
  if (menuState == MENU_TIME || menuState == MENU_ALARM) {
    if (setTimeState == TIME_HOURS || setAlarmState == ALARM_HOURS) {
      if (setTimeState == TIME_HOURS) {
        temp_time_hours = wrapValues(temp_time_hours, 23, true);
      }
      else if (setAlarmState == ALARM_HOURS) {
        alarm_hourse = wrapValues(alarm_hourse, 23, true);
      }
    } 
    else if (setTimeState == TIME_MINUTES || setAlarmState == ALARM_MINUTES) {
      if (setTimeState == TIME_MINUTES) {
        temp_time_minutes = wrapValues(temp_time_minutes, 59, true);
      }
      else if (setAlarmState == ALARM_MINUTES) {
        alarm_minutes = wrapValues(alarm_minutes, 59, true);
      }
    } 
    else if (setTimeState == TIME_SECONDS) {
      time_seconds = 0;
      temp_time_seconds = 0;
    }
  }
  else if (menuState == MENU_TOGGLE_ALARM) {
    if (setToggleState == TOGGLE_ALARM) {
      toggleAlarm();
    }
  }
  waitRelease(A4);
}

void handleMinusButtonPress() {
   if (menuState == MENU_TIME || menuState == MENU_ALARM) {
    if (setTimeState == TIME_HOURS || setAlarmState == ALARM_HOURS) {
      if (setTimeState == TIME_HOURS) {
        temp_time_hours = wrapValues(temp_time_hours, 23, false);
      }
      else if (setAlarmState == ALARM_HOURS) {
        alarm_hourse = wrapValues(alarm_hourse, 23, false);
      }
    } 
    else if (setTimeState == TIME_MINUTES || setAlarmState == ALARM_MINUTES) {
      if (setTimeState == TIME_MINUTES) {
        temp_time_minutes = wrapValues(temp_time_minutes, 59, false);
      }
      else if (setAlarmState == ALARM_MINUTES) {
        alarm_minutes = wrapValues(alarm_minutes, 59, false);
      }
    } 
    else if (setTimeState == TIME_SECONDS) {
      time_seconds = 0;
      temp_time_seconds = 0;
    }
  }
  else if (menuState == MENU_TOGGLE_ALARM) {
    if (setToggleState == TOGGLE_ALARM) {
      toggleAlarm();
    }
  }
  waitRelease(A5);
}

/////////////////////////////////////////////////////////////////////////
// MAIN /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void update() {
  updateTime();
  updateTemperature();
}

void setup() {
  Serial.begin(115200);

  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  
  lcd.begin(16, 2);
  beginTemperature();
}

void loop() {
  int menuBtn = digitalRead(A2);
  int setBtn = digitalRead(A3);
  int plusBtn = digitalRead(A4);
  int minusBtn = digitalRead(A5);

  if (menuBtn == LOW) { handleMenuButtonPress(); }
  else if (setBtn == LOW) { handleSetButtonPress(); }
  else if (plusBtn == LOW) { handlePlusButtonPress(); }
  else if (minusBtn == LOW) { handleMinusButtonPress(); }
  
  displaySelectedMenu(menuState);
  lcd.home();
  update();
}
