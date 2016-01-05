#include "master.h"

int main(int argc, char *argv[]) {
	char buf[MAX_BUFF]; 
	char commandbuff[MAX_BUFF];
	char *pinNum;
	char *pinVal;
	int adc_fd;
	int success, i, res, btdata_ready, uTs, temp;
	int value[DEPTH_LEN];
	fd_set set;
	struct timeval timeout;
	size_t size = sizeof(buf);
	uTs = 1000000 / Fs;

	// Checking if file exists 
	if(access(UARTPORT, F_OK) == -1) {
		FILE *ain;
		ain = fopen(DTBOLOAD_FILE, "w"); 
		fseek(ain, 0, SEEK_SET);
		fprintf(ain, "BB-UART1"); 
		fflush(ain);
		fclose(ain);
	}

	uart_Init();
	pid = atoi(argv[1]);

	// Initializing variables
	tankMode = AVERSION;
	lastDir = STATIONARY;

	mknod(sensor_fifo, S_IFIFO | 0666, 0);

	while(1){ 
		////////////////////START:ADC///////////////////////////
		adc_fd = open(sensor_fifo, O_RDONLY);
		success = read(adc_fd, buf, size); 
		close(adc_fd);

		// Copies the current value in the buffer to permanent array holding pin values
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

		// Converting all values into integer scale from 0 to 1000
		for (i = 0; i < DEPTH_LEN; i++) {
			norm_vals[i] = (int) (value[i] * norm) / (max_adc_range[i] - min_adc_range[i]);
		}

		printf("ADC Values:\t1) %d\t2) %d\t3) %d\t4) %d\t", norm_vals[ADC_FRONT], norm_vals[ADC_BACK], norm_vals[ADC_LEFT], norm_vals[ADC_RIGHT]);
		////////////////////END:ADC///////////////////////////////

		////////////////////START:Serial//////////////////////////
		
		memset(&UART_buf[0], 0, sizeof(UART_buf));
		FD_ZERO(&set); /* clear the set */
		FD_SET(uart_fd, &set); /* add our file descriptor to the set */
		timeout.tv_sec = 0;
		timeout.tv_usec = 200000; // 200 milliseconds 
		btdata_ready = select(uart_fd + 1, &set, NULL, NULL, &timeout);
		
		////////////////////START:Logic///////////////////////////
		printf("M: %d\t", tankMode);
		if (btdata_ready > 0) {	// BT command received
			res = read(uart_fd, UART_buf, sizeof(UART_buf));
			UART_buf[res] = 0;
			if (strstr(UART_buf, "n") != NULL) {
				printf("NM            ");
				commandbuff[0] = '\0';
				tankMode = NORMAL;
			} else if(strstr(UART_buf, "v") != NULL) { 
				printf("AM            ");
				commandbuff[0] = '\0';
				tankMode = AVERSION;
			} else if (strstr(UART_buf, "q") != NULL) {
				printf("Exiting\n");
				close(uart_fd);
				return 0;
			}

			// Normal mode
			if (tankMode == NORMAL) { //normalmode 
				if (strstr(UART_buf, "f") != NULL) {
					if (!proximity_check(FRONT)) {
						printf("S1       ");
						strcat(commandbuff, "f");
						// send_string(clear);
						send_sig(lastDir, SIG_USER_FORWARD); 
						lastDir = FRONT;
					} else {
						send_string(hazard);
						printf("F1       ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = STATIONARY;
					}
				} else if (strstr(UART_buf, "b") != NULL) {
					if(!proximity_check(BACK)) {
						printf("S2       ");
						strcat(commandbuff,"f");
						send_sig(lastDir, SIG_USER_BACKWARD); 
						lastDir = BACK;
						// send_string(clear);
					} else {
						send_string(hazard);
						printf("F2       ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = STATIONARY;
					}
				} else if (strstr(UART_buf, "l") != NULL) {
					if(!proximity_check(LEFT)) {
						printf("S3       ");
						strcat(commandbuff,"l");
						send_sig(lastDir, SIG_USER_LEFT); 
						lastDir = LEFT;
						// send_string(clear);
					} else {
						send_string(hazard);
						printf("F3       ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = STATIONARY;
					}					
				} else if (strstr(UART_buf, "r") != NULL) {
					if(!proximity_check(RIGHT)) {
						printf("S4       ");
						strcat(commandbuff,"r");
						send_sig(lastDir, SIG_USER_RIGHT); 
						lastDir = RIGHT;
						// send_string(clear);
					} else {
						send_string(hazard);
						printf("F4     ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = STATIONARY;
					}
				} 
			} else { // bt command decomposed in aversion mode
				lastDir = avert(lastDir);
			}
		} else { // no bt command
			if (tankMode == NORMAL) {
				printf("PC: ");
				temp = 0;
				for(i = 0; i < DEPTH_LEN; i ++) {
					printf("%d", proximity_check(i + 1));
				}
				send_sig(lastDir, SIG_USER_STANDBY);
				lastDir = STATIONARY;
			} else {
				lastDir = avert(lastDir);
			}
		}
		printf("\n");
		usleep(MAX(1000, (uTs - timeout.tv_usec))); // sleep for remainder of sample period (limit 1 ms)  
		////////////////////END:Logic//////////////////////////
	}
	return 0;
}

// Sets up serial connection with Bluetooth
void uart_Init() {
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

// Checking the distance sensor, index, for obstruction, returning 1 if true 0 otherwise   
int proximity_check(int index) {
	switch(index) {
		case 1: //check forward dir
			return (norm_vals[ADC_FRONT] > (norm * threshold_val / 100));
		case 2: //check backward dir
			return (norm_vals[ADC_BACK] > (norm * threshold_val / 100));
		case 3: //check left/front dir
			return ((norm_vals[ADC_FRONT] > (norm * threshold_val / 100)) || (norm_vals[ADC_LEFT] > (norm * threshold_val / 100)));
		case 4: //check right/front dir
			return ((norm_vals[ADC_FRONT] > (norm * threshold_val / 100)) || (norm_vals[ADC_RIGHT] > (norm * threshold_val / 100)));
		default: //default dir
			return (norm_vals[index] > (norm * 100 / threshold_val));
	}
}

//checks all adc values and sends appropriate signals as wells as updates last direction taken
int avert(int state) {
	int thresh = (norm * threshold_val / 100);
	if ((norm_vals[ADC_BACK] > thresh && norm_vals[ADC_FRONT] <= thresh)) {// 
		// Check sides of tank
		if (norm_vals[ADC_LEFT] > thresh && norm_vals[ADC_RIGHT] <= thresh) {
			printf("Av: FR       ");
			send_sig(state, SIG_USER_RIGHT);
			return RIGHT;
		} else if(norm_vals[ADC_RIGHT] > thresh && norm_vals[ADC_LEFT] <= thresh) {
			printf("Av: FL      ");
			send_sig(state, SIG_USER_LEFT);
			return LEFT;
		} else { // either sides are blocked or completely free - either way go forward
			printf("Av: FF       ");			
			send_sig(state, SIG_USER_FORWARD);
			return FRONT; 
		}
	} else if (norm_vals[ADC_FRONT] > thresh && norm_vals[ADC_BACK] <= thresh) {
		printf("Av: BB       ");
		send_sig(state, SIG_USER_BACKWARD);
		return BACK;
	} else { 
		printf("Av: N/A         ");
		send_sig(state, SIG_USER_STANDBY);
		return STATIONARY; 
	}
}

//Checks if last signal sent is same as current signal 
void send_sig(int state, int sig_user) {
	if(mode[state] != sig_user) {	
		printf("Signal Sent\n");
		kill(pid, sig_user);
	}
}

void send_string(char *str) {
	int i;
	for (i = 0; i < 17; i++) {
		write(uart_fd, str, 1);
		str++;
	}
}