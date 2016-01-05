// MPU-6050 Short Example Sketch
// By Arduino User JohnChi
// August 17, 2014
// Public Domain

#include <Wire.h>
#include <SoftwareSerial.h>

const int MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int num = 4;
float Vref = 3.3;

String new_str = "";
String old_str = "";
String str_forward =  "forward";
String str_backward = "back";
String str_left =     "left";
String str_right =    "right";

int bluetoothTx = 7;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 6;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

int latchPin = 8;
int clockPin = 12;
int dataPin = 11;
int ePin = 2;
int rsPin = 4;
int rwPin = 3;

void setup(){
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);

    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    pinMode(ePin, OUTPUT);
    pinMode(rsPin, OUTPUT);
    pinMode(rwPin, OUTPUT);

    Serial.begin(9600);

    bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
    bluetooth.print("$");  // Print three times individually
    bluetooth.print("$");
    bluetooth.print("$");  // Enter command mode
    delay(100);  // Short delay, wait for the Mate to send back CMD
    bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
    bluetooth.begin(9600);  // Start bluetooth serial at 9600

    Serial.println("finished setting up bluetooth");

    lcd_start();
}

void loop(){
    read_bluetooth();
    if(Serial.available())  // If stuff was typed in the serial monitor
    {
        // Send any characters the Serial monitor prints to the bluetooth
        bluetooth.print((char) Serial.read());
    }
    calculate_roll_pitch();
}

void read_bluetooth() {
    String message = "";
    int i = 0;
    while (bluetooth.available() > 0) {
        if (i == 0) {
            // clean_screen();
            change_line(1);
        }
        char newchar = (char) bluetooth.read();
        message += newchar; 

        if (newchar == '\n') {
            // Serial.print("Arduino Received: ");
            // Serial.print(message);
            print_to_lcd(message);
            message = "";
        }
        i++;
    }
    change_line(0);
}

void calculate_roll_pitch() {
    char movement;
    int long AcXTot = 0;
    int long AcYTot = 0;
    int long AcZTot = 0;

    for (int i = 0; i < num; i++) {
        Wire.beginTransmission(MPU_addr);
        Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_addr, 14,true);  // request a total of 14 registers

        AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
        AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
        AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
        Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
        GyX = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
        GyY = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
        GyZ = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

        AcXTot += AcX;
        AcYTot += AcY;
        AcZTot += AcZ;

        // delay(100);
    }

    float Vx = analog_to_voltage(AcXTot);
    float Vy = analog_to_voltage(AcYTot);
    float Vz = analog_to_voltage(AcZTot);

    float Ax = voltage_to_acc(zero_x(Vx));
    float Ay = voltage_to_acc(zero_y(Vy));
    float Az = voltage_to_acc(zero_z(Vz));

    float pitch = atan(Ax / sqrt(pow(Ay, 2) + pow(Az, 2))); 
    float roll = atan(Ay / sqrt(pow(Ax, 2) + pow(Az, 2)));
    pitch = pitch * (180.0 / PI);
    roll = roll * (180.0 / PI) ;

    if (roll > 20) {        
        // Serial.println("go left");
        movement = 'l';
        bluetooth.println(movement); 
        new_str = str_left;
    } else if (roll < -20) {
        // Serial.println("go right");
        movement = 'r';
        bluetooth.println(movement); 
        new_str = str_right;
    } else if (pitch > 30) {
        // Serial.println("go forward");
        movement = 'f';
        bluetooth.println(movement); 
        new_str = str_forward;
    } else if (pitch < -20) {
        // Serial.println("go backward");
        movement = 'b';
        bluetooth.println(movement); 
        new_str = str_backward;
    } else {
        new_str = "";
    }

    if (new_str != old_str) {
        clean_screen();
        print_to_lcd(new_str);
    }
    old_str = new_str;
}

void print_to_lcd(String str) {
    for (int i = 0; i < str.length(); i++) {
        write_char(str.charAt(i));
    }
}

float analog_to_voltage(int long value) {
    float voltage = ((float) value / num) * Vref / 65536.0;
    return voltage;
}

float zero_x(float voltage) {
    float calibrated = voltage - 0.04;
    return calibrated;
}

float zero_y(float voltage) {
    float calibrated = voltage + 0.02;
    return calibrated;
}

float zero_z(float voltage) {
    float calibrated = voltage + 0.03;
    return calibrated;
}

float voltage_to_acc(float voltage) {
    float acceleration = (voltage / 0.91) * 9.8; 
    return acceleration;
}

void write_int(int numberToDisp) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, numberToDisp);
    digitalWrite(latchPin, HIGH);
    delay(10);
    digitalWrite(latchPin, LOW);
}

void lcd_start() {
    Serial.println("start initializing LCD");
    delay(50);

    digitalWrite(rsPin, LOW);
    digitalWrite(rwPin, LOW);
    write_int(0);
    toggle_e();

    write_int(48);
    toggle_e();

    write_int(48);
    toggle_e();

    write_int(56);
    toggle_e();

    write_int(8);
    toggle_e();

    Serial.println("clean the screen");
    clean_screen();
    Serial.println("finish initializing LCD");
}

void toggle_e() {
    digitalWrite(ePin, HIGH);
    delay(10);
    digitalWrite(ePin, LOW);
    delay(10);
}

void clean_screen() {
    digitalWrite(rsPin, LOW);
    digitalWrite(rwPin, LOW);
    write_int(1);
    toggle_e();

    write_int(6);
    toggle_e();

    write_int(12);
    toggle_e();
}

void change_line(int pos) {
    int ascii;
    ascii = 128 + 64 * pos;
    digitalWrite(rsPin, LOW);
    digitalWrite(rwPin, LOW);
    write_int(ascii);
    toggle_e();
}

void shift(int sc, int rl) {
    int ascii;
    ascii = 16 + sc * 8 + rl * 4;
    digitalWrite(rsPin, LOW);
    digitalWrite(rwPin, LOW);
    write_int(ascii);
    toggle_e();
}

void turn_off() {
    digitalWrite(rsPin, LOW);
    digitalWrite(rwPin, LOW);
    write_int(8);
    toggle_e();
}

void write_char(char chr) {
    int ascii = chr - '0' + 48;
    if (ascii > 31) {
        digitalWrite(rsPin, HIGH);
        digitalWrite(rwPin, LOW);
        write_int(ascii);
        toggle_e();        
    }
} 
