#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const byte monkey[8] = {
        B01110,
        B01110,
        B00100,
        B01110,
        B10101,
        B00100,
        B01010,
        B10001
};
const byte mon_trapped[8] = {
        B10001,
        B10001,
        B11011,
        B10001,
        B01010,
        B11011,
        B10101,
        B01110
};
const byte trap[8] = {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111
};
const byte tiger[8] = {
        B10001,
        B01110,
        B10001,
        B10101,
        B00100,
        B10001,
        B01010,
        B00100
};
const byte bananas[8] = {
        B10101,
        B10101,
        B10101,
        B00000,
        B10101,
        B10101,
        B10101,
        B00000
};

struct Movement {
    bool x;
    int8_t d;
};

const String MENU1 = "  MONKEY TRAPS  ";
const String MENU2 = " Press Joystick ";
const String DIED = "     D I E D    ";
const String TRAPPED = "  T R A P P E D ";
const String HAPPY = "   H A P P Y   ";
const String MONKEY_TEXT = "  M O N K E Y  "; 
uint8_t remaining_time = 0;
unsigned long start_time = 0;
unsigned long last_change = 0;
unsigned long last_movement = 0;

uint8_t monkey_pos[2] = {1, 1}; 
uint8_t tiger_pos = 0;
int8_t traps_pos[14] = {-1, 0, -1, 1, 0, -1, 0, 1, 1, -1, 0, 1, 1, 0};

uint8_t MONKEY = 1;
uint8_t TRAP = 2;
uint8_t TIGER = 3;
uint8_t BANANA = 4;
uint8_t MONK_TRAP = 5;

const int SW_pin = 7;
const int X_pin = 0;
const int Y_pin = 1;
    
void setup() {
    randomSeed(analogRead(0));
    // LCD contrast voltage to pin 6
    analogWrite(6,120);
    
    Serial.begin(9600);
    
    pinMode(SW_pin, INPUT_PULLUP);
    
    lcd.createChar(MONKEY, (uint8_t*)monkey);
    lcd.createChar(TRAP, (uint8_t*)trap);
    lcd.createChar(TIGER, (uint8_t*)tiger);
    lcd.createChar(BANANA, (uint8_t*)bananas);
    lcd.createChar(MONK_TRAP, (uint8_t*)mon_trapped);
    lcd.begin(16, 2);
}

void resetTime() {
    start_time = millis();
    remaining_time = 32;
}

void updateTime() {
    uint8_t elapsed = (millis() - start_time) / 1000;
    remaining_time = 32 - elapsed;
}

void showTime() {
    int frst_unit = remaining_time / 10;
    int scnd_unit = remaining_time % 10;
    lcd.setCursor(0,0);
    lcd.print(frst_unit);
    lcd.setCursor(0,1);
    lcd.print(scnd_unit);
}

bool isTraped() {
   if (monkey_pos[1] == traps_pos[monkey_pos[0] - 1]) {
       return true;
   }
   
   return false;
}

void showMonkey() {
    lcd.setCursor(monkey_pos[0], monkey_pos[1]);

    if (isTraped()) {
        lcd.write(MONK_TRAP);
    } else {
        lcd.write(MONKEY);
    }
}

void shuffleTraps() {
    if (millis() - last_change > 3000) {
        for (int i = 0; i < 14; i++) {
            if (i==0 || i == 2 || i == 5 || i == 9) {
                continue;
            }
            traps_pos[i] = random(1,8) % 2;
        }
        last_change = millis();
    }
}

void moveTiger(uint8_t pos) {
    tiger_pos = pos;
}

void showTigerAndBanana() {
    uint8_t banana_pos = tiger_pos ^ 0x01;
    lcd.setCursor(15, banana_pos);
    lcd.write(BANANA);
    lcd.setCursor(15, tiger_pos);
    lcd.write(TIGER);
}

void showAlley() {
    for (int i = 0; i<14; i++) {
        if (i == 0 || i == 2 || i == 5 || i == 9) {
            continue;
        }
        
        lcd.setCursor(i + 1, traps_pos[i]);
        lcd.write(TRAP);
    }
}

void moveMonkey(Movement movement) {
    int8_t d = movement.d;
    bool x = movement.x;
    if (x) {
        monkey_pos[0] += d;
    } else {
        monkey_pos[1] += d;
    }
}

bool validMovement(Movement movement) {
    int8_t d = movement.d;
    bool x = movement.x;
    
    if (d == 0) {
        return false;
    }
    if (x) {
        int new_pos = monkey_pos[0] + d;
        if (new_pos >= 1 && new_pos <= 15) {
            return true;
        }
    } else {
        int new_pos = monkey_pos[1] + d;
        if (new_pos >= 0 || new_pos <= 1) {
            return true;
        }
    }
    return false;
}

Movement getMovement() {
  Movement mov;
  int x_val = analogRead(X_pin);
  int y_val = analogRead(Y_pin);

  if (x_val > 800) {
      mov = {true, 1}; 
  } else if (x_val < 200) {
      mov = {true, -1}; 
  } else if (y_val > 800) {
      mov = {false, 1}; 
  } else if (y_val < 200) {
      mov = {false, -1}; 
  } else {
      mov = {true, 0};
  }
  return mov;
}

void renderAll() {
    lcd.clear();
    showTime();
    showAlley();
    showMonkey();
    showTigerAndBanana();
}

void showMessage(String line1, String line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);        
} 

void tryMovement() {
    long unsigned int elapsed = millis() - last_movement;
    if (elapsed > 250 && !isTraped()) {
        Movement movement = getMovement();
        if (validMovement(movement)) {
            moveMonkey(movement);
            last_movement = millis();
            if (isTraped()) {
                moveTiger(monkey_pos[1]);
            }
        }
    }
}

void waitStart() {
    while (digitalRead(SW_pin)) {
       delay(25);
    }
}

void waitUnpressed() {
    while (!digitalRead(SW_pin)) {
       delay(25);
    }
}

bool eaten() {
    if (monkey_pos[0] == 15 && monkey_pos[1] == tiger_pos) {
        return true;
    }
    return false;
}

bool gotBanana() {
    if (monkey_pos[0] == 15 && monkey_pos[1] != tiger_pos) {
        return true;
    }
    return false;
}

bool isFinish() {
    if (remaining_time < 1) {
        showMessage(TRAPPED, MONKEY_TEXT);
        return true;
    } else if (eaten()) {
        showMessage(DIED, MONKEY_TEXT);
        return true;
    } else if (gotBanana()) {
        showMessage(HAPPY, MONKEY_TEXT);
        return true;
    }
    return false;    
}

void resetMonkey() {
    monkey_pos[0] = 1;
    monkey_pos[1] = 1;
}

void loop() {
    showMessage(MENU1, MENU2);
    waitStart();
    resetMonkey();
    resetTime();
    while (!isFinish()) {
        updateTime();
        renderAll();
        tryMovement();
        shuffleTraps();
        delay(125);
    }
    waitStart();
    waitUnpressed();
  
}
