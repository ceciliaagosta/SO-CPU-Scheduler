CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_ 
AR=ar

OBJS=linked_list.o\
     fake_process.o\
     fake_os.o\
     probability_distribution.o

HEADERS=linked_list.h  fake_process.h  fake_os.h  probability_distribution.h

BINS=fake_process_test sched_sim probability_test probability_sample

#disastros_test

.phony: clean all


all:	$(BINS)

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

fake_process_test:	fake_process_test.c $(OBJS)
	$(CC) $(CCOPTS) -o $@ $^

sched_sim:	sched_sim.c $(OBJS)
	$(CC) $(CCOPTS) -o $@ $^
	
probability_test:	probability_test.c $(OBJS)
	$(CC) $(CCOPTS) -o $@ $^
	
probability_sample:	probability_sample.c $(OBJS)
	$(CC) $(CCOPTS) -o $@ $^

clean:
	rm -rf *.o *~ $(OBJS) $(BINS)
