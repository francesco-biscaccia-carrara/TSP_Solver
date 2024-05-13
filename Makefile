CC = gcc
CFLAGS = -I./include
CPLEXDIR = /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CPLEXINC = -I$(CPLEXDIR)/include 
CPLEXLIB = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex
AR = ar src
CCDIR = concorde
CCLIB = -L./$(CCDIR)/mincut.a
SRCDIR = src
INCDIR = include

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=%.o) 
OBJS += main.o
OBJS += $(CCDIR)/mincut.o
DEPS = $(wildcard $(INCDIR)/*.h)
TARGET = main
TEST = test

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(CPLEXLIB) $(CCLIB) -lm -lpthread -g -O3 && rm -f $(OBJS)

main.o: main.c
	$(CC) $(CPLEXINC) -c main.c -o main.o

test.o: test.c
	$(CC) $(CPLEXINC) -c test.c -o test.o

%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) $(CPLEXLIB) -c $< -o $@

$(CCDIR)/mincut.o: $(CCDIR)/mincut.c $(INCDIR)/mincut.h
	$(CC) $(CFLAGS) -c $(CCDIR)/mincut.c -o $(CCDIR)/mincut.o
	$(AR) $(CCDIR)/mincut.a $(CCDIR)/mincut.o

tsp_exact.o: $(SRCDIR)/tsp_exact.c $(INCDIR)/tsp_exact.h
	$(CC) $(CFLAGS) $(CPLEXINC) $(CPLEXLIB) $(CCLIB) -c $< -o $@

tsp_eutils.o: $(SRCDIR)/tsp_eutils.c $(INCDIR)/tsp_eutils.h
	$(CC) $(CFLAGS) $(CPLEXINC) $(CPLEXLIB) $(CCLIB) -c $< -o $@

clean:
	rm -f $(OBJS)
