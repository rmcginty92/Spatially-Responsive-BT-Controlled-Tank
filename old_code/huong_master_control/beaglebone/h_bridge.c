#include "h_bridge.h"

int main (int argc, char *argv[]) {
	sys = fopen("/sys/class/gpio/export", "w");

	// creating the gpio pins
	writeGPIO(sys, PWMA);
	writeGPIO(sys, AIN1);
	writeGPIO(sys, AIN2);

	writeGPIO(sys, PWMB);
	writeGPIO(sys, BIN1);
	writeGPIO(sys, BIN2);

	writeGPIO(sys, STANDBY);

	// declaring directions of all gpio pins
	dir_pwma = fopen("/sys/class/gpio/gpio66/direction", "w");
	dir_ain2 = fopen("/sys/class/gpio/gpio69/direction", "w");
	dir_ain1 = fopen("/sys/class/gpio/gpio45/direction", "w");
	dir_pwmb = fopen("/sys/class/gpio/gpio67/direction", "w");
	dir_bin2 = fopen("/sys/class/gpio/gpio68/direction", "w");
	dir_bin1 = fopen("/sys/class/gpio/gpio44/direction", "w");
	dir_standby = fopen("/sys/class/gpio/gpio61/direction", "w");

	writeDirGPIO(dir_pwma, "out");
	writeDirGPIO(dir_ain2, "out");
	writeDirGPIO(dir_ain1, "out");
	writeDirGPIO(dir_pwmb, "out");
	writeDirGPIO(dir_bin2, "out");
	writeDirGPIO(dir_bin1, "out");
	writeDirGPIO(dir_standby, "out");

	// opening file pointers for writing values to the gpio pins
	value_pwma = fopen("/sys/class/gpio/gpio66/value", "w");
	value_ain2 = fopen("/sys/class/gpio/gpio69/value", "w");
	value_ain1 = fopen("/sys/class/gpio/gpio45/value", "w");
	value_pwmb = fopen("/sys/class/gpio/gpio67/value", "w");
	value_bin2 = fopen("/sys/class/gpio/gpio68/value", "w");
	value_bin1 = fopen("/sys/class/gpio/gpio44/value", "w");
	value_standby = fopen("/sys/class/gpio/gpio61/value", "w");

	// associate handlers for movements
	setupInterrupt(SIG_USER_LEFT, left_action, left_signals, left_handler);
	setupInterrupt(SIG_USER_RIGHT, right_action, right_signals, right_handler);
	setupInterrupt(SIG_USER_FORWARD, forward_action, forward_signals, forward_handler);
	setupInterrupt(SIG_USER_BACKWARD, backward_action, backward_signals, backward_handler);
	setupInterrupt(SIG_USER_BRAKE, brake_action, brake_signals, brake_handler);
	setupInterrupt(SIG_USER_STANDBY, standby_action, standby_signals, standby_handler);

 	char buf[255];
 	int fd, c, res;
	struct termios newtio;

	printf("finished setting up all pins\n");

	fd = open(SERIALPORT, O_RDWR | O_NOCTTY);
	printf("finished opening up serial port\n");
 	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

    /* BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
                 all necessary lines. See sect. 7 of Serial-HOWTO)
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

    /* IGNPAR  : ignore bytes with parity errors
                 otherwise make device raw (no other input processing) */
    newtio.c_iflag = IGNPAR;

    /*  Raw output  */
    newtio.c_oflag = 0;

    /* ICANON  : enable canonical input
                 disable all echo functionality, and don't send signals to calling
                 program */
    newtio.c_lflag = ICANON;

    /* now clean the modem line and activate the settings for the port */
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    printf("finished setting up bluetooth\n");

    while (1) { /* loop continuously */
    	sysStandby(value_standby);
   		memset(&buf[0], 0, sizeof(buf));
        res = read(fd, buf, sizeof(buf));
        buf[res] = 0;
        printf("%s\n", buf);

        if (strstr(buf, "f") != NULL) {
        	printf("go forward\n");
			moveCCW(value_pwma, value_ain1, value_ain2, value_standby);
			moveCW(value_pwmb, value_bin1, value_bin2, value_standby);
			// usleep(1000000);
			// sysStandby(value_standby);

        } else if (strstr(buf, "b") != NULL) {
        	printf("go backward\n");
			moveCW(value_pwma, value_ain1, value_ain2, value_standby);
			moveCCW(value_pwmb, value_bin1, value_bin2, value_standby);
			// usleep(1000000);
			// sysStandby(value_standby);
		} else {
			sysStandby(value_standby);
		}
        // } else if (strcmp(buf, "c") != 0) {
        	
        // } else if (strcmp(buf, "d") != 0) {
        	
        // }
    }
    	
	fclose(sys);
	fclose(value_pwma);
	fclose(value_ain2);
	fclose(value_ain1);
	fclose(value_pwmb);
	fclose(value_bin2);
	fclose(value_bin1);
	fclose(value_standby);
}

