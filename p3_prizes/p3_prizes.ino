#include <math.h>
#include <Wire.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int button = 7;

const String P1 = "P1:";
const String P2 = "P2:";
const String P3 = "P3:";
const String P4 = "P4:";
const String PRIX = "Su premio es el ";
const String NONE_LEFT = "No quedan premios";

const unsigned int debounce_time = 150;
unsigned long last_bounce = millis();

const int expander = 0x38;
byte current_led = 0x7;

byte prizes[4] = {1,2,5,10};

void setup() {
    randomSeed(analogRead(0));
    Wire.begin();
    Serial.begin(9600);
    pinMode(button, INPUT);
    // Contrast for LCD
    analogWrite(6,60);
    //resetPrizes();
    checkPrizes();
    showPrizes();
}

void resetPrizes() {
    Serial.println("Writing 0 to reset");
    EEPROM.write(0, 0);
}

void checkPrizes() {
    if (EEPROM.read(0) == 0x0) {
        Serial.println("Writing 1 to avoid reset");
        EEPROM.write(0, 0x1);
        for (int i = 1; i < 5; i++) {
            EEPROM.write(i, prizes[i - 1]);
        }
    } else {
        Serial.println("There was a 1");
        for (int i = 1; i < 5; i++) {
            prizes[i - 1] = EEPROM.read(i);
        }
    }
}

void updatePrizes(int n) {
    prizes[n - 1] -= 1;
    EEPROM.write(n, prizes[n - 1]);
}

int selectPrize() {
    int sum = prizes[0] + prizes[1] + prizes[2] + prizes[3];
    if (sum == 0) {
        return 0;
    }
    int luck = random(1, sum);
    int prize = 0;
    if (luck <= prizes[0]) {
        prize = 1;
    } else if (luck <=  (prizes[0] + prizes[1])) {
        prize = 2;
    } else if (luck <=  (prizes[0] + prizes[1] + prizes[2])) {
        prize = 3;
    } else {
        prize = 4;
    }
    
    return prize;
}

byte nextLed() {
    if (current_led == 0x7) {
        current_led = 0xE;
    } else {
        current_led -= 0xF - current_led;
    }
}

void switchNextLed() {
    nextLed();
    Wire.beginTransmission(expander);
    Wire.write(current_led);
    Wire.endTransmission(expander);
}

void brakeTo(int endingLed) {
    byte endLedHex = 0xF - (pow(2, endingLed - 1));
    while (current_led != endLedHex) {
        switchNextLed();
        delay(400);
    }
}

void rollTo(int endingLed) {
    unsigned int elapsed = 0;
    int step = 100;
    while (elapsed < 8000) {
        switchNextLed();
        elapsed += step;
        delay(step);
        step += 1;
    }
    brakeTo(endingLed);
}

void pressedButton() {
    while (true) {
        if (digitalRead(button)) {
            unsigned long since_bounce = millis() - last_bounce;
            if (since_bounce < debounce_time) {
                delay(debounce_time - since_bounce);
            } else {
                break;
            }
        }
    }
    last_bounce = millis(); 
}

boolean isPressed() {
    if (millis() - last_bounce > debounce_time) {
        if (digitalRead(button)) {        
            last_bounce = millis();
            return true;
        } 
    }
    return false;
}

void showPrizes() {
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print(P1 + prizes[0]);
    lcd.setCursor(10, 0);
    lcd.print(P2 + prizes[1]);
    lcd.setCursor(0, 1);
    lcd.print(P3 + prizes[2]);
    lcd.setCursor(10, 1);
    lcd.print(P4 + prizes[3]);
}

void loop() {
    if (isPressed()) {
        int prize = selectPrize();
        if (prize == 0) {
            Serial.println(NONE_LEFT);
            return;
        }
        rollTo(prize);
        Serial.print(PRIX + prize);
        updatePrizes(prize);
        showPrizes();
    }
}
