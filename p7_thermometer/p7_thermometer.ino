#include <MsTimer2.h>

// Pins to segments
const byte segs[7]= {3,4,5,6,7,8,9}; //a-b-c-d-e-f-g
// Pins to digits
const byte digs[4]= {10,11,12,13}; // 1-2-3-4
// Digits 0 to 9
const byte digits[10]= {0b00111111, 0b00000110, 0b01011011,
            0b01001111, 0b01100110, 0b01101101, 0b01111101,
            0b00000111, 0b01111111, 0b01101111};
// Symbols
const byte MINUS = {0b01000000};
const byte POINT = {0b10000000};
const byte DEGREE = {0b01100011};
const byte CELSIUS = {0b00111001};

const unsigned int SAMPLE_SIZE = 40;
const unsigned int MOV_AVG_SIZE = 5;

// Current digit values
volatile byte bytes[]= {0,0,0,0};

int sampling_buffer[SAMPLE_SIZE];
int moving_avg[MOV_AVG_SIZE];
unsigned int curr_sample = 0;
unsigned int curr_avg = 0;

void setup() {
    Serial.begin(9600);
    analogReference(INTERNAL);
    for (int pin = 3; pin <= 13; pin++) {
        pinMode(pin, OUTPUT);
    }  
    MsTimer2::set(5, handle);
    MsTimer2::start();
}

void loop() {
}


void handle() {
    addSample();
    if (curr_sample ==  0) {
        int temp = temperature();
        displayNumber(temp);
    }
    multiplexate();
}

void displayNumber(int number) {
    for (int i = 0; i < 4; i++) {
        bytes[i] = 0;
    }
    int pos = 0;
    int abs_num = number;
    if (number < 0) {
        abs_num = number * -1;  
        bytes[pos] = MINUS;
        pos++;
    }
    if (abs_num > 9) {
        int fst_digit = abs_num / 10;
        bytes[pos] = digits[fst_digit];
        pos++;
    }
    int scd_digit = abs_num % 10;
    bytes[pos] = digits[scd_digit];
    pos++;
    bytes[pos] = DEGREE;
    pos++;
    if (number >= -9) {
        bytes[pos] = CELSIUS;
    }
}

void addSample() {
    int value = analogRead(0);
    //Serial.println(value);
    sampling_buffer[curr_sample] = value;
    if (++curr_sample == SAMPLE_SIZE) {
        addMean();
        curr_sample = 0;
    }
}

void addMean() {
    int sum = 0;
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        sum += sampling_buffer[i]; 
    }
    int avg = sum / SAMPLE_SIZE;
    moving_avg[curr_avg] = avg;

    if (++curr_avg == MOV_AVG_SIZE) {
        curr_avg = 0;
    }
}

// Measure range -50 to 60 Celsius
// 0 Celsius = 500mV
// Celsius = (Voltage - ((1025 * 500) /  1100)) * (110 / 1024)  
int temperature() {
    int sum = 0;
    for (int i = 0; i < MOV_AVG_SIZE; i++) {
        sum += moving_avg[i]; 
    }
    int avg = sum / MOV_AVG_SIZE;
    float celsius = ((float)avg - ((1025.0 * 500.0) /  1100.0)) * (110.0 / 1024.0);

    return (int)celsius;
}

void multiplexate() {
    static int pos = 0;
    printData(bytes[pos]);
    for (int i = 0; i <= 3; i++) {
        if (i == pos) {
          digitalWrite(digs[i], LOW);
        } else {
          digitalWrite(digs[i], HIGH);
        }
    }
    pos = ++pos % 4;
}

void printData(byte value) {
  for (int seg = 0; seg < 7; seg++) {
      digitalWrite(segs[seg], bitRead(value, seg));
  }
}
