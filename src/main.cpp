#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define MAGIC_VALUE 0x12345678

// ALARM //////////////////////////////////////////////////////////////
/* 
  Take on me
  Connect a piezo buzzer or speaker to pin 11 or select a new pin.
  More songs available at https://github.com/robsoncouto/arduino-songs                                            
                                              
                                              Robson Couto, 2019
*/

#define NOTE_B4  494
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_FS5 740
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_B5  988
#define REST      0

int tempo = 140;
int buzzer = 8;
int melody[] = {

  // Take on me, by A-ha
  // Score available at https://musescore.com/user/27103612/scores/4834399
  // Arranged by Edward Truong
  NOTE_FS5,8, NOTE_FS5,8,NOTE_D5,8, NOTE_B4,8, REST,8, NOTE_B4,8, REST,8, NOTE_E5,8, 
  REST,8, NOTE_E5,8, REST,8, NOTE_E5,8, NOTE_GS5,8, NOTE_GS5,8, NOTE_A5,8, NOTE_B5,8,
  NOTE_A5,8, NOTE_A5,8, NOTE_A5,8, NOTE_E5,8, REST,8, NOTE_D5,8, REST,8, NOTE_FS5,8, 
  REST,8, NOTE_FS5,8, REST,8, NOTE_FS5,8, NOTE_E5,8, NOTE_E5,8, NOTE_FS5,8, NOTE_E5,8,
  NOTE_FS5,8, NOTE_FS5,8,NOTE_D5,8, NOTE_B4,8, REST,8, NOTE_B4,8, REST,8, NOTE_E5,8, 
  
  REST,8, NOTE_E5,8, REST,8, NOTE_E5,8, NOTE_GS5,8, NOTE_GS5,8, NOTE_A5,8, NOTE_B5,8,
  NOTE_A5,8, NOTE_A5,8, NOTE_A5,8, NOTE_E5,8, REST,8, NOTE_D5,8, REST,8, NOTE_FS5,8, 
  REST,8, NOTE_FS5,8, REST,8, NOTE_FS5,8, NOTE_E5,8, NOTE_E5,8, NOTE_FS5,8, NOTE_E5,8,
  NOTE_FS5,8, NOTE_FS5,8,NOTE_D5,8, NOTE_B4,8, REST,8, NOTE_B4,8, REST,8, NOTE_E5,8, 
  REST,8, NOTE_E5,8, REST,8, NOTE_E5,8, NOTE_GS5,8, NOTE_GS5,8, NOTE_A5,8, NOTE_B5,8,
  NOTE_A5,8, NOTE_A5,8, NOTE_A5,8, NOTE_E5,8, REST,8, NOTE_D5,8, REST,8, NOTE_FS5,8, 
  REST,8, NOTE_FS5,8, REST,8, NOTE_FS5,8, NOTE_E5,8, NOTE_E5,8, NOTE_FS5,8, NOTE_E5,8,
};

int currentNote = 0;
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;

// STRUCTS //////////////////////////////////////////////////////////////
struct Settings {
  bool alarmStatus;
  unsigned int alarm_hours;
  unsigned int alarm_minutes;
  unsigned int time_hours;
  unsigned int time_minutes;
  unsigned int time_seconds;
  uint32_t magic;
};
Settings settings;

// ENUMS ////////////////////////////////////////////////////////////////
enum SetTimeState {
  TIME_NONE,
  TIME_HOURS,
  TIME_MINUTES,
  TIME_SECONDS
};

enum SetAlarmState {
  ALARM_NONE,
  ALARM_HOURS,
  ALARM_MINUTES
};

enum SetToggleState {
  TOGGLE_NONE,
  TOGGLE_ALARM
};

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

// VARIABLES ////////////////////////////////////////////////////////////
// TIME
unsigned int time_hours = 12, time_minutes = 0, time_seconds = 0;
unsigned int temp_time_hours = 12, temp_time_minutes = 0, temp_time_seconds = 0;

// TEMPERATURE
bool firstReading = true;
double tempCelsius;
unsigned long lastUpdate = 0;

// ALARM
bool alarmPlaying = false;
bool alarmStatus = true;
bool isSnoozing = false;
bool ledState = false;
bool alarmStoppedManually = false;
unsigned int alarm_hours = 0, alarm_minutes = 0;
unsigned int temp_alarm_hours = 0, temp_alarm_minutes = 0;
unsigned long alarmStartTime = 0, alarmStopTime = 0, lastNoteTime = 0, lastSnoozeTime = 0, lastLedToggleTime = 0, alarmRestartDelay = 0;

// DISPLAY
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// FUNCTIONS ////////////////////////////////////////////////////////////
void waitRelease(int pin) {
  while (digitalRead(pin) == LOW) {}
}

