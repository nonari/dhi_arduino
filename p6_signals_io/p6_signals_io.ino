const float Vref = 5.0; // ADC Reference Voltage
const unsigned int RANGE = 1024; // Arduino Uno ADC Range (10 bits)
const float VOLTS_BY_STEP = Vref / RANGE;
const unsigned int T = 1000; // Interruption interval for T1 from 1000 to 64000us
const byte Tg = 1; // Signal generation interval Tg*T us  
const byte Ts = 1; // Signal acquisition interval Ts*T us

const unsigned int READ_INTERVAL = 200;
const unsigned int DEBOUNCE = 200;
const int CH1_SWITCH = 5;
const int CH2_SWITCH = 6;
const int HZ_ROTATOR = 4;


unsigned long timer = 0; 

unsigned int last_read_btn = 0;
unsigned int last_read_amp = 100;

unsigned long last_bounce = 0;

bool button_enable_ch1 = true;
bool button_enable_ch2 = true;
bool button_enable_hz = true;

int (*functions_ch1[4])(int);
int (*functions_ch2[4])(int);
unsigned int current_signal[2] = {0, 0};
int (*curr_func_ch1) (int) = &sine;
int (*curr_func_ch2) (int) = &cosine;

unsigned int N = 200; // Samples per cicle
unsigned int n = 0; // Sample number
unsigned int dg = 0; // Generation divider (0 to Tg-1)ms
unsigned int ds = 0; // Acquisition divider (0 to Ts-1)ms
unsigned int A_in_1;
unsigned int A_in_2;  
byte channel = 0; // Current channel
byte amp1 = 127, amp2 = 127; // Amplitudes
byte ch1_out, ch2_out;

unsigned int Q = N / 4;
unsigned int Q3 = 3 * N / 4;
unsigned int Q2 = N / 2;
unsigned int inter = 0;


void setup()  {  
    Serial.begin(230400);
    pinMode(CH1_SWITCH, INPUT_PULLUP);
    pinMode(CH2_SWITCH, INPUT_PULLUP);
    pinMode(HZ_ROTATOR, INPUT_PULLUP);
    addFunctions();
    TCCR1B = bit(ICES1)|bit(CS10); // Prescaler T1 /1
    TIMSK1 = bit(TOIE1); // Enable int. overflow T1
    TCNT1= 0; // T1 counter to zero
    TCCR2B &= 0xF8; // Zero in prescaler T2 (PWM 3 y 11)
    TCCR2B |= 0x01; // PWM 3 & 11 to 32 kHz
}  

void recalcLimits() {
    Q = N / 4;
    Q3 = 3 * N / 4;
    Q2 = N / 2;
    inter = 0;
}

void addFunctions() { 
    functions_ch1[0] = &sine;
    functions_ch1[1] = &triangular;
    functions_ch1[2] = &square;
    functions_ch1[3] = &DC;
    functions_ch2[0] = &cosine;
    functions_ch2[1] = &triangular;
    functions_ch2[2] = &square;
    functions_ch2[3] = &DC;
} 

int buttonPressed() {
    if (digitalRead(CH1_SWITCH) == 0) {
        if (button_enable_ch1) {
            button_enable_ch1 = false;
            return CH1_SWITCH;
        }
    } else {
        button_enable_ch1 = true;
    }
    if (digitalRead(CH2_SWITCH) == 0) {
        if (button_enable_ch2) {
            button_enable_ch2 = false;
        return CH2_SWITCH;
        }
    } else {
        button_enable_ch2 = true;
    }
    if (digitalRead(HZ_ROTATOR) == 0) {
        if (button_enable_hz) {
            button_enable_hz = false;
            return HZ_ROTATOR;
        }
    } else {
        button_enable_hz = true;
    }
    return 0;
}  

bool isPressed(int button) {
    if (button != 0 && ((timer - last_bounce) > DEBOUNCE)) {        
        last_bounce = timer;
        return true;
    } 
    return false;
}
    
void loop() {
}

int mapAmp(int amp) {
    return map(amp, 1024, 0, 0, 127); 
}

// 2.5V + amp cosine wave
int cosine(int amp) {
   return 128 + amp * cos(n * 2 * PI / N); 
}

// 2.5V + amp sine wave
int sine(int amp) {
    return 128 + amp * sin(n * 2 * PI / N); 
}

// 2.5V + amp triangular wave
int triangular(int amp) {
    float q =  (4.0 * (float)n / (float)N);
    
    if (0 <= n && n < Q) {
        return 128 - ((float)amp * -q);
    }
    if (Q <= n && n < Q3) {
        return 128 + ((float)amp * (2.0 - q));
    }
    if (Q3 <= n && n < N) {
        return 128 - ((float)amp * (4.0 - q));
    }
}

// 2.5V + amp square 50% duty cicle
int square(int amp) {
    if (n < Q2) {
        return 128 + amp;
    } else {
        return 0;
    }
}

// (amp * 2)V DC
int DC(int amp) {
    return amp * 2;
}

void readButtons() {
    if (timer - last_read_btn > READ_INTERVAL) {
        last_read_btn = timer;
        
        if ((timer - last_bounce) > DEBOUNCE) {    
            int button = buttonPressed();
            if (button != 0) {
                last_bounce = timer;
                if (button == HZ_ROTATOR) {
                    if (N == 500 ) {
                        N = 50;
                    } else {
                        N += 50;
                    }
                    recalcLimits();
                }
                if (button == CH1_SWITCH) {
                    current_signal[0] = ++current_signal[0] % 4;
                    curr_func_ch1 = functions_ch1[current_signal[0]];
                }
                if (button == CH2_SWITCH) {
                    current_signal[1] = ++current_signal[1] % 4;
                    curr_func_ch2 = functions_ch2[current_signal[1]];
                }
            }
        }
    }
}

void readAmp() {
    if (timer - last_read_amp > READ_INTERVAL) {
        last_read_amp = timer;
        amp1 = mapAmp(analogRead(2));
        amp2 = mapAmp(analogRead(3));
    }
}

// Interruption handler
ISR(TIMER1_OVF_vect) {
    TCNT1 += 65536 - 16*T; // T1 interrupt each T us
    timer ++;
    
    // PWM signal generation each Tg*T us
    if (++dg == Tg) {
        dg= 0;
        
        analogWrite(3, curr_func_ch1(amp1));
        analogWrite(11, curr_func_ch2(amp2));
    }
    if (++n >= N) {
        n= 0;
    }
    
    // Signal acquisition Ts*T us
    if (++ds == Ts) {
        ds = 0;
        // Data sent to plotter is splitted between channels in T interval
        if (Ts == 1) {
            if (channel == 0) {
                A_in_1 = analogRead(0);
            } else {
                A_in_2 = analogRead(1);
            }
        } else {
            A_in_1 = analogRead(0);
            A_in_2 = analogRead(1);
        }
    }
    if (channel++ == 0) {
        Serial.print(A_in_1 * VOLTS_BY_STEP);
        Serial.print(" ");
    } else {
        Serial.println(A_in_2 * VOLTS_BY_STEP);
        channel = 0;
    }
    
    readAmp();
    readButtons();
}
    
