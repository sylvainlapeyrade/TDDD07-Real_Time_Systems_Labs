/**
 * @file	scheduler.c
 * @author  Eriks Zaharans and Massimiiliano Raciti
 *  Modified by Sylvain Lapeyrade & Reda Bourakkadi
 * 
 * @date    1 Jul 2013
 *  Last Update: 12/12/2019
 * 
 * @section DESCRIPTION
 *	TDDD07 Real Time Systems: Lab 1
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
#define MAJOR_CYCLE 3000 // Our major cycle in ms (<=>3 seconds)
volatile sig_atomic_t stop; 

struct victims_info_struct
{
	char *id; // Victim ID
	int x;	// Victim X position
	int y;	// Victim Y position
};

/* Id with x and y positions of the victims (taken from the pdf) */
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

/**
 * Catch the SIGINT signal and interrupt the main while functions
 */
void signal_handeling()
{
	stop = 1;
	printf("Interrupt Signal sent.\n");
}

/**
 * Compute the euclidian distance between the real and measured victim position 
 * @param id Id of the victim
 * @param x X coordinate of the victim
 * @param y Y coordinate of the victim
 * @return The euclidian distance from the real and measured position of the victim
 */
float victim_distance(char *id, int x, int y)
{
	for (int i = 0; i < 24; i++)
	{
		if (strcmp(id, victims_info[i].id) == 0)
		{
			return sqrt(pow(victims_info[i].x - abs(x), 2) +
						pow(victims_info[i].y - abs(y), 2));
		}
	}
}

/**
 * Run scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_run(scheduler_t *ces)
{
	/* --- Local variables (define variables here) --- */
	signal(SIGINT, signal_handeling); // Signal used when CTRL+C
	struct timeval start_time;		  // Timer used to measure execution time
	int minor_cycle_overrun = 0;	  // Counter on minor cycle overrun

	// Get UNIX timestamp in miliseconds
	double current_second = timelib_unix_timestamp() / 1000; // To have second
	double time_to_next_second = ceil(current_second) - current_second;

	/* --- Set minor cycle period --- */
	ces->minor = 100; // Our minor cycle is 100 ms

	/* --- Write your code here --- */

	// Number of minor cycles : 3000/100 = 10
	printf("Scheduler Running.\n");

	int minor_cycles = MAJOR_CYCLE / ces->minor;
	// Time to wait until our next time slot for communicating (in seconds)
	double time_to_wait = time_to_next_second + 0.25; // 0.125 * (ID-1)

	// Sleep until our communication timeslot
	usleep(time_to_wait * 1e6);

	// Start scheduler
	scheduler_start(ces);

	while (!stop)
	{
		for (int i = 1; i <= minor_cycles; i++)
		{
			timelib_timer_set(&start_time); // Start the timer

			if (i == 1 || i == 11 || i == 21) // Communicate every second
			{
				printf("Communication start: %lf\n", timelib_unix_timestamp() / 1000);
				scheduler_exec_task(ces, s_TASK_COMMUNICATE_ID);
				printf("Communication end: %lf\n\n", timelib_unix_timestamp() / 1000);
			}

			scheduler_exec_task(ces, s_TASK_REFINE_ID);

			scheduler_exec_task(ces, s_TASK_REPORT_ID);

			scheduler_exec_task(ces, s_TASK_MISSION_ID);

			if (i == 1 || i == 11 || i == 21) // Control task every 1000 ms (can be a bit lower)
			{
				scheduler_exec_task(ces, s_TASK_CONTROL_ID);
			}

			scheduler_exec_task(ces, s_TASK_AVOID_ID);

			scheduler_exec_task(ces, s_TASK_NAVIGATE_ID);

			double end_time = timelib_timer_get(start_time); // Stop the timer

			// If execution time of the tasks took more time than the minor cycle
			if (end_time > ces->minor)
			{
				minor_cycle_overrun++; // Increment overrun number
			}

			scheduler_wait_for_timer(ces); // Wait until the end of the minor task
		}
	}

	// For each victims detected by the robot, print the id and coordinates and
	// print the distance from the actual position as well as the average distance
	if (g_task_mission_data.victim_count > 0)
	{
		float average_distance = 0;
		for (int j = 0; j < g_task_mission_data.victim_count; j++)
		{
			float distance = victim_distance(g_task_mission_data.victims[j].id,
											 g_task_mission_data.victims[j].x,
											 g_task_mission_data.victims[j].y);
			average_distance += distance;
			printf("Victim id : %s x : %d y : %d distance : %f\n",
				   g_task_mission_data.victims[j].id,
				   g_task_mission_data.victims[j].x,
				   g_task_mission_data.victims[j].y, distance);
		}
		printf("Average distance of  victims = %f\n", average_distance /
														  g_task_mission_data.victim_count);
	}

	printf("Number of minor cycle overrun  : %d\n", minor_cycle_overrun);
}
