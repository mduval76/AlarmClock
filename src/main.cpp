#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define MAGIC_VALUE 0x12345678

/////////////////////////////////////////////////////////////////////////
// BUZZER ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/* 
  Take on me
  Connect a piezo buzzer or speaker to pin 11 or select a new pin.
  More songs available at https://github.com/robsoncouto/arduino-songs                                            
                                              
                                              Robson Couto, 2019
*/
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
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

/////////////////////////////////////////////////////////////////////////
// STRUCTS //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

struct Settings {
  unsigned int alarm_hours;
  unsigned int alarm_minutes;
  bool alarmStatus;
  uint32_t magic;
};

Settings settings;

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
bool alarmPlaying = false;
bool alarmStatus = true;
unsigned int alarm_hours = 6;
unsigned int alarm_minutes = 0;
unsigned long alarmStartTime = 0;
unsigned long lastNoteTime = 0;

// DISPLAY
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/////////////////////////////////////////////////////////////////////////
// FUNCTIONS ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

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
  } 
  else {
    alarm_hours = 12;
    alarm_minutes = 1;
    alarmStatus = true;
    Serial.println("No prior settings found. Setting defaults.");
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

  if (currentTime - lastNoteTime >= noteDuration) {
    lastNoteTime = currentTime;
    noTone(buzzer);

    divider = melody[currentNote + 1];
    if (divider > 0) {
      noteDuration = (wholenote) / divider;
    }
    else if (divider < 0) {
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5;
    }

    tone(buzzer, melody[currentNote], noteDuration * 0.9);

    currentNote += 2;
    if (currentNote >= notes * 2) {
      currentNote = 0;
    }
  }

  // for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
  //   divider = melody[thisNote + 1];
  //   if (divider > 0) {
  //     noteDuration = (wholenote) / divider;
  //   } else if (divider < 0) {
  //     noteDuration = (wholenote) / abs(divider);
  //     noteDuration *= 1.5;
  //   }
  //   tone(buzzer, melody[thisNote], noteDuration * 0.9);
  //   delay(noteDuration);
  //   noTone(buzzer);
  // }
}

void updateAlarm() {
  if (time_hours == alarm_hours && time_minutes == alarm_minutes && alarmStatus && !alarmPlaying) {
    soundAlarm();
  }

  if (alarmPlaying) {
    soundAlarm();
  }
}

String getAlarm() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%2d:%02d", alarm_hours, alarm_minutes);
  return String(buffer);
}

void setAlarm(unsigned int hours, unsigned int minutes) {
  alarm_hours = hours;
  alarm_minutes = minutes;
  saveSettings();
}

void toggleAlarm() {
  lcd.clear();
  alarmStatus = !alarmStatus;
  saveSettings(); 
}

void stopAlarm() {
  noTone(buzzer);
  alarmPlaying = false;
  currentNote = 0;
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

// HELPER FUNCTIONS /////////////////////////////////////////////////////
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

// BUTTONS //////////////////////////////////////////////////////////////
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
        alarm_hours = wrapValues(alarm_hours, 23, true);
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
        alarm_hours = wrapValues(alarm_hours, 23, false);
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
  updateAlarm();
}

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);

  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);

  loadSettings();
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
