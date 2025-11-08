# Command Line 2D Projective Point Matcher 

The Paper: 
[Two Dimensional Projective Point Matching](https://github.com/jasondenton/Publications/blob/main/2dpntmatch.pdf)

## Building the program

This is research code. Although it should generally work, neither the
code nor the binaries it produces are meant to do anything but support
my own research efforts. This code is heavily optimized for speed, and
intended to be used in batch systems running a large number of trials.

## Setup 

The program can be built using 'make'. See the comments in the Makefle
for configuration details. In particular, the provide Makefile is set
to build on a MacOS system with libjpeg installed from homebrew.

Run make from the expr directory to have it create soft links
to the binaries and all provided data files. This provides a convinent
way to separate data and binaries and setup an area for research
results.

## Running the program

The main program is "pntmatcher". Run the program without arguments
for instructions. Invoking this program runs the key feature local
search point matching program. By default the makefiles produce three
symbolic links to this program; ransac, iransac, and
pntmatch_rs. Invoking the program with one of these names causes it to
use RANSAC, iterative RANSAC, or random starts local search.

The program takes one required parameter, and a second optional
parameter. The first parameter is a problem description file,
describing the problem to be solved. This file specifies model and
data sets, and other algorithm parameters. The second parameter is the
number of trials to run. If not supplied, the default is to run 1000
trials, or some portion of the key feature list. What portion of the
key feature list this is a heuristic implemented in the key feature
code, and prone to change.

The model and point sets referenced in the problem description file
should be available in the current directory when the program
runs. The program reports the optimal match or matches found.  The
program assumes that the model and data sets reference an underlying
image file using a #image comment in the data file. If no such images
are refereed or available the program will seg fault. This is a
bug. It would be easy to fix. The program transforms the model image
by the optimal pose for each match, and super imposes the resulting
image over the data image. The results of this are saved into a series
of .pgm files.

This version of the point matcher will attempt to exploit multiple
processors in the same system. It determines the number of processors
as described below, then spawns additional posix threads for each
additional processor. The process_list function takes care of all the
coordination for this, and attempts to keep the load balanced with a
minimum of overhead. On a dual processor mac this works like you would
expect, and cuts the wall clock time for large runs roughly in
half. On Intel hyper-threaded systems the second logical processor is
detected and the program proceeds to trip over itself nearly doubling
the required time.

The key to making the multiprocessor support work is determining the
number of processes. The num_proc_*.c files (and the master file
num_proc.c) contains the code for determining the number of processors
on different types of systems.  This code will honor the value in the
environmental variable NUMBER_OF_PROCESSORS, so you can force
hyper-threaded CPUS to behave as though only one CPU is there. On
systems for which a number_of_processors function is not provided the
default version simple returns 1 or whatever value is defined in this
variable. Systems with multi-threading processors, such as Intel's HTT
feature, should set the number of processors to match the number of
number physical processors.

## Problem file format

The best way to write problem description files is to use one of the
provided examples. The model and data fields specify the model and
point sets to be used. These are plain text files, containing a series
of floating point numbers. Each pair of numbers specify the x and y
coordinates of a single point. Each point file should start with an
#image comment specifying the .pgm file that contains the underlying
image. The sigma field gives the distance between paired model and
data points at which the objective function deems it better to drop
the pairing than accept it. The transform field should be either
projective or similarity. The projective cases works. The similarity
case is under tested in this version. It should be optimized by the
time the next version hits. Despite initial apparences in the code
base, affine does not work. The equations in my dissertation for the
optimal affine pose appear to be wrong, I think the ones in this code
are correct. If any one wants to develop the case, I'd be happy to
hear about it. The instances field specifies how many matches should
be reported by the system. Scale is a maximum "stretch" that can be
applied to the bounding box before a linear penalty is applied. For
problems drawn from real imagery, sigma=5.0 and scale=3.5 appear to be
good values in most cases. Increasing scale appears to be helpful in
some cases.

Jason Denton
jason.denton@gmail.com
