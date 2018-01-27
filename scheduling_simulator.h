#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#include "task.h"

#define STACK_SIZE 50000

enum TASK_STATE {
	TASK_RUNNING,
	TASK_READY,
	TASK_WAITING,
	TASK_TERMINATED
};

struct task_t {
	enum TASK_STATE state;
	int time_quantum;
	int queue_time;
	int suspend_time;
	int run_time;
	char task_name[6];
	ucontext_t context;
	char st[STACK_SIZE];
	int removed;
};

void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);
int all_terminated(void);
void ctrlz(void);
void timer_1ms(void);
void simulation(void);
int get_remove_pid(char s[]);

int task_num;
int running_pid;

struct task_t task[1000];
ucontext_t shell_mode, simulation_mode, scheduler;
static char state_str[4][20] = {"TASK_RUNNING",
                                "TASK_READY",
                                "TASK_WAITING",
                                "TASK_TERMINATED"
                               };

#endif
