#ifndef ACD_H
#define ACD_H

#include <stdlib.h>
#include <stdio.h>  
#include <signal.h>
#include <time.h>  
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/time.h> 
 
#define SIG_USER 20
#define BUFF_LEN 20
#define DEPTH_LEN 4

static int ADC_BUFFS[DEPTH_LEN][BUFF_LEN]; // {Front, Back, Left, Right}
static int ibuff, Fs;
static int adc_vals[DEPTH_LEN]; // {Front, Back, Left, Right}

// creating temporary file name
static int fd; // {Front, Back, Left, Right}
static char *sensor_fifo = "/tmp/adc_vals";
static const char *ADCpaths[DEPTH_LEN] = {"/sys/bus/iio/devices/iio:device0/in_voltage0_raw",
										  "/sys/bus/iio/devices/iio:device0/in_voltage1_raw",
										  "/sys/bus/iio/devices/iio:device0/in_voltage2_raw",
										  "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"};

static char *error_Msg = "Error opening file, checking if file exists.\n";

// variables for general interrupts
static int pid;
static struct sigaction action;
static sigset_t signals;

static struct sigaction pipe_action;
static sigset_t pipe_signals;

// variables for timers
static timer_t timerid;
static struct itimerspec timeout;
static struct sigevent sigevent;
static clockid_t clkid;

// declaring functions
int getAverage(int [], int);
void pipe_handler(int);
void timer_handler(int);
int timer_Init(int, void (*)(int));
int pipe_Init(int, void (*)(int));

#endif