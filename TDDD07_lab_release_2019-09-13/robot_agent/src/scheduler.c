/**
 * @file	scheduler.c
 * @author  Eriks Zaharans and Massimiiliano Raciti
 * @date    1 Jul 2013
 *
 * @section DESCRIPTION
 *
 * Cyclic executive scheduler library.
 */

/* -- Includes -- */
/* system libraries */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
/* project libraries */
#include "scheduler.h"
#include "task.h"
#include "timelib.h"

/* -- Defines -- */
volatile sig_atomic_t stop;

/* -- Functions -- */

/**
 * Initialize cyclic executive scheduler
 * @param minor Minor cycle in miliseconds (ms)
 * @return Pointer to scheduler structure
 */
scheduler_t *scheduler_init(void)
{
	// Allocate memory for Scheduler structure
	scheduler_t *ces = (scheduler_t *)malloc(sizeof(scheduler_t));

	return ces;
}

/**
 * Deinitialize cyclic executive scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_destroy(scheduler_t *ces)
{
	// Free memory
	free(ces);
}

/**
 * Start scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_start(scheduler_t *ces)
{
	// Set timers
	timelib_timer_set(&ces->tv_started);
	timelib_timer_set(&ces->tv_cycle);
}

/**
 * Wait (sleep) till end of minor cycle
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_wait_for_timer(scheduler_t *ces)
{
	int sleep_time; // Sleep time in microseconds

	// Calculate time till end of the minor cycle
	sleep_time = (ces->minor * 1000) - (int)(timelib_timer_get(ces->tv_cycle) * 1000);

	// Add minor cycle period to timer
	timelib_timer_add_ms(&ces->tv_cycle, ces->minor);

	// Check for overrun and execute sleep only if there is no
	if (sleep_time > 0)
	{
		// Go to sleep (multipy with 1000 to get miliseconds)
		usleep(sleep_time);
	}
}

/**
 * Execute task
 * @param ces Pointer to scheduler structure
 * @param task_id Task ID
 * @return Void
 */
void scheduler_exec_task(scheduler_t *ces, int task_id)
{
	switch (task_id)
	{
	// Mission
	case s_TASK_MISSION_ID:
		task_mission();
		break;
	// Navigate
	case s_TASK_NAVIGATE_ID:
		task_navigate();
		break;
	// Control
	case s_TASK_CONTROL_ID:
		task_control();
		break;
	// Refine
	case s_TASK_REFINE_ID:
		task_refine();
		break;
	// Report
	case s_TASK_REPORT_ID:
		task_report();
		break;
	// Communicate
	case s_TASK_COMMUNICATE_ID:
		task_communicate();
		break;
	// Collision detection
	case s_TASK_AVOID_ID:
		task_avoid();
		break;
	// Other
	default:
		// Do nothing
		break;
	}
}

void signal_handeling()
{
	stop = 1;
	printf("Interrupt Signal sent.\n");
}

/**
 * Run scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_run(scheduler_t *ces)
{
	/* --- Local variables (define variables here) --- */
	signal(SIGINT, signal_handeling);
	struct timeval start;

	/* --- Set minor cycle period --- */
	ces->minor = 1 * 100;

	/* --- Write your code here --- */
	printf("Scheduler Running.\n");

	// Start scheduler
	scheduler_start(ces);

	while (!stop)
	{
		timelib_timer_set(&start);

		scheduler_exec_task(ces, s_TASK_MISSION_ID);
		double exec_time1 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_NAVIGATE_ID);
		double exec_time2 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_CONTROL_ID);
		double exec_time3 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_REPORT_ID);
		double exec_time4 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_REFINE_ID);
		double exec_time5 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_AVOID_ID);
		double exec_time6 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_COMMUNICATE_ID);
		double exec_time7 = timelib_timer_get(start);

		printf("exec_time1 %f ms.\n", exec_time1);
		printf("exec_time2 %f ms.\n", exec_time2);
		printf("exec_time3 %f ms.\n", exec_time3);
		printf("exec_time4 %f ms.\n", exec_time4);
		printf("exec_time5 %f ms.\n", exec_time5);
		printf("exec_time6 %f ms.\n", exec_time6);
		printf("exec_time7 %f ms.\n\n", exec_time7);

		//scheduler_wait_for_timer(ces);
	}
}
