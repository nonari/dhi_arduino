const int button = 3;
const int red_led = 10;
const int green_led = 5;
const int debounce_time = 50;

void setup() {
    randomSeed(54155);
    Serial.begin(115200);
    pinMode(button, INPUT);
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
}

boolean pressedButton() {
    boolean pressed = false;
    static unsigned long starttime = millis(); 
    if (digitalRead(button)) {
        if (millis() - starttime > debounce_time) {
            pressed = true;
        }
        starttime = millis();
    }
    return pressed;
}

void unpressedButton() {
    while (true) {
        if (!digitalRead(button)) {
           delay(debounce_time);
           if (!digitalRead(button)) {
               break;
           }
        }
    }
}

void loop() {
    Serial.println("Start");
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    while (!pressedButton());
    while (digitalRead(button));
    int counter = random(400, 1200);
    digitalWrite(green_led, LOW);
    boolean fail = false;
    while (counter != 0 && !fail) {
        delay(1);
        if (pressedButton()) {
            Serial.println("Demasiado rapido");
            fail = true;
        }
        counter--;
    }
    if (!fail) {
        digitalWrite(red_led, HIGH);
        unsigned int time = millis();
        while(!pressedButton());
        digitalWrite(red_led, LOW);
        unsigned int total = millis() - time;
        if (total < 100) {
            Serial.print("Buenos reflejos");
        } else {
            Serial.print("Conduces como una ancianita");
        }
    }
    while (digitalRead(button));
    loop();
}