int wrapValues(int value, int limit, bool isIncrement) {
  if (isIncrement) {
    if (value < limit) {
      value++;
    } else {
      value = 0;
    }
  } else {
    if (value > 0) {
      value--;
    } else {
      value = limit;
    }
  }
  return value;
}

// EEPROM ///////////////////////////////////////////////////////////////
void saveSettings() {
  settings.alarm_hours = alarm_hours;
  settings.alarm_minutes = alarm_minutes;
  settings.alarmStatus = alarmStatus;
  settings.magic = MAGIC_VALUE;
  EEPROM.put(0, settings);
}

void loadSettings() {
  EEPROM.get(0, settings);
  if (settings.magic == MAGIC_VALUE) {
    alarm_hours = settings.alarm_hours;
    alarm_minutes = settings.alarm_minutes;
    alarmStatus = settings.alarmStatus;
    Serial.println("Loading settings");
  } else {
    alarm_hours = 12;
    alarm_minutes = 1;
    alarmStatus = true;
    time_hours = 12;
    time_minutes = 0;
    time_seconds = 0;

    settings.alarm_hours = alarm_hours;
    settings.alarm_minutes = alarm_minutes;
    settings.alarmStatus = alarmStatus;
    settings.time_hours = time_hours;
    settings.time_minutes = time_minutes;
    settings.time_seconds = time_seconds;
    settings.magic = MAGIC_VALUE;
    EEPROM.put(0, settings);
    Serial.println("Setting defaults.");
  }
}

// TIME /////////////////////////////////////////////////////////////////
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

// TEMPERATURE //////////////////////////////////////////////////////////
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

// ALARM ////////////////////////////////////////////////////////////////
void soundAlarm() {
  if (!alarmPlaying) {
    currentNote = 0;
    alarmPlaying = true;
    lastNoteTime = millis();
  }
  unsigned long currentTime = millis();
  if (currentTime - lastNoteTime >= static_cast<unsigned long>(noteDuration)) {
    lastNoteTime = currentTime;
    noTone(buzzer);
    divider = melody[currentNote + 1];

    if (divider > 0) {
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5;
    }
    tone(buzzer, melody[currentNote], noteDuration * 0.9);
    currentNote += 2;

    if (currentNote >= notes * 2) {
      currentNote = 0;
    }
  }
}

void setAlarm(unsigned int hours, unsigned int minutes) {
  alarm_hours = hours;
  alarm_minutes = minutes;
  alarmStoppedManually = false;
  saveSettings();
}

void stopAlarm() {
  noTone(buzzer);
  isSnoozing = false;
  alarmPlaying = false;
  currentNote = 0;
  alarmStoppedManually = true;
  alarmRestartDelay = (60 - time_seconds) * 1000;
  alarmStopTime = millis();
  setAlarm(alarm_hours, alarm_minutes);
  Serial.println("Alarm stopped.");
  waitRelease(A4);
  waitRelease(A5);
}

void updateAlarm() {
  if (isSnoozing) {
    if (millis() - lastSnoozeTime >= 120000) {
      isSnoozing = false;
      currentNote = 0;
      alarmStoppedManually = false;
    }
  } else if (!alarmStoppedManually && (millis() - alarmStopTime >= alarmRestartDelay)) {
    if (time_hours == alarm_hours && time_minutes == alarm_minutes && alarmStatus && !alarmPlaying) {
      soundAlarm();
    }

    if (alarmPlaying) {
      soundAlarm();
    }
  }
}

String getAlarm() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%2d:%02d", alarm_hours, alarm_minutes);
  return String(buffer);
}

String getTempAlarm() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%2d:%02d", temp_alarm_hours, temp_alarm_minutes);
  return String(buffer);
}

void toggleAlarm() {
  lcd.clear();
  alarmStatus = !alarmStatus;
  alarmStoppedManually = false; 
  saveSettings(); 
}

void handleAlarmLED() {
  unsigned long currentTime = millis();
  if (alarmPlaying && !isSnoozing) {
    if (ledState && (currentTime - lastLedToggleTime >= 250)) {
      digitalWrite(13, LOW);
      ledState = false;
      lastLedToggleTime = currentTime;
    } else if (!ledState && (currentTime - lastLedToggleTime >= 500)) {
      digitalWrite(13, HIGH);
      ledState = true;
      lastLedToggleTime = currentTime;
    }
  } else {
    digitalWrite(13, LOW);
  }
}

// DISPLAY //////////////////////////////////////////////////////////////
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
    lcd.print(getTempAlarm());
  } else {
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
      lcd.print("You screwed up");
      break;
  }
}

