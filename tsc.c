#define _GNU_SOURCE
#include <sys/resource.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <inttypes.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>

#define SLEEP_PERIOD 1E6L
#define SEPERATOR_PERIOD 1E6L  /* 1 millisecond = 10^6 nanoseconds */  
#define TIMES_TO_SLEEP 10

void access_counter(unsigned *hi, unsigned *lo);
void start_counter();
u_int64_t get_counter();
  

/* the variable will hold the num_periodsber of cpu cycles in 10^6 ns or 1ms*/
u_int64_t cycles_per_ms = 0;

/* cycle counter */
u_int64_t start = 0;

/* Set *hi and *lo to the high and low order bits of the cycle counter.
 * Implementation requires assembly code to use the rdtsc instruction.
 */
inline void access_counter(unsigned *hi, unsigned *lo)
{
  asm volatile("rdtsc; movl %%edx, %0; movl %%eax, %1" /* Read cycle counter */
      : "=r" (*hi), "=r" (*lo)                /* and move results to */
      : /* No input */                        /* the two outputs */
      : "%edx", "%eax");
}

/*
Gets an initial value from the TSC counter and stores it in start
*/
inline void start_counter(){
  unsigned hi, lo;
  access_counter(&hi, &lo);
  start = ((u_int64_t)hi << 32) | lo;
}

/*returns a reading from the TSC counter */
inline u_int64_t get_counter(){
  unsigned ncyc_hi, ncyc_lo;
  access_counter(&ncyc_hi, &ncyc_lo);
  return (((u_int64_t)ncyc_hi << 32) | ncyc_lo) - start;
}

/*
returns the operating frequency of the CPU. Note: This measure is not 100% accurate and the returned value is a 
reasonable approximatation of the actual frequency done through taking the smallest measured sample. This is based upon
the assumption that our measurement method overestimates actual values becuase of overhead.  

imbench.pdf mentions that the nanosleep() system call provides reasonably accurate (requested ~ actual duration) sleep durations for requests less than 2000 µs. Going by this information in imbench.pdf, we decided to use a sleep interval duration of 1000 us = 10^6 ns = 1ms.
*/
u_int64_t computeMHz(){
	struct timespec time_to_Sleep;
	time_to_Sleep.tv_sec = 0;
	time_to_Sleep.tv_nsec = SLEEP_PERIOD; //sleep for 10^6 nanoseconds
	u_int64_t interval_samples_incycles[TIMES_TO_SLEEP]; //get ten samples and compute an average measurement
	
	int i = 0;
	while(i < TIMES_TO_SLEEP) {
		/* add some separation between consecutive sleep system calls*/
		for(int j = 0; j < SEPERATOR_PERIOD; j++);
		
		start_counter();
		if(nanosleep(&time_to_Sleep, NULL) == 0) { 
			interval_samples_incycles[i] = get_counter();
			i++;
		}
	}
	
   //compue the average of all measurements
   uint64_t sum = 0;
   for(int i = 0; i < TIMES_TO_SLEEP; i++){
	   sum += interval_samples_incycles[i];
   }
   cycles_per_ms = sum/TIMES_TO_SLEEP; 
   double MHZ = cycles_per_ms * 1E3 / 1E6; //num_periodsber of cycles per second / 1E6 to get num_periodsber of 1E6 cycles per second i.e. in MHz
   return MHZ;	
}

/*
 * this functions detects and records num_periods inactive periods and stores the start and end time of each such period in the samples array.
 */
u_int64_t inactive_periods(int num_periods, u_int64_t threshold, u_int64_t *samples) {
    u_int64_t starting_point, prev_reading, curr_reading;
	int i = 0;
    start_counter();
	prev_reading = curr_reading = starting_point = get_counter();
    while(i < num_periods) {
        prev_reading = curr_reading;
        curr_reading = get_counter();
        if(curr_reading - prev_reading >= threshold) {
            samples[2*i] = prev_reading;
            samples[2*i + 1] = curr_reading;
            i++;
        }
    }
    return starting_point;
}

int main(int argc, char const *argv[]){
	/* confirm valid command line sarguments*/
	if (argc < 2) {
	   fprintf(stderr, "Usage: %s num_periods\n", argv[0]);
	   exit(EXIT_FAILURE);
    }
	
	/* read second command line argument as a base 10 long long integer*/
    uint64_t num_periods = strtoull(argv[1], NULL, 10);
	assert(num_periods > 5);
	
	// get and print out the frequency of the CPU in MHz
	double cpuFrequency = computeMHz();
	printf("Clock Speed: %.2f MHz\n", cpuFrequency);
	
	/*determine threshold:
	threshold is the num_periodsber of cycles between two consectuve polls of TSC (time stamp counter register) at/obove which one can 
	assume that the process has been preempted.
	We estimate threshold to be in the magnitude of 1000 ns = 1us, going by what we've learned about the various access latencies of a modern x86 CPUs.
	For example, the cost of main memory reference is ~100ns and this accounts for L1, L2, and L3 cashe misses*/
	u_int64_t threshold = cycles_per_ms / 1E3; //number of cycles in 1000ns
	
	//bind our process to a low-acitivty/idle CPU. We'll bind to CPU #3
	cpu_set_t set; // this variable represents a set of CPUs. 
    CPU_ZERO(&set); //clear the set of CPUs. 
    CPU_SET(3, &set); //add CPU #3 to the set
	 
	if (sched_setaffinity( getpid(), sizeof(cpu_set_t), &set ))
    {
        perror( "sched_setaffinity" );
                exit(EXIT_FAILURE);
    }
	
	// get measurements of activity periods
	u_int64_t *samples = (u_int64_t *)malloc(sizeof(u_int64_t) * num_periods * 2);	//memory for the samples array
	assert(samples != NULL);
	
	
	//print activity periods
    u_int64_t start_point = inactive_periods(num_periods, threshold, samples);
	u_int64_t active_start_point = start_point;
	int i;
	long double averageActive=0; //average inactive period 
	for( i = 0; i < num_periods; i++) {
		/* print each active period followd by an inactive periods. These periods alternate in succession.
		the end of an active period is the start of the following inactive period and the of end of an inactive period is the start of the following active period, and so forth. */ 
        printf("Active %d: start at %llu, duration %llu cycles, (%.6Lf ms)\n",i, 
				active_start_point, (samples[2*i] - active_start_point), 
				(samples[2*i] - active_start_point)/(long double)cycles_per_ms);
		
		if(i > 0 && i < num_periods - 1){
				averageActive += (samples[2*i] - active_start_point)/(long double)cycles_per_ms;
		}
		
        printf("Inactive %d: start at %llu, duration %llu cycles, (%.6Lf ms)\n", i, 
				samples[2*i], (samples[2*i+1] - samples[2*i]), 
				(samples[2*i+1] - samples[2*i])/(long double)cycles_per_ms);
		//update active_start_point to point to the end point of the current inactive period.
        active_start_point = samples[2*i+1];
    }
	
	//compute average active period
		averageActive = averageActive/(num_periods - 2);
		
	//print average active time 
	printf("average activity period of this process is%.6Lfms\n");

	//free malloc'ed memory
	free(samples);
}

