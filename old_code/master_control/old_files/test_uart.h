#ifndef TEST_UART_H
#define TEST_UART_H

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

#define SIG_USER 20
#define DEPTH_LEN 4
#define BUFF_LEN 20
#define MAX_BUFF 1024

// H-Bridge Signal definition
#define SIG_USER_LEFT 21
#define SIG_USER_RIGHT 22
#define SIG_USER_FORWARD 23
#define SIG_USER_BACKWARD 24
#define SIG_USER_BRAKE 25
#define SIG_USER_STANDBY 26

//Bluetooth Definitions
#define UARTPORT "/dev/ttyO1"
#define BAUDRATE B115200
static char UART_buf[255];


static char *sensor_fifo = "/tmp/adc_vals";
static char *DTBOLOAD_FILE = "/sys/devices/bone_capemgr.9/slots";
static int pid, Fs, uart_fd, isCalibrated;

/// Control variables ///
// Buffer range of ADC values. Used to normalized process of signal analysis 
static int min_adc_range[DEPTH_LEN] = {0,0,0,0};
static int max_adc_range[DEPTH_LEN] = {2000,2000,2000,2000};
static int isStandby = 0;
static int norm = 1000; // Scalar value for normalizing ADC values -> range [ 0 , 1000 ]
static int threshold_val = 500; // 50 % of 
static int norm_vals[DEPTH_LEN] = {0,0,0,0}; // Buffer for normalized values 
// /// /// /// //

// variables for timers and interrupts
static timer_t timerid;
static struct itimerspec timeout;
static struct sigevent sigevent;
static clockid_t clkid;

static struct sigaction action;
static sigset_t signals;

void timer_handler(int);
int timer_Init(int, void (*)(int));
void uart_Init();

#endif