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
#include <math.h>
/* project libraries */
#include "scheduler.h"
#include "task.h"
#include "timelib.h"

/* -- Defines -- */
volatile sig_atomic_t stop;
#define NBR_TASKS 7
#define MAJOR_CYCLE 3000

int minor_cycle_overrun = 0;

struct victims_info_struct
{
	char *id;
	int x;
	int y;
};
struct victims_info_struct victims_info[24] = {{"020058F5BD", 340, 340},
											   {"020053A537", 975, 1115},
											   {"020053E0BA", 1845, 925},
											   {"01004B835E", 2670, 355},
											   {"020053C80E", 3395, 870},
											   {"020058100D", 4645, 910},
											   {"0200580B96", 4800, 250},
											   {"02005345B6", 5395, 1060},
											   {"020058F121", 5830, 1895},
											   {"0200581B9E", 5110, 2390},
											   {"020058066F", 5770, 3790},
											   {"020058212D", 4500, 3190},
											   {"020058022D", 4315, 3200},
											   {"0200581542", 4150, 1810},
											   {"0200534E5C", 3720, 3710},
											   {"020053AB2C", 2580, 3770},
											   {"01004A11E8", 2970, 2805},
											   {"020053E282", 3030, 2070},
											   {"0200553505", 3120, 1965},
											   {"01004751A2", 2880, 1840},
											   {"02005097C0", 1890, 2580},
											   {"020053BF78", 985, 3020},
											   {"020056D0EF", 730, 3175},
											   {"01004BDF7B", 320, 1800}};

/* -- Functions -- */
float victim_distance(char *id, int x, int y)
{
	for (int i = 0; i < 24; i++)
	{
		if (strcmp(id, victims_info[i].id) == 0)
			return sqrt(pow(victims_info[i].x - abs(x), 2) + pow(victims_info[i].y - abs(y), 2));
	}
}
/**
 * Initialize cyclic executive scheduler
 * @param minor Minor cycle in miliseconds (ms)
 * @return Pointer to scheduler structure
 */
scheduler_t *
scheduler_init(void)
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
	int i = 0;

	/* --- Set minor cycle period --- */
	ces->minor = 100;

	/* --- Write your code here --- */

	int nbr_of_minor_cycles = MAJOR_CYCLE / ces->minor;
	printf("Scheduler Running.\n");

	// Start scheduler
	scheduler_start(ces);

	while (!stop)
	{

		for (i = 1; i <= nbr_of_minor_cycles; i++)
		{

			timelib_timer_set(&start);

			if (i % 10 == 0)
				scheduler_exec_task(ces, s_TASK_COMMUNICATE_ID);

			scheduler_exec_task(ces, s_TASK_REFINE_ID);

			scheduler_exec_task(ces, s_TASK_REPORT_ID);

			scheduler_exec_task(ces, s_TASK_MISSION_ID);

			if (i % 3 == 0)
				scheduler_exec_task(ces, s_TASK_CONTROL_ID);

			/* 			scheduler_exec_task(ces, s_TASK_REFINE_ID);

			scheduler_exec_task(ces, s_TASK_REPORT_ID);

			scheduler_exec_task(ces, s_TASK_MISSION_ID); */

			scheduler_exec_task(ces, s_TASK_AVOID_ID);

			scheduler_exec_task(ces, s_TASK_NAVIGATE_ID);

			/* 			scheduler_exec_task(ces, s_TASK_REFINE_ID);

			scheduler_exec_task(ces, s_TASK_REPORT_ID);

			scheduler_exec_task(ces, s_TASK_MISSION_ID); */

			double end = timelib_timer_get(start);

			printf("end time : %lf\n", end);

			if (end > ces->minor)
				minor_cycle_overrun++;

			scheduler_wait_for_timer(ces);
		}

		/* 		 timelib_timer_set(&start);

		scheduler_exec_task(ces, s_TASK_MISSION_ID);
		double exec_time1 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_NAVIGATE_ID);
		double exec_time2 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_CONTROL_ID);
		double exec_time3 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_AVOID_ID);
		double exec_time4 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_REFINE_ID);
		double exec_time5 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_REPORT_ID);
		double exec_time6 = timelib_timer_get(start);

		scheduler_exec_task(ces, s_TASK_COMMUNICATE_ID);
		double exec_time7 = timelib_timer_get(start);

		printf("s_TASK_MISSION_ID %f ms.\n", exec_time1);
		printf("s_TASK_NAVIGATE_ID %f ms.\n", exec_time2 - exec_time1);
		printf("s_TASK_CONTROL_ID %f ms.\n", exec_time3 - exec_time2);
		printf("s_TASK_AVOID_ID %f ms.\n", exec_time4 - exec_time3);
		printf("s_TASK_REFINE_ID %f ms.\n", exec_time5 - exec_time4);
		printf("s_TASK_REPORT_ID %f ms.\n", exec_time6 - exec_time5);
		printf("s_TASK_COMMUNICATE_ID %f ms.\n\n", exec_time7 - exec_time6); */
	}

	if (g_task_mission_data.victim_count > 0)
	{
		float average_distance = 0;
		for (int j = 0; j < g_task_mission_data.victim_count; j++)
		{
			float distance = victim_distance(g_task_mission_data.victims[j].id, g_task_mission_data.victims[j].x, g_task_mission_data.victims[j].y);
			average_distance += distance;
			printf(" id : %s x : %d y : %d distance : %f\n", g_task_mission_data.victims[j].id, g_task_mission_data.victims[j].x, g_task_mission_data.victims[j].y, distance);
		}
		printf("Average distance = %f\n", average_distance / g_task_mission_data.victim_count);
	}

	printf("minor_cycle_overrun  : %d\n", minor_cycle_overrun);
}
