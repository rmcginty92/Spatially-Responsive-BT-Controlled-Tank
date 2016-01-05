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
		ain = fopen(DTBOLOAD_FILE, "w"); // use this for older version of debian on bot's micro sd card
		fseek(ain, 0, SEEK_SET);
		fprintf(ain, "BB-UART1"); // use this for micro sd card
		fflush(ain);
		fclose(ain);
	}
	uart_Init();
	pid = atoi(argv[1]);



	// Initializing variables
	tankMode = 1; // 1->NormalMode | 0->AversionMode
	lastDir = 0; // 0->staionary | 1->forward | 2->back | 3->left | 4->right
	mknod(sensor_fifo, S_IFIFO | 0666, 0); // ADC FIFO file creation
	
	while(1){ 
		////////////////////START:ADC///////////////////////////
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
		// Converting all values into integer scale from 0 to 1000
		for (i = 0; i < DEPTH_LEN; i++) {
			norm_vals[i] = (int) (value[i] * norm) / (max_adc_range[i] - min_adc_range[i]);
		}
		printf("ADC Values:\t1) %d\t2) %d\t3) %d\t4) %d\t",norm_vals[0],norm_vals[1],norm_vals[2],norm_vals[3]);
		////////////////////END:ADC///////////////////////////////

		////////////////////START:Serial//////////////////////////
		
		memset(&UART_buf[0], 0, sizeof(UART_buf));
		FD_ZERO(&set); /* clear the set */
		FD_SET(uart_fd, &set); /* add our file descriptor to the set */
		timeout.tv_sec = 0;
		timeout.tv_usec = 200000; // 200 milliseconds 
		btdata_ready = select(uart_fd + 1, &set, NULL, NULL, &timeout);
		
		////////////////////START:Serial//////////////////////////	
		////////////////////START:Logic///////////////////////////
		printf("M: %d\t", tankMode);
		if (btdata_ready > 0) {	// BT command received
			res = read(uart_fd, UART_buf, sizeof(UART_buf));
			UART_buf[res] = 0;
			if (strstr(UART_buf, "n") != NULL) {
				printf("NM            ");
				commandbuff[0] = '\0';
				tankMode = 1;
			} else if(strstr(UART_buf, "v") != NULL) { //aversion
				printf("AM            ");
				commandbuff[0] = '\0';
				tankMode = 0;
			} else if (strstr(UART_buf, "q") != NULL) { // quit
				printf("Exiting\n");
				close(uart_fd);
				return 0;
			}
			//Normal mode
			if (tankMode == 1) { //normalmode 
				if (strstr(UART_buf, "f") != NULL) {
					if(!proximity_check(1)) {
						printf("S1       ");
						strcat(commandbuff,"f");
						send_sig(lastDir, SIG_USER_FORWARD); 
						lastDir = 1;
					} else {
						printf("F1       ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = 0;
					}
				} else if (strstr(UART_buf, "b") != NULL) {
					if(!proximity_check(2)) {
						printf("S2       ");
						strcat(commandbuff,"f");
						send_sig(lastDir, SIG_USER_BACKWARD); 
						lastDir = 2;
					} else {
						printf("F2       ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = 0;
					}
				} else if (strstr(UART_buf, "l") != NULL) {
					if(!proximity_check(3)) {
						printf("S3       ");
						strcat(commandbuff,"l");
						send_sig(lastDir, SIG_USER_LEFT); 
						lastDir = 3;
					} else {
						printf("F3       ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = 0;
					}					
				} else if (strstr(UART_buf, "r") != NULL) {
					if(!proximity_check(4)) {
						printf("S4       ");
						strcat(commandbuff,"r");
						send_sig(lastDir, SIG_USER_RIGHT); 
						lastDir = 4;
					} else {
						printf("F4     ");
						send_sig(lastDir, SIG_USER_STANDBY); 
						lastDir = 0;
					}
				} else if (strstr(UART_buf, "x") != NULL) {
					printf("RT       ");
					printf("\nRetrace:%s",commandbuff);
				} 
			} else { // bt command decomposed in aversion mode
				lastDir = avert(lastDir);
			}
		} else { // no bt command
			if(tankMode == 1) {
				printf("PC: ");
				temp = 0;
				for(i = 0; i < DEPTH_LEN; i ++) {
					printf("%d",proximity_check(i+1));
				}
				send_sig(lastDir,SIG_USER_STANDBY);
				lastDir = 0;
			} else {
				lastDir = avert(lastDir);
			}
		}
		printf("\n");
		usleep(MAX(1000,(uTs - timeout.tv_usec))); // sleep for remainder of sample period (limit 1 ms)  
		////////////////////END:Logic//////////////////////////
	}
	return 0;
}

//Sets up Serial connection with Bluetooth Connection
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
			return (norm_vals[0] > (norm*threshold_val/100));
		case 2: //check backward dir
			return (norm_vals[1] > (norm*threshold_val/100));
		case 3: //check left/front dir
			return ((norm_vals[0] > (norm*threshold_val/100)) || (norm_vals[2] > (norm*threshold_val/100)));
		case 4: //check right/front dir
			return ((norm_vals[0] > (norm*threshold_val/100)) || (norm_vals[3] > (norm*threshold_val/100)));
		default: //default dir
			return (norm_vals[index] > (norm*100 / threshold_val));
	}
}

//checks all adc values and sends appropriate signals as wells as updates last direction taken
int avert(int state) {
	int thresh = (norm*threshold_val/100);
	if ((norm_vals[1] > thresh && norm_vals[0] <= thresh)) {// 
		// Check sides of tank
		if (norm_vals[2] > thresh && norm_vals[3] <= thresh) {
		   printf("Av: FR       ");
		   send_sig(state, SIG_USER_LEFT);
		   return 3; // state is now 3
		} else if(norm_vals[3] > thresh && norm_vals[2] <= thresh) {
			printf("Av: FL      ");
		   send_sig(state, SIG_USER_RIGHT);
		   return 4;
		} else { // either sides are blocked or completely free - either way go forward
		   printf("Av: FF       ");			
		   send_sig(state, SIG_USER_FORWARD);
		   return 1; 
		}
	} else if (norm_vals[0] > thresh && norm_vals[1] <= thresh) {
		printf("Av: BB       ");
		// No back-left or back-right funcstionality yet, so just goes backward
		send_sig(state, SIG_USER_BACKWARD);
		return 2;
	} else { 
		printf("Av: N/A         ");
		send_sig(state, SIG_USER_STANDBY);
		return 0; 
	}
}

//Checks if last signal sent is same as current signal 
void send_sig(int state, int sig_user) {
	if(mode[state] != sig_user) {	
		printf("Signal Sent\n");
		kill(pid, sig_user);
	}
}