

#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/**
  Connection
  Arduino    VoiceRecognitionModule
   2   ------->     TX
   3   ------->     RX
*/

LiquidCrystal_I2C lcd(0x20, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

VR myVR(2, 3);  // 2:RX 3:TX, you can choose your favourite pins.

uint8_t records[7];  // save record
uint8_t buf[64];

const int lock = 6;
const int unlock = 7;
const int enable = 5;
const int buzzer = 12;
const int buttonPin = 10;
const int reButtonPin = 4;
const int redLedPin = 11;
const int greenLedPin = 9;
const int yellowLedPin = 8;
int warnPin = A0;
int failNum = 0;


int buttonState = 0;
int reButtonState = 0;
int warnPinState = 0;

#define onRecord (0)
#define offRecord (1)
// #define reRecord (45)

void printSignature(uint8_t *buf, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (buf[i] > 0x19 && buf[i] < 0x7F) {
      Serial.write(buf[i]);
    } else {
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}


void printVR(uint8_t *buf) {
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if (buf[0] == 0xFF) {
    Serial.print("NONE");
  } else if (buf[0] & 0x80) {
    Serial.print("UG ");
    Serial.print(buf[0] & (~0x80), DEC);
  } else {
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if (buf[3] > 0) {
    printSignature(buf + 4, buf[3]);
  } else {
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}

void printInLCD(String wordsLine, int cols, int rows) {
  lcd.setCursor(cols, rows);
  lcd.print(wordsLine);
}

void setup() {
  /** initialize */
  myVR.begin(9600);
  Serial.begin(115200);
  lcd.init();       // initialize the lcd
  lcd.backlight();  // Turn on the LCD screen backlight

  boolean connectionFlag = false;
  boolean onRecordFlag = false;
  boolean offRecordFlag = false;

  lcd.clear();
  printInLCD("This is a voice", 1, 0);
  printInLCD("control lock", 1, 1);

  Serial.println("Elechouse Voice Recognition V3 Module\r\nControl LED sample");

  pinMode(lock, OUTPUT);
  pinMode(unlock, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(reButtonPin, INPUT_PULLUP);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(warnPin, INPUT);


  if (myVR.clear() == 0) {
    Serial.println("Recognizer cleared.");
    connectionFlag = true;
  } else {
    Serial.println("Not find VoiceRecognitionModule.");
    Serial.println("Please check connection and restart Arduino.");

    lcd.clear();
    printInLCD("Connect wrong!", 0, 0);
    printInLCD("Try reconnect", 0, 1);

    while (1)
      ;
  }

  if (myVR.load((uint8_t)onRecord) >= 0) {
    Serial.println("onRecord loaded");
    onRecordFlag = true;
  }

  if (myVR.load((uint8_t)offRecord) >= 0) {
    Serial.println("offRecord loaded");
    offRecordFlag = true;
  }

  lcd.clear();
  printInLCD("con", 0, 0);
  printInLCD("onRe", 4, 0);
  printInLCD("offRe", 10, 0);
  printInLCD("T", 1, 1);
  printInLCD("T", 6, 1);
  printInLCD("T", 12, 1);

  if (!onRecordFlag) {
    printInLCD("F", 6, 1);
  }
  if (!offRecordFlag) {
    printInLCD("F", 12, 1);
  }

}

void loop() {
  int ret;
  long stopTime;

  buttonState = digitalRead(buttonPin);
  digitalWrite(enable, HIGH);

  warnPinState = digitalRead(warnPin);

  if (warnPinState == HIGH) {
    failNum = 4;
  }

  reButtonState = digitalRead(reButtonPin);
  if (buttonState == LOW || reButtonState == LOW) {
    if (failNum < 3) {
      digitalWrite(yellowLedPin, HIGH);
      stopTime = millis() + 5000;

      while (1) {
        ret = myVR.recognize(buf, 50);
        buttonState = digitalRead(buttonPin);
        if (((millis() > stopTime && failNum < 3) || buttonState == HIGH)) {
          failNum = failNum + 1;
          Serial.println(failNum);
          Serial.println("********************");
          digitalWrite(yellowLedPin, LOW);
          digitalWrite(redLedPin, HIGH);
          lcd.clear();
          printInLCD("Time out", 0, 0);
          delay(1000);
          digitalWrite(redLedPin, LOW);
          break;
        }

        if (ret > 0) {
          switch (buf[1]) {
            case onRecord:
              digitalWrite(enable, HIGH);
              digitalWrite(lock, HIGH);

              lcd.clear();
              printInLCD("Have locked", 0, 0);

              digitalWrite(greenLedPin, HIGH);
              delay(1000);
              digitalWrite(greenLedPin, LOW);

              delay(500);
              digitalWrite(lock, LOW);
              failNum = 0;

              break;

            case offRecord:
              digitalWrite(enable, HIGH);
              digitalWrite(unlock, HIGH);

              lcd.clear();
              printInLCD("Have unlocked", 0, 0);

              digitalWrite(greenLedPin, HIGH);
              delay(1000);
              digitalWrite(greenLedPin, LOW);

              delay(500);
              digitalWrite(unlock, LOW);
              failNum = 0;

              break;

            default:
              Serial.println("Record function undefined");

              lcd.clear();
              printInLCD("Undefine wrong", 0, 0);
              break;
          }
          /** voice recognized */
          printVR(buf);
        }
      }
    } else {
      digitalWrite(buzzer, HIGH);
      digitalWrite(redLedPin, HIGH);
      digitalWrite(yellowLedPin, LOW);
      lcd.clear();
      printInLCD("Door Locked", 0, 0);
      reButtonState = digitalRead(reButtonPin);
      if (reButtonState == LOW) {
        failNum = 0;
        Serial.println("-------------------");

        Serial.println(failNum);
        lcd.clear();
        printInLCD("Try again", 0, 0);

        digitalWrite(buzzer, LOW);
        digitalWrite(redLedPin, LOW);
        delay(1000);
      }
    }
  }
}