// BUTTONS //////////////////////////////////////////////////////////////
void handleMenuButtonPress() {
  if (alarmPlaying && !isSnoozing) {
    isSnoozing = true;
    lastSnoozeTime = millis();
  } else {
    if (menuState != MENU_TOGGLE_ALARM) {
      menuState = static_cast<MenuState>(static_cast<int>(menuState) + 1);
    } else {
      menuState = MENU_NONE;
      setTimeState = TIME_NONE;
    }
    lcd.clear();
    Serial.println("MenuState = " + String(menuState));
  }

  if (menuState == MENU_ALARM) {
    temp_alarm_hours = alarm_hours;
    temp_alarm_minutes = alarm_minutes;
  }
  waitRelease(A2);
}

void handleSetButtonPress() {
  if (menuState == MENU_TIME) {
    if (setTimeState != TIME_SECONDS) {
      setTimeState = static_cast<SetTimeState>(static_cast<int>(setTimeState) + 1);
    } else {
      time_hours = temp_time_hours;
      time_minutes = temp_time_minutes;
      setTimeState = TIME_NONE;
      menuState = MENU_NONE;
      lcd.clear();
    }
  } else if (menuState == MENU_ALARM) {
    if (setAlarmState != ALARM_MINUTES) {
      setAlarmState = static_cast<SetAlarmState>(static_cast<int>(setAlarmState) + 1);
    } else {
      setAlarmState = ALARM_NONE;
      menuState = MENU_NONE;
      setAlarm(temp_alarm_hours, temp_alarm_minutes);
      lcd.clear();
    }
  } else if (menuState == MENU_TOGGLE_ALARM) {
    if (setToggleState != TOGGLE_ALARM) {
      setToggleState = static_cast<SetToggleState>(static_cast<int>(setToggleState) + 1);
    } else {
      setToggleState = TOGGLE_NONE;
      menuState = MENU_NONE;
    }
    lcd.clear();
  }
  waitRelease(A3);
}

void handlePlusButtonPress() {
  if (menuState == MENU_TIME || menuState == MENU_ALARM) {
    if (setTimeState == TIME_HOURS || setAlarmState == ALARM_HOURS) {
      if (setTimeState == TIME_HOURS) {
        temp_time_hours = wrapValues(temp_time_hours, 23, true);
      } else if (setAlarmState == ALARM_HOURS) {
        temp_alarm_hours = wrapValues(temp_alarm_hours, 23, true);
      }
    } else if (setTimeState == TIME_MINUTES || setAlarmState == ALARM_MINUTES) {
      if (setTimeState == TIME_MINUTES) {
        temp_time_minutes = wrapValues(temp_time_minutes, 59, true);
      } else if (setAlarmState == ALARM_MINUTES) {
        temp_alarm_minutes = wrapValues(temp_alarm_minutes, 59, true);
      }
    } else if (setTimeState == TIME_SECONDS) {
      time_seconds = 0;
      temp_time_seconds = 0;
    }
  } else if (menuState == MENU_TOGGLE_ALARM) {
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
      } else if (setAlarmState == ALARM_HOURS) {
        temp_alarm_hours = wrapValues(temp_alarm_hours, 23, false);
      }
    } else if (setTimeState == TIME_MINUTES || setAlarmState == ALARM_MINUTES) {
      if (setTimeState == TIME_MINUTES) {
        temp_time_minutes = wrapValues(temp_time_minutes, 59, false);
      } else if (setAlarmState == ALARM_MINUTES) {
        temp_alarm_minutes = wrapValues(temp_alarm_minutes, 59, false);
      }
    } else if (setTimeState == TIME_SECONDS) {
      time_seconds = 0;
      temp_time_seconds = 0;
    }
  } else if (menuState == MENU_TOGGLE_ALARM) {
    if (setToggleState == TOGGLE_ALARM) {
      toggleAlarm();
    }
  }
  waitRelease(A5);
}

// MAIN /////////////////////////////////////////////////////////////////
void update() {
  updateTime();
  updateTemperature();
  updateAlarm();
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  loadSettings();
  beginTemperature();
}

void loop() {
  int menuBtn = digitalRead(A2);
  int setBtn = digitalRead(A3);
  int plusBtn = digitalRead(A4);
  int minusBtn = digitalRead(A5);

  handleAlarmLED();
  
  if (menuBtn == LOW) { 
    handleMenuButtonPress(); 
  } else if (setBtn == LOW) { 
    handleSetButtonPress();
  } else if (plusBtn == LOW) {
    if (digitalRead(A5) == LOW) {
      stopAlarm();
      return;
    }
    handlePlusButtonPress();
  } else if (minusBtn == LOW) { 
    if (digitalRead(A4) == LOW) {
      stopAlarm();
      return;
    }
    handleMinusButtonPress();
  }
  displaySelectedMenu(menuState);
  lcd.home();
  update();
}
