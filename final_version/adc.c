/*	ADC_PWM.c
 *	
*/
#include "adc.h"

int main() {
	FILE *ADC_files[4], *ain;
	int i, j, val;
	
	ain = fopen("/sys/devices/bone_capemgr.9/slots", "w"); // use this for older version of debian on bot's micro sd card
	fseek(ain, 0, SEEK_SET);
	fprintf(ain, "BB-ADC"); 
	fflush(ain);
	fclose(ain);

  	// initializing variables
  	ibuff = 0;
  	for(i = 0; i < DEPTH_LEN; i++) {
  		for(j = 0; j < BUFF_LEN; j++) {
  			ADC_BUFFS[i][j] = 0;
  		}
  	}

  	Fs = 40; // Hz
  	int Ts = (int)(10*10*10*10*10*10 / Fs); // converting to microseconds
	pid = getpid();

	timer_Init(SIGALRM, timer_handler);
	pipe_Init(SIGPIPE, pipe_handler);
	
	// collects values at each ADC pin each iteration
	while(1) {
		for(i = 0; i < DEPTH_LEN; i++) {
			// Opening ADC files, collecting values, and closing files
			ADC_files[i] = fopen(ADCpaths[i], "r");
			fscanf(ADC_files[i], "%d", &val);
			ADC_BUFFS[i][ibuff % BUFF_LEN] = val;
			fclose(ADC_files[i]);
		}
		usleep(Ts); // sleep for Ts microseconds
		ibuff = (ibuff + 1) % BUFF_LEN;
	}

	timer_delete(timerid);

	return 0;
}

// returns average of buffer array passed   
int getAverage(int buff[], int size) {
	int i, avg, sum;
	int total;
	sum = 0;
	total = size;
	for (i = 0; i < size; ++i) {	
		sum += buff[i];
	}
	avg = (int) sum / size;
	return avg;
}

// interrupt handler for timer
void timer_handler(int signum)
{
	int i, val;
	char adc_val_buff[30];
	char string[100];
	char num[5];

	for(i = 0; i < DEPTH_LEN; i++) {
		val = getAverage(ADC_BUFFS[i], BUFF_LEN);
		sprintf(adc_val_buff, "%d", val);
		if (!i) {
			sprintf(string, "%d", i); 
		} else {
			sprintf(num, "%d", i);
			strcat(string, num);
		}
		strcat(string, ":");
		strcat(string, adc_val_buff);
		strcat(string, ",");
	}

	if ((fd = open(sensor_fifo, O_WRONLY)) < 0) {
		printf("%s\n", error_Msg);
	};
	write(fd, string, sizeof(string));
	close(fd);
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

// interrupt handler for broken pipe issue
void pipe_handler(int signum) {
	printf("pipe failed\n");
}

// interrupt signals for broken pipe issue
int pipe_Init(int signum, void (*handler)(int)) {
	/* Unblock the signal passed in. */
	sigemptyset(&pipe_signals);
	sigaddset(&pipe_signals, signum);
	sigprocmask(SIG_UNBLOCK, &pipe_signals, NULL); // adds the signal passed in to the set of unblocked signals

	/* Install the signal handler. */
	pipe_action.sa_handler = handler;
	pipe_action.sa_flags = 0; // tells us that sa_handler instead of sa_sigaction should be used 
	sigemptyset(&pipe_action.sa_mask); // empties the set of signals which should be ignored
	sigaction(signum, &pipe_action, NULL); // examine and change a signal action to sigaction struct
}





