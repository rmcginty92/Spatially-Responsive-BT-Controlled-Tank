#include "master.h"

int main(int argc, char *argv[]) {
	// bind handler to signal interrupt
	
	Fs = 40;

	// Checking if file exists 
	if(access(UARTPORT, F_OK) == -1) {
		FILE *ain;
		ain = fopen(DTBOLOAD_FILE, "w"); // use this for older version of debian on bot's micro sd card
		fseek(ain, 0, SEEK_SET);
		fprintf(ain, "BB-UART1"); // use this for micro sd card
		fflush(ain);
		fclose(ain);
	}
	uart_Init();
	timer_Init(SIGALRM, timer_handler);
	pid = atoi(argv[1]);
	while(1){ 
		usleep(1000000);
	}

	return 0;
}

void timer_handler(int signo) {  
	char buf[MAX_BUFF];  
	char *pinNum;
	char *pinVal;
	int adc_fd;
	int success, i, res;
	int value[DEPTH_LEN];
	fd_set set;
	struct timeval timeout2;
	size_t size = sizeof(buf);

	mknod(sensor_fifo, S_IFIFO | 0666, 0);
	adc_fd = open(sensor_fifo, O_RDONLY);
	success = read(adc_fd, buf, size); 
	close(adc_fd);

	// copies the current value in the buffer to permanent array holding pin values
	if (success != 0 & buf[0] != (int) NULL) {
		pinNum = strtok(buf, ":");
		pinVal = strtok(NULL, ",");           
		value[atoi(pinNum)] = atoi(pinVal);

		for (i = 1; i < DEPTH_LEN; i++) {
			pinNum = strtok(NULL, ":");
			pinVal = strtok(NULL, ",");
			value[atoi(pinNum)] = atoi(pinVal);
		}
	}
	// ******************************************
	// Control Logic

	// Converting all values into integer scale from 0 to 1000
	for (i = 0; i < DEPTH_LEN; i++) {
		norm_vals[i] = (int) (value[i] * norm) / (max_adc_range[i] - min_adc_range[i]);
	}

	// Serial Port Information Collection

	FD_ZERO(&set); /* clear the set */
	FD_SET(uart_fd, &set); /* add our file descriptor to the set */
	timeout2.tv_sec = 0;
	timeout2.tv_usec = 100000; // 10 milliseconds 
	if (select(uart_fd + 1, &set, NULL, NULL, &timeout2)) { // bt data exists	
		memset(&UART_buf[0], 0, sizeof(UART_buf));
		res = read(uart_fd, UART_buf, sizeof(UART_buf));
		UART_buf[res] = 0;
	
		if (strstr(UART_buf, "f") != NULL) {
			printf("FORWARD\n");
		} else if (strstr(UART_buf, "b") != NULL) {
			printf("BACKWARD\n");
		} else if (strstr(UART_buf, "l") != NULL) {
			printf("LEFT\n");
		} else if (strstr(UART_buf, "r") != NULL) {
			printf("RIGHT\n");
		} else if (strstr(UART_buf, "c") != NULL) { // callibrate
			printf("RIGHT\n");
		} else if (strstr(UART_buf, "x") != NULL) {
			printf("retrace\n");
		} 
	} else {
		printf("Stationary\n");
	}

/*	// Commented out for testing purposes
	// Start with whether something lies exclusively behind the tank (i.e. can move other direction if necessary)
	if ((norm_vals[1] > threshold_val && norm_vals[0] <= threshold_val)) {
		// Check sides of tank
		isStandby = 0;
		if (norm_vals[2] > threshold_val && norm_vals[3] <= threshold_val) {
		   kill(pid, SIG_USER_LEFT); 
		} else if(norm_vals[3] > threshold_val && norm_vals[2] <= threshold_val) {
		   kill(pid, SIG_USER_RIGHT);
		} else { // either sides are blocked or completely free - either way go forward
		   kill(pid, SIG_USER_FORWARD); 
		}
	} else if (norm_vals[0] > threshold_val && norm_vals[1] <= threshold_val) {
		// No back-left or back-right funcstionality yet, so just goes backward
		isStandby = 0;
		kill(pid, SIG_USER_BACKWARD);
	} else { 
		if (!isStandby) {
		   kill(pid, SIG_USER_STANDBY);
		   //kill(pid, SIG_USER_BRAKE); 
		   isStandby = 1;
		}
	}
	// ******************************************/
}

// creates a timer and associate interrupt signals
int timer_Init(int signum, void (*handler)(int)) {
	/* unblock the signal passed in. */
	sigemptyset(&signals);
	sigaddset(&signals, signum);
	sigprocmask(SIG_UNBLOCK, &signals, NULL); // adds the signal passed in to the set of unblocked signals

	/* install the signal handler. */
	action.sa_handler = handler;
	action.sa_flags = 0; // tells us that sa_handler instead of sa_sigaction should be used 
	sigemptyset(&action.sa_mask); // empties the set of signals which should be ignored
	sigaction(signum, &action, NULL); // examine and change a signal action to sigaction struct

	sigevent.sigev_notify = SIGEV_SIGNAL; // notification method 
	sigevent.sigev_signo = SIGALRM; // notification signal
	if (timer_create(clkid, &sigevent, &timerid)) { // create a timer with the given notification struct
		perror("timer_create");
		return -1;
	}

	/* start the timer. */
	memset(&timeout, 0, sizeof(timeout)); // clears out the struct itimerspec by copying 0 to the whole thing
	timeout.it_interval.tv_sec = 0;
	timeout.it_interval.tv_nsec = (BUFF_LEN*(1000000000 / Fs)) % 1000000000;
	timeout.it_value.tv_sec = 0;
	timeout.it_value.tv_nsec = (BUFF_LEN*(1000000000 / Fs)) % 1000000000;
	if (timer_settime(timerid, 0, &timeout, NULL)) {
		perror("timer_settime");
		return -1;
	}

	return 0;
}

//Sets up Serial connection with Bluetooth Connection
void uart_Init() {
	int c, res;
	struct termios newtio;

	uart_fd = open(UARTPORT, O_RDWR | O_NOCTTY);
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
	tcflush(uart_fd, TCIFLUSH);
	tcsetattr(uart_fd, TCSANOW, &newtio);
}

// Checking the distance sensors and returning 1 if 
int proximity_check() {

}