/**********************************************MOTOR CONTROLS**************************************************/
void moveCW(FILE *pwm, FILE *in1, FILE *in2, FILE *standby) {
	writeGPIO(in1, HIGH);
	writeGPIO(in2, LOW);
	writeGPIO(pwm, HIGH);
	writeGPIO(standby, HIGH);
}

void moveCCW(FILE *pwm, FILE *in1, FILE *in2, FILE *standby) {
	writeGPIO(in1, LOW);
	writeGPIO(in2, HIGH);
	writeGPIO(pwm, HIGH);
	writeGPIO(standby, HIGH);
}

void brake(FILE *pwm, FILE *in1, FILE *in2, FILE *standby) {
	writeGPIO(in1, HIGH);
	writeGPIO(in2, HIGH);
	writeGPIO(pwm, LOW);
	writeGPIO(standby, HIGH);
}

void stop(FILE *pwm, FILE *in1, FILE *in2, FILE *standby) {
	writeGPIO(in1, LOW);
	writeGPIO(in2, LOW);	
	writeGPIO(pwm, HIGH);
	writeGPIO(standby, HIGH);
}

void sysStandby(FILE *standby) {
	writeGPIO(standby, LOW); // LOW is OFF, HIGH is ON
}

/*********************************************INTERRUPT HANDLERS************************************************/
void left_handler(int signum) {
	moveCW(value_pwma, value_ain1, value_ain2, value_standby);
	// brake(value_pwmb, value_bin1, value_bin2, value_standby);
}

void right_handler(int signum) {
	moveCW(value_pwmb, value_bin1, value_bin2, value_standby);
	// brake(value_pwma, value_ain1, value_ain2, value_standby);
}

void forward_handler(int signum) {
	moveCW(value_pwma, value_ain1, value_ain2, value_standby);
	moveCCW(value_pwmb, value_bin1, value_bin2, value_standby);
}

void backward_handler(int signum) {
	moveCCW(value_pwma, value_ain1, value_ain2, value_standby);
	moveCW(value_pwmb, value_bin1, value_bin2, value_standby);
}

void brake_handler(int signum) {
	brake(value_pwma, value_ain1, value_ain2, value_standby);
	brake(value_pwmb, value_bin1, value_bin2, value_standby);
}

void standby_handler(int signum) {
	sysStandby(value_standby);
}


/**********************************************HELPER FUNCTIONS*************************************************/
void writeGPIO(FILE* filePtr, int value) {
	fseek(filePtr, 0, SEEK_SET);
	fprintf(filePtr, "%d", value);
	fflush(filePtr);	
}

void writeDirGPIO(FILE* filePtr, char* dir) {
	fseek(filePtr, 0, SEEK_SET);
	fprintf(filePtr, "%s", dir);
	fflush(filePtr);
	fclose(filePtr);
}

void setupInterrupt(int signum, struct sigaction action, sigset_t signals, void (*handler)(int)) {
	/* Unblock the signal passed in. */
	sigemptyset(&signals);
	sigaddset(&signals, signum);
	sigprocmask(SIG_UNBLOCK, &signals, NULL); // adds the signal passed in to the set of unblocked signals

	/* Install the signal handler. */
	action.sa_handler = handler;
	action.sa_flags = 0; 					  // tells us that sa_handler instead of sa_sigaction should be used 
	sigemptyset(&action.sa_mask); 			  // empties the set of signals which should be ignored
	sigaction(signum, &action, NULL); 		  // examine and change a signal action to sigaction struct
}
