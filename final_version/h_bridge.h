#ifndef H_BRIDGE_H
#define H_BRIDGE_H

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define PWMA 66 
#define AIN2 69 
#define AIN1 45 

#define PWMB 67 // pwm2
#define BIN2 68 // bin2
#define BIN1 44 // bin1

#define STANDBY 61

#define LOW 0
#define HIGH 1

#define SIG_USER_LEFT 21
#define SIG_USER_RIGHT 22
#define SIG_USER_FORWARD 23
#define SIG_USER_BACKWARD 24
#define SIG_USER_BRAKE 25
#define SIG_USER_STANDBY 26

static FILE *sys;

static FILE *dir_pwma, *dir_pwmb;
static FILE *dir_ain1, *dir_ain2;
static FILE *dir_bin1, *dir_bin2;
static FILE *dir_standby;

static FILE *value_pwma, *value_pwmb;
static FILE *value_ain1, *value_ain2;
static FILE *value_bin1, *value_bin2;
static FILE *value_standby;

static struct sigaction left_action;
static sigset_t left_signals;

static struct sigaction right_action;
static sigset_t right_signals;

static struct sigaction forward_action;
static sigset_t forward_signals;

static struct sigaction backward_action;
static sigset_t backward_signals;

static struct sigaction brake_action;
static sigset_t brake_signals;

static struct sigaction standby_action;
static sigset_t standby_signals;

void moveCW(FILE*, FILE*, FILE*, FILE*);
void moveCCW(FILE*, FILE*, FILE*, FILE*);
void brake(FILE*, FILE*, FILE*, FILE*);
void stop(FILE*, FILE*, FILE*, FILE*);
void sysStandby(FILE*);

void writeGPIO(FILE*, int);
void writeDirGPIO(FILE*, char*);

void left_handler(int);
void right_handler(int);
void forward_handler(int);
void backward_handler(int);
void brake_handler(int);
void standby_handler(int);

int left_Init(int, void (*)(int));
int right_Init(int, void (*)(int));
int forward_Init(int, void (*)(int));
int backward_Init(int, void (*)(int));
int brake_Init(int, void (*)(int));
int standby_Init(int, void (*)(int));

void setupInterrupt(int, struct sigaction, sigset_t, void (*)(int));

#endif