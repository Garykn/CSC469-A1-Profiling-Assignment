import sys
import os
import StringIO

BAR_BOTTOM = 0.5
BAR_HEIGHT = 3

class PeriodPlot(object):
  '''A simple interface for creating plots of time periods. This class requires
  gnuplot 4.2 patchlevel 2 to be installed.'''

  def __init__(self):
    self.__periods = []
    self.__max_end = 0

  def AddPeriod(self, start, end, color):
    self.__periods.append((start, end, color))
    self.__max_end = max(end, self.__max_end)

  def OutputGNUPlotScript(self, title, output_file, stream):
    '''Outputs the Gnuplot script that will create a plot of the given periods.
    The title an eps file name are specified. Output is written to the given
    stream.'''
    stream.write('''set title "%s"
set xlabel "Time (ms)"
set nokey
set noytics
set term postscript eps 10
set size 0.45, 0.35
set output "%s"
''' % (title, output_file))

    object_count = 1
    for period in self.__periods:
      stream.write(
          'set object %d rect from %f, %f to %f, %f, 2 fc rgb "%s" fs solid\n' %
              (object_count, period[0], BAR_BOTTOM,
               period[1], BAR_BOTTOM + BAR_HEIGHT, period[2]))
      object_count += 1
    #this is set object someNumber rect from x_0, y_0, x_1, y_1, color
    #here if we set BAR_BOTTOM TO 0 and BAR_HEIGHT to 3.5, we can get full
    #rectangles in the plot like the ones in the assignment sheet. 

    stream.write('plot [0:%d] [0:%d] 0\n' %
        (self.__max_end, BAR_BOTTOM + BAR_HEIGHT))

  def CreatePlot(self, title, output_file):
    '''Runs Gnuplot to create a period plot. The output is an eps file with the
    given name.'''
    
    
    '''StringIO implements in-memory file object. This object can be used as
    input or output to most functions that expect a standard file object.'''
    command = StringIO.StringIO()
    # Write to command which is a memory file/stream
    command.write('gnuplot << ---EOF---\n') 
    # Command is a memory file/stream. # write output_file to command
    self.OutputGNUPlotScript(title, output_file, command)
    command.write('---EOF---\n')
    '''getvalue method that returns the internal string value of the file
    command os.system(program ) executes program in a subshell. Here it
    executes the gnu script that's returned by comand.getvalue().'''
    ret = os.system(command.getvalue()) 
    if ret != 0:
      raise Exception(
          'Error running the plotting script:\n%s' % (command.getvalue()))
