#flags
CCFLAGS = gcc -g

all : tsc context_switch context_switch_pthread
#make executables

tsc : tsc.c
	$(CCFLAGS) $^ -o $@ -D_GNU_SOURCE
		  
context_switch : context_switch.c
	$(CCFLAGS) $^ -o $@ -D_GNU_SOURCE

context_switch_pthread : context_switch_pthread.c
	$(CCFLAGS) $^ -o $@ -lpthread -D_GNU_SOURCE	
	
clean : 
	rm -f *.o tsc context_switch context_switch_pthread
