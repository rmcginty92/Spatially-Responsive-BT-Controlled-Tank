#ifndef MASTER_H
#define MASTER_H

#include <stdlib.h>
#include <stdio.h>  
#include <signal.h>
#include <time.h>  
#include <stddef.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/time.h> 

#define NORMAL 1
#define AVERSION 0

#define STATIONARY 0
#define FRONT 1
#define BACK 2
#define LEFT 3 
#define RIGHT 4

#define ADC_FRONT 0
#define ADC_BACK 1
#define ADC_LEFT 2
#define ADC_RIGHT 3

#define SIG_USER 20
#define DEPTH_LEN 4
#define BUFF_LEN 20
#define MAX_BUFF 512
#define Fs 4 // hz

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


// H-Bridge Signal definition
#define SIG_USER_LEFT 21
#define SIG_USER_RIGHT 22
#define SIG_USER_FORWARD 23
#define SIG_USER_BACKWARD 24
#define SIG_USER_BRAKE 25
#define SIG_USER_STANDBY 26

//Bluetooth Definitions
#define UARTPORT "/dev/ttyO1"
#define BAUDRATE B9600

#define STRINGLEN 19

static char UART_buf[255];

static char *hazard = "HAZARD DETECTED\n\0";
static char *clear =  "BOT FREE TO MOVE\n\0";
static char *sensor_fifo = "/tmp/adc_vals";
static char *DTBOLOAD_FILE = "/sys/devices/bone_capemgr.9/slots";

static int pid, uart_fd, tankMode, lastDir;

/// Control variables ///
// Buffer range of ADC values. Used to normalized process of signal analysis 
static int min_adc_range[DEPTH_LEN] = {0, 0, 0, 0};
static int max_adc_range[DEPTH_LEN] = {2000, 2000, 2000, 2000};
static int isStandby = 0;
static int norm = 1000; // Scalar value for normalizing ADC values -> range [ 0 , 1000 ]
static int threshold_val = 75; // 50 % of 
static int norm_vals[DEPTH_LEN] = {0, 0, 0, 0}; // Buffer for normalized values 
static int mode[5] = {SIG_USER_STANDBY, SIG_USER_FORWARD, SIG_USER_BACKWARD, SIG_USER_LEFT, SIG_USER_RIGHT};
// /// /// /// //

// variables for timers and interrupts
static struct sigaction action;
static sigset_t signals;

void send_string(char *);
void uart_Init();
int proximity_check(int);
int avert(int);
void send_sig(int, int);

#endif