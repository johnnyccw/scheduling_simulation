#include "scheduling_simulator.h"

void hw_suspend(int msec_10)
{
	task[running_pid].suspend_time = msec_10*10;
	task[running_pid].state = TASK_WAITING;
	swapcontext(&task[running_pid].context, &scheduler);
	return;
}

void hw_wakeup_pid(int pid)
{
	task[pid].suspend_time = 0;
	return;
}

int hw_wakeup_taskname(char *task_name)
{
	int i;
	int count = 0;
	for (i = 0; i < task_num; i++) {
		if(strcmp(task[i].task_name,task_name)==0 && task[i].state==TASK_WAITING) {
			count++;
			hw_wakeup_pid(i);
		}
	}
	return count;

}

int hw_task_create(char *task_name)
{
	//task initialization
	task[task_num].state = TASK_READY;
	task[task_num].time_quantum = 10;
	task[task_num].queue_time = 0;
	task[task_num].suspend_time = 0;
	task[task_num].run_time = 0;
	strcpy(task[task_num].task_name,task_name);
	getcontext(&task[task_num].context);
	task[task_num].context.uc_link = &scheduler;
	task[task_num].context.uc_stack.ss_sp = task[task_num].st;
	task[task_num].context.uc_stack.ss_size = STACK_SIZE;
	if(task[task_num].task_name[4]=='1') {
		makecontext(&task[task_num].context, task1,0);
	} else if(task[task_num].task_name[4]=='2') {
		makecontext(&task[task_num].context, task2,0);
	} else if(task[task_num].task_name[4]=='3') {
		makecontext(&task[task_num].context, task3,0);
	} else if(task[task_num].task_name[4]=='4') {
		makecontext(&task[task_num].context, task4,0);
	} else if(task[task_num].task_name[4]=='5') {
		makecontext(&task[task_num].context, task5,0);
	} else if(task[task_num].task_name[4]=='6') {
		makecontext(&task[task_num].context, task6,0);
	} else {
		return -1;
	}
	task[task_num].removed = 0;
	task_num++;
	return task_num-1; // the pid of created task name
}

int all_terminated()
{
	if(task_num==0) {
		return 1;
	}
	int i;
	for (i = 0; i < task_num; i++) {
		if(task[i].state!=TASK_TERMINATED) {
			return 0;
		}
	}
	return 1;
}

void ctrlz()
{
	//stop timer and record its remained value
	int remain = ualarm(0, 0);
	//ignore sigtstp
	signal(SIGTSTP, SIG_IGN);
	//go to shell mode
	swapcontext(&simulation_mode, &shell_mode);
	//connect sigtstp to its handler
	signal(SIGTSTP, ctrlz);
	//resume timer
	ualarm(remain,1000);
	return;
}

void timer_1ms()
{
	int i;
	for (i = 0; i < task_num; i++) {
		if(task[i].state == TASK_READY) {
			task[i].queue_time++;
		} else if(task[i].state == TASK_WAITING) {
			task[i].suspend_time--;
			if(task[i].suspend_time <= 0) {
				task[i].suspend_time = 0;
				task[i].state = TASK_READY;
			}
		} else if(task[i].state == TASK_RUNNING) {
			task[running_pid].run_time ++;
		}
	}

	if(task[running_pid].run_time>=task[running_pid].time_quantum) {
		task[running_pid].run_time = 0;
		task[running_pid].state = TASK_READY;
		swapcontext(&task[running_pid].context,&scheduler);
	}
	return;
}

void simulation()
{
	int i;
	signal(SIGTSTP, ctrlz);
	signal(SIGALRM, timer_1ms);
	ualarm(1000, 1000);
	while(1) {
		if(task[i].state==TASK_READY) {
			task[i].state = TASK_RUNNING;
			running_pid = i;
			//run task
			swapcontext(&scheduler, &task[i].context);
			/**************************
			if return due to time expired ,task's state will be TASK_READY.
			else it will be TASK_RUNNING, which means task is finished and come back here through uc_link.
			***************************/
			if (task[i].state == TASK_RUNNING) {
				task[i].state = TASK_TERMINATED;
			}
		}
		//if all tasks are terminated, go to shell mode
		if(all_terminated()==1) {
			i = 0;
			ctrlz();
		}
		//avoid floating point exception
		if(task_num!=0) {
			i = (i + 1) % task_num;
		}
	}
}

int get_remove_pid(char s[])
{
	char *pid;
	pid = s+7;
	return atoi(pid);
}

int main()
{
	char st[STACK_SIZE];
	getcontext(&simulation_mode);
	simulation_mode.uc_link = &shell_mode;
	simulation_mode.uc_stack.ss_sp = st;
	simulation_mode.uc_stack.ss_size = STACK_SIZE;
	makecontext(&simulation_mode, simulation, 0);

	int i;
	task_num = 0;
	char input[20];
	char task_name[6] = "task";
	printf("shell mode\n");
	while (1) {
		printf("$");
		strcpy(input,"");
		strcpy(task_name,"task");
		gets(input);
		if (input[0] == 'a') {
			task_name[4] = input[8];
			hw_task_create(task_name);
			if(input[13]=='L') {
				task[task_num-1].time_quantum = 20;
			}
		} else if(input[0]=='p') {
			for (i = 0; i < task_num; i++) {
				if(task[i].removed==0) {
					printf("%-3d %s   %-15s   %-8d\n",i,task[i].task_name,state_str[task[i].state],
					       task[i].queue_time);
				}
			}
		} else if(input[0]=='s') {
			printf("simulating ...\n");
			fflush(stdout);
			//go to simulation mode
			swapcontext(&shell_mode,&simulation_mode);
			printf("\nshell mode\n");
		} else if(input[0]=='r') {
			task[get_remove_pid(input)].state = TASK_TERMINATED;
			task[get_remove_pid(input)].removed = 1;
		}
	}

	return 0;
}
