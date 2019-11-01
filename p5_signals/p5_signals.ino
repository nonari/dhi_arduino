#include <MsTimer2.h>
#include <math.h>

// Samples per cicle
const unsigned int N = 200;
// Samples interval in ms
const unsigned int Ts = 5; // Samples interval in ms
// Signal period T= N*Ts (ms)

// Triangular signal points
const unsigned int Q = N / 4;
const unsigned int Q3 = 3 * N / 4;

// Square signal raising point
const unsigned int Q5 = N / 5;

// Current sample number
volatile unsigned int n = 0;
// Current signal value
volatile float x;

void (*functions[6])();
unsigned int current_signal = 0;
unsigned long since_change = 0;
const unsigned long SHIFT_TIME = 5000;

void setup() {  
    Serial.begin(230400);
    
    addFunctions();
    
    MsTimer2::set(Ts, plot);
    MsTimer2::start();
}  
    
void loop() {
}

void addFunctions() {
  functions[0] = &sine;
  functions[1] = &cosine;
  functions[2] = &AM;
  functions[3] = &triangular;
  functions[4] = &square;
  functions[5] = &BPSK;
}

void plot() {
    since_change += Ts;
    if (since_change > SHIFT_TIME) {
        since_change = 0;
        current_signal = ++current_signal % 6;
    }
    functions[current_signal]();
    Serial.println(x);
    if (++n == N) {
        n = 0;
    }
}

// 2.5V DC + sine 1.25V 1Hz
void sine() {
    x = 2.5 + 1.25 * sin(n * 2 * PI / N); 
}

// 2.5V DC + cosine 1.25V 20Hz
void cosine() {
    x = 2.5 + 1.25 * cos(n * 20 * PI / N); 
}

// 2.5V DC + AM signal with cosine 10Hz bearer and sine 1Hz modulator
void AM() {
    x = 2.5 + (1 + 0.5 * sin(2 * PI * n / N)) * cos(20 * PI * n / N );
}

// 2.5V DC + triangular 1.25V 1Hz
void triangular() {
    float q =  (4.0 * (float)n) / (float)N;

    if (0 <= n && n < Q) {
        x = 2.5 - (1.25 * -q);
    }
    if (Q <= n && n < Q3) {
        x = 2.5 + (1.25 * (2.0 - q));
    }
    if (Q3 <= n && n < N) {
        x = 2.5 - (1.25 * (4.0 - q));
    }
}

// 5V square 20% duty cicle
void square() {
    if (n < Q5) {
        x = 5;
    } else {
        x = 0;
    }
}

// Binary Phase-Shift Keying with cosine 10Hz bearer and square 20% duty cycle modulator
void BPSK() {
    if (n < Q5) {
        x = 2.5 + 1.25 * cos(20 * PI * n / N);
    } else {
        x = 2.5 - 1.25 * cos(20 * PI * n / N);
    }
}
