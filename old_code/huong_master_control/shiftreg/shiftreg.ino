int latchPin = 8;
int clockPin = 12;
int dataPin = 11;
int ePin = 2;
int rsPin = 4;
int rwPin = 3;

void setup() {
	Serial.begin(9600);
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	pinMode(ePin, OUTPUT);
	pinMode(rsPin, OUTPUT);
	pinMode(rwPin, OUTPUT);
	lcd_start();
}

void loop() {
	// if (Serial.available()) {
	// 	char ch = Serial.read();
	// 	write_char(ch);
	// }
}

void check_special_chars(char val) {
	int ascii = val - '0';
	if (ascii == 45) {
		clean_screen();
	} else if (ascii == 47) { // move to second line
		change_line(1);
	} else if (ascii == 63) { // move to first line
		change_line(0);
	} else if (ascii == 44) { // shift left
		shift(0, 0);
	} else if (ascii == 46) { // shift righT
		shift(0, 1);
	} else if (ascii == 60) { // shift display left
		shift(1, 0);
	} else if (ascii == 62) { // shift display right 
		shift(1, 1);
	}
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

	write_int(15);
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
	int ascii = chr - '0';
	digitalWrite(rsPin, HIGH);
	digitalWrite(rwPin, LOW);
	write_int(ascii);
	toggle_e();
} 


