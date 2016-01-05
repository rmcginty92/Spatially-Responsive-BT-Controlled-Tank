#include "master.h"

int main(int argc, char *argv[]) {
    // bind handler to signal interrupt
    
    Fs = 40;
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
    int fd;
    int success, i;
    int value[DEPTH_LEN];
    size_t size = sizeof(buf);

    mknod(sensor_fifo, S_IFIFO | 0666, 0);
    fd = open(sensor_fifo, O_RDONLY);
    success = read(fd, buf, size); 
    close(fd);

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

    printf("*******************************************************************\n");
    printf("pin[0] is: %d\n", value[0]);
    printf("pin[1] is: %d\n", value[1]);
    printf("pin[2] is: %d\n", value[2]);
    printf("pin[3] is: %d\n", value[3]);
    printf("*******************************************************************\n");

    // ******************************************
    // Control Logic

    // Converting all values into integer scale from 0 to 1000
    for (i = 0; i < DEPTH_LEN; i++) {
        norm_vals[i] = (int) (value[i] * norm) / (max_adc_range[i] - min_adc_range[i]);
    }

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
        // No back-left or back-right functionality yet, so just goes backward
        isStandby = 0;
    	kill(pid, SIG_USER_BACKWARD);
    } else { 
        if (!isStandby) {
    	   kill(pid, SIG_USER_STANDBY);
           //kill(pid, SIG_USER_BRAKE); 
           isStandby = 1;
        }
    }
    // ******************************************
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




