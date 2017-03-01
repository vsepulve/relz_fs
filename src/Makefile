SRCDIR = ./
BINDIR = ../bin
LIBDIR = ../lib
DATADIR = ../data
TYPE ?= -O3
CPPFLAGS = $(TYPE) -Wall -I../includes/ -L../lib/ -std=c++0x
CFLAGS = $(TYPE) -Wall -I../includes/ -L../lib/

CPP = g++

%.o: %.cc 
	$(CPP) $(CPPFLAGS) -c $< -o $(LIBDIR)/$@


%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $(LIBDIR)/$@


%.o: %.c
	$(CPP) $(CFLAGS) -c $< -o $(LIBDIR)/$@

	
