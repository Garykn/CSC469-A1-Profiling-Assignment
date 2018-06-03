#!/usr/bin/env python

from period_plot import PeriodPlot

import os
import re
import subprocess
import sys

'''
Runs the given shell command and returns its standard output as a string.
Throws an exception if the command returns anything other than 0.'''
def cmd(command):
  ''' Popen spawns a new process (fork from c) and sets its standard streams.
  Here both stdout and stderr of p have pipes opened to them which allows
  to read from them. '''
  p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  ret = os.waitpid(p.pid, 0)[1]
  if ret != 0:
    raise Exception('Command [%s] failed with status %d\n:%s' % (command,
      ret, p.stderr.read()))
  # read() reads data from standard output of the spawned process p and
  # returns it as a string
  return p.stdout.read() 


if len(sys.argv) != 2:
  raise Exception('Invalid number of arguments. Expected 1')

# Parse/get the number of periods from the command line arguments.
num = int(sys.argv[1]);

# Build the program //or all programs listed in make. 
cmd('make')

# Run the program and create the plot.
output = cmd(['./tsc', str(num)])
plot = PeriodPlot()

startpoint=0
# splitlines() returns an array of strings where each line is a string
# in the array
for line in output.splitlines():
  print(line)
  # Parse the program's output.
  match = re.match('(Active|Inactive)(.*)(: start at )(.*)(, duration )(.*)( cycles, \()(.*)(ms\))', line)
  #match = re.match('(.*:)(start at)(.*)(,)(.*)(,)(\(.*ms\))', line) 
  ''' Returns all matching subgroups in a tuple. The subgroups are the matches
  from line to (start=),(.*),(,),(end=), and (.*) respectively.
  we have 5 groups 0 to 4 here.'''
  if not match:
    continue
  print(match)
  #start = float(match.groups()[3])
  #duration = float(match.groups()[5])
  time = float(match.groups()[7]) 
  if match.groups()[0] == "Inactive":
    color = "red"
  else:
    color = "blue" #active period - blue rectangluar 
  plot.AddPeriod(startpoint, startpoint + time, color);
  startpoint+=time
plot.CreatePlot('activity periods', 'tsc.eps')
