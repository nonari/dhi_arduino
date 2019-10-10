const int button = 3;
const int red_led = 10;
const int green_led = 5;

const unsigned int debounce_time = 60;
unsigned int last_bounce = millis();

const String bad = "ms Intente de nuevo";
const String good = "ms BIEN! Buenos reflejos";
const String excelent = "ms ENHORABUENA! Excelentes reflejos";
const String too_fast = "NO SE PRECIPITE. Intente de nuevo";

void setup() {
    randomSeed(analogRead(0));
    Serial.begin(115200);
    pinMode(button, INPUT);
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
}

void pressedButton() {
    while (true) {
        if (digitalRead(button)) {
            unsigned int since_bounce = millis() - last_bounce;
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
            return true;
        } 
    }
    return false;
}

void unpressedButton() {
    while (true) {
        if (!digitalRead(button)) {
            unsigned int since_bounce = millis() - last_bounce;
            if (since_bounce < debounce_time) {
                delay(debounce_time - since_bounce);
            } else {
                break;
            }
        }
    }
    last_bounce = millis();
}

void loop() {
    Serial.println("Start");
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    pressedButton();
    unpressedButton();
    int counter = random(400, 1200);
    digitalWrite(green_led, LOW);
    boolean fail = false;
    while (counter != 0 && !fail) {
        delay(1);
        if (isPressed()) {
            Serial.println();
            fail = true;
        }
        counter--;
    }
    if (!fail) {
        digitalWrite(red_led, HIGH);
        unsigned int time = millis();
        pressedButton();
        digitalWrite(red_led, LOW);
        int total = millis() - time;
        if (total < 100) {
            Serial.println(total + excelent);
        } else if (total < 200) {
            Serial.println(total + good);
        } else {
            Serial.println(total + bad);
        }
    }
    unpressedButton();
}
