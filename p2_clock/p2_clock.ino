#include <avr/sleep.h>
#include <MsTimer2.h>

const int button = 3;
const int red_led = 10;
const int green_led = 5;

const String point = ":";

int total_recv = 0;
char data[9];

volatile boolean keep_sleeping = false;
volatile unsigned long counter = 0;
unsigned long last_print = 0;
volatile boolean tick = false;

void setup() {
    randomSeed(analogRead(0));
    Serial.begin(115200);
    pinMode(button, INPUT);
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
    attachInterrupt(1, wakeUp, HIGH);
    MsTimer2::set(1000, handleTimer);
    MsTimer2::start();
}

void handleTimer() {
    counter++;
    tick = !tick;
    if (keep_sleeping) {
        sleepNow();
    } else {
        if (tick) {
            digitalWrite(green_led, LOW);
            digitalWrite(red_led, HIGH);
        } else {
            digitalWrite(red_led, LOW);
            digitalWrite(green_led, HIGH);
        }
        
        if (last_print < counter) {
            Serial.println(currentTime(counter));
            last_print = counter;
        }
    }   
}

String currentTime(unsigned long raw_seconds) {
    unsigned long total_seconds = raw_seconds % 86400;
    unsigned long hours = total_seconds / 3600;
    unsigned long minutes = (total_seconds - (3600 * hours)) / 60;
    unsigned long seconds = total_seconds - (minutes * 60) - (hours * 3600);
    String time = hours + point + minutes + point + seconds;
    return time;
}

unsigned int parseInt(char frst, char scnd) {
    unsigned int first_char = frst - 48;
    unsigned int second_char = scnd - 48;
    return (first_char * 10) + second_char;
}

boolean isInt(char point) {
    return (point > 47 && point < 58);
}

unsigned long parseTime(char time_str[], int size) {
    char accum[8];
    int acc = 0;
    long result[3] = {0,0,0};
    int res = 0;
    for (int i = 0; i < size; i++) {
        char code = time_str[i];
        if (isInt(code)) {
            accum[acc++] = code;
        } else {
            long parsed;
            if (acc > 1) {
                parsed = parseInt(accum[0], accum[1]);
            } else {
                parsed = parseInt('0', accum[0]);
            }
            result[res++] = parsed;
            acc = 0;
        }
    }
    return (result[0] * 3600) + (result[1] * 60) + result[2];
}

void sleepNow() {
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, LOW);
    keep_sleeping = true;
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sei();
    sleep_cpu();
    sleep_disable();
}

void adjust() {
    if (Serial.available()) {
        char recv = Serial.read();
        if (total_recv < 9) {
            data[total_recv++] = recv;
        }
        if (recv == '\n') {
            if (total_recv < 10) {
                unsigned long new_counter = parseTime(data, total_recv);
                if (new_counter > 86400) {
                    Serial.println("Paso a modo sleep");
                    Serial.flush();
                    sleepNow();
                } else {
                    counter = new_counter;
                    last_print = 0;
                }
            }
            total_recv = 0;
        }
    }
}

void wakeUp() {
    sleep_disable();
    keep_sleeping = false;
}

void loop() {
    if (!keep_sleeping) {
        adjust();
    } else {
        Serial.read();
    }
}
