#include <Servo.h>

const int PWD_PIN = 9;
Servo servo;

const float Kp = 0.05;
const float Kd = 0.05;
const float Ki = 0.05;

const unsigned int MIN_ERROR = 20;

const unsigned int SAMPLE_SIZE = 40;
int sampling_buffer_a[SAMPLE_SIZE];
int sampling_buffer_b[SAMPLE_SIZE];

byte curr_sample = 0;

long integral_term = 0;
int last_error = 0; 

void setup() {
    Serial.begin(9600);
    servo.attach(PWD_PIN);
    servo.write(0);
}

void addSample() {
    int value_a = analogRead(0);
    int value_b = analogRead(1);

    sampling_buffer_a[curr_sample] = value_a;
    sampling_buffer_b[curr_sample] = value_b;
    curr_sample++;
}

int calcError() {
    int accum_a = 0;
    int accum_b = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        accum_a += sampling_buffer_a[i];
        accum_b += sampling_buffer_b[i];
    }
    return (accum_a / SAMPLE_SIZE) - (accum_b / SAMPLE_SIZE);
}

int calcPDI(int error) {
    updateIntegralTerm(error);
    
    int pdi = (Kp * float(error)) + integral_term + Kd*(error - last_error);
    last_error = error;
    if (pdi > 180) {
        return 180;
    } else if (pdi < 0) {
        return 0;
    }
    return pdi;
}

void updateIntegralTerm(int error) {
    integral_term += Ki * error;
    if (integral_term > 180) {
        integral_term = 180;
    } else if (integral_term < 0) {
        integral_term = 0;
    }
}

void loop() {
    while (curr_sample != SAMPLE_SIZE) {
        addSample();
        delay(1);
    }
    delay(40);
    int error = calcError();
    if (abs(error) > MIN_ERROR) {
        int angle = calcPDI(error);   
        servo.write(angle);
    }
    curr_sample = 0;
}
