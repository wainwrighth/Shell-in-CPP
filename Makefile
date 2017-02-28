# Makefile for Project 1 of CS485, Spring 2015.
# This project is to manipulate binary data in C.

PROJECT = project4
GOAL = project4
CFLAGS = -Wall  -g
DATAFILE = testfile
INPUT = inputFile
OUTPUT = outputFile

# interactive
run-i: $(GOAL) $(DATAFILE)
	./$(GOAL) $(DATAFILE)

# batch
run-b: $(GOAL) $(DATAFILE) $(INPUT) $(OUTPUT)
	./$(GOAL) $(DATAFILE) < $(INPUT) > $(OUTPUT)

clean:
	rm -f $(GOAL) $(OUTPUT)

submit: $(GOAL).cpp Makefile README
	tar -czf $(PROJECT).tgz $(GOAL).cpp Makefile README
