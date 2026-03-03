#! /usr/bin/make

#----------------------------------------------------------------------------
#
# --- Configuration variables ---
#
#  These variables are used in building the programs, as well as configuring
#    how tests are run.  These values have default over-rideable values
#    (which is what the ?= operator does; it uses the value in this declaration
#    if a different value isn't provided)  
#

# Compiler optimization level; debug only by default
#
#  To turn on both optimization and debugging information, pass both options
#    to make as a string, e.g.,
#
#    make OPT="-O2 -g2" ...
#
OPT ?= -g

# Program configuration values (used by the C++ compilers)
MIN_BYTES ?= 3
MAX_BYTES ?= 100

# Program execution and test values (used when running the programs)
NUM_BLOCKS ?= 10000
NUM_TRIALS ?= 10

#----------------------------------------------------------------------------
#
# --- Tool and option variables
#
#  Variables used to select files and specify filenames, specify commands, 
#    an provide options to commands.  These variables are used in the build
#    rules in the next section
#

# Select our source code files
SOURCES = $(wildcard *.cpp)

# Map source code files to executable names.  In this case, each executable
#   uses only a single source file, so we merely replace the .cpp source file
#   extension with an executable name, .out, leaving the file's "basename"
#   unmodified
#
TARGETS = $(SOURCES:.cpp=.out)

# Construct C++ compiler flags (CXXFLAGS).  
CXXDEFS = -DMIN_BYTES=$(MIN_BYTES) -DMAX_BYTES=$(MAX_BYTES)
CXXFLAGS = $(OPT) $(CXXDEFS) 

# Select files that should be removed when we need to "clean" a project
DIRT = $(wildcard *.o *.out *.dSYM)

# Specify paths to various commands used for compilation or executing programs.
#   Many of these commands' paths are standard across operating systems, and
#   may not necessarily need this, but using variables should also make reading
#   and maintaining the commands simpler.
PRINTF := /usr/bin/printf
RM := rm -fr
STRACE := /usr/bin/strace
GREP := /usr/bin/grep
WC := /usr/bin/wc -l
AWK_CMDS := awk.cmds
AWK := /usr/bin/awk -f $(AWK_CMDS)
LOG = trials.log
TIME := /usr/bin/time -f "%e real\t%U user\t%S sys\t%M memory (KB)"

# Configure operating-system specific tools
ifeq ($(OSTYPE), darwin) # macOS
	TIME := /usr/bin/time
endif

# Sorry Windows users :-P

#----------------------------------------------------------------------------
#
# --- Make "Build" rules ---
#
#  Define what operations should happen for this make file.  Any words on
#    the left of a colon (:), are options that can be passed into make (even
#    on the same command line, e.g., "make clean test")

# Specify the default action (i.e., what to do when no options are passed to
#  make).  A "default" target isn't strictly required as the first rule
#  encountered will be executed by default, but adding that rule helps make
#  the order of operations clearer.
default: targets

# Provide a "convenience target" to simplify rule construction.  "targets"
#   is used in several targets (including "default" above; rules can appear
#   in any order).
targets: $(TARGETS)

# A "transform rule" specifying how .cpp files are converted to .out
#   (executable) files, which is essentially how they're compiled and linked. 
%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# Reset the build by removing compiled executables, debugging information, etc.
clean:
	$(RM) $(DIRT)

#----------------------------------------------------------------------------
#
# --- Make "Run" rules ---
#
# Rules to simplify testing and conducting benchmark trials.
#

# Execute each target to verify they run and generate the correct results.
#   Each program is passed NUM_BLOCKS to specify how many iterations it
#   should execute
test: targets
	@ for pgm in $(TARGETS) ; do \
		result=$$($$pgm $(NUM_BLOCKS)) ;\
		$(PRINTF) "%-18s %s\n" "$$pgm:" "$$result" ;\
	done

# Execute each target to determine how many pages the kernel needed to add
#   to the heap to accommodate the program's dynamic memory requests. 
#   Each program is passed NUM_BLOCKS to specify how many iterations it
#   should execute
breaks: targets
	@ for pgm in $(TARGETS) ; do \
		result=$$($(STRACE) $$pgm $(NUM_BLOCKS) 2>&1 $(GREP) brk | $(WC)) ;\
		$(PRINTF) "%-18s %s\n" "$$pgm:" "$$result" ;\
	done

# Execute a benchmarking run.  Each program is run NUM_TRIALS times for
#   NUM_BLOCKS iterations while being timed.  The execution times for each
#   run is output, as well as collected into a log file, which is processed
#   after all the program's runs are completed to generate some simple
#   statistics
trials: targets
	@ $(RM) $(LOG) ;\
	for pgm in $(TARGETS) ; do \
		$(PRINTF) "%s (list length = %d):\n" $$pgm $(NUM_BLOCKS) ;\
		for t in {1..$(NUM_TRIALS)} ; do \
			( $(TIME) $$pgm $(NUM_BLOCKS) > /dev/null ) 2>&1 | tee -a $(LOG) ;\
		done ;\
		cat $(LOG) | $(AWK) ;\
		$(PRINTF) "\n" ;\
		$(RM) $(LOG) ;\
	done

# Optimize make's execution by specifying targets that aren't filenames.
#   By default, make assumes that any name in a rule is a filename, and
#   will search for it.  By specifying .PHONY options, make won't look
#   for a file, speeding up the build
.PHONY: default targets clean test trials
