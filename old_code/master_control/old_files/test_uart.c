#include "test_uart.h"



int main(int argc, char *argv[]) {
	// bind handler to signal interrupt
	
	Fs = 40;

	// Checking if file exists 
	if(access(UARTPORT, F_OK) == -1) {
		printf("UART serial device file does not exist. Setting up now.\n");
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
	size_t size = sizeof(buf);

	// Control Logic
	
	// Serial Port Information Collection
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
	} else if (strstr(UART_buf, "r") != NULL) {
		printf("RIGHT\n");
	} else {
		printf("Stationary\n");
	}
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

