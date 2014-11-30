#define FALSE 0
#define TRUE 1

#define HIGH 1
#define LOW 0

class DPin
{
public:
    int pin;

    DPin(int pin) : pin(pin) { }

    const operator bool(void) {
        pinMode(pin, INPUT);
        return digitalRead(pin);
    }

    const operator = (bool b) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, b);
    }
};

class APin
{
public:
    int pin;

    APin(int pin) : pin(pin) { }

    const operator unsigned int(void) {
        return analogRead(pin);
    }
};


const APin THW(A1);
const int  THWO = 4;
const DPin FAN(3);

const DPin SPD(2);
const APin VTA(A0);
const DPin S1(5);
const DPin S2(6);
const DPin SL(7);

const DPin STP(8);

unsigned char time = 0;
unsigned short speed_raw = 0;
unsigned short throttle_raw = 0;
unsigned char current_step = 0;

void step_set(unsigned char step)
{
    if(step == 0) S2 = 1, S1 = 0;
    else if(step == 1) S1 = 1, S2 = 1;
    else if(step == 2) S2 = 0, S1 = 1;
    else S1 = 0, S2 = 0;

    current_step = step;
}

void setup()
{
    S1.mode(OUTPUT);
    S2.mode(OUTPUT);
    SL.mode(OUTPUT);

    THWO.mode(OUTPUT);
    FAN.mode(OUTPUT);
    STP.mode(INPUT);

    S1 = 0;
    S2 = 0;
    SL = 0;

    FAN = 0;

    attachInterrupt(0, speed_int, FALLING);
}

unsigned char ms = 0;
unsigned char throttle = 0;

void loop(void)
{
    int i;
    static unsigned char sl_counter = 0;
    static unsigned char speed_prev = 0;
    unsigned short speed = 3000/speed_raw;
    throttle = (VTA-40)*10/66;

    tone(THWO, 35);

    Serial.print("Speed: ");
    Serial.print(speed);
    Serial.print(" S1 = ");
    Serial.print(S1);
    Serial.print(" S2 = ");
    Serial.print(S2);
    Serial.print(" SL = ");
    Serial.println(SL);

    if(throttle < 1) throttle = 1;
    if(throttle > 100) throttle = 100;

    if(speed < 70) {
        static unsigned int last_step_val = 0;

        unsigned int step_val = (speed*15)/(throttle);
        unsigned char new_step = step_val/10;

        unsigned int distance = last_step_val > step_val ? last_step_val-step_val : step_val-last_step_val;
        SL = 0;
        sl_counter = 0;

        if(distance > 5) step_set(new_step), last_step_val = step_val;
    }
    else {
        if(throttle < 60) step_set(3);
        else {
            step_set(2);
            sl_counter = 0;
            SL = 0;
        }

        if(speed_prev > speed) SL = 0, sl_counter = 0;
        else {
            if(++sl_counter > 10) {
                sl_counter--;
                SL = 1;
            }
        }
    }

    speed_prev = speed;

    for(i = 0; i < 20000; i++);
}

void speed_int(void)
{
    static unsigned long pmoment = millis();

    speed_raw = (millis()-pmoment)/30;

    pmoment = millis();
}
