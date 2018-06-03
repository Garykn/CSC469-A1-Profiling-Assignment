files:
  driver.py - runs the experimental program (tsc.c), parses its output, and creates an esp image 
  representing the activity periods of the program.
  Run: 
  ./driver.py num 
  where num is the number of periods you want to generate.
  
  driver2.py - runs the experimental program (context_switch.c), parses its output, and creates an esp image 
  representing the activity periods of the program.
   Run: 
  ./driver2.py num 
  where num is the number of periods you want to generate.
  
  Makefile - compiles tsc.c and context_switch.c
 
  period_plot.py   - python class for creating period plots with Gnuplot.
  
  
Latest revision on svn master repo is:   


  
requirements:
  python 2.6
  gnuplot 4.2 patchlevel 2
  gcc
