OBJS	= bloomFilter.o skipList.o travelMonitor.o
SOURCE	= bloomFilter.c skipList.c travelMonitor.c
HEADER	= bloomFilter.h skipList.h travelMonitor.h
OUT	= travelMonitor
CC	= gcc
FLAGS	= -g -c

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

bloomFilter.o: bloomFilter.c bloomFilter.h
	$(CC) $(FLAGS) bloomFilter.c

skipList.o: skipList.c
	$(CC) $(FLAGS) skipList.c

travelMonitor.o: travelMonitor.c
	$(CC) $(FLAGS) travelMonitor.c

clean:
	rm -f $(OBJS) $(OUT) Monitor mon* tm_to_mon* 