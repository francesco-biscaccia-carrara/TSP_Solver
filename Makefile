CC = gcc
CFLAGS = -I./include
CPLEXDIR = /opt/ibm/ILOG/CPLEX_Studio2211/cplex
CPLEXINC = -I$(CPLEXDIR)/include 
CPLEXLIB = -L$(CPLEXDIR)/lib/x86-64_linux/static_pic -lcplex
CNOCOLIB = -L./concorde/mincut.a
SRCDIR = src
INCDIR = include

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=%.o) 
OBJS += main.o
DEPS = $(wildcard $(INCDIR)/*.h)
TARGET = main
TEST = test

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(CPLEXINC) $(CPLEXLIB) -lm -lpthread -g -O3 && rm -f $(OBJS)

main.o: main.c
	$(CC) $(CPLEXINC) $(CPLEXINC) $(CPLEXLIB) -c main.c -o main.o

test.o: test.c
	$(CC) $(CPLEXINC) $(CPLEXINC) $(CPLEXLIB) -c test.c -o test.o

%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) $(CPLEXINC) $(CPLEXLIB) -c $< -o $@

clean:
	rm -f $(OBJS)
