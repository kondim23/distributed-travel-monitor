OBJS	= bloomFilter.o country.o genericHashTable.o record.o skipList.o vaccinationData.o Monitor.o virus.o
SOURCE	= bloomFilter.c country.c genericHashTable.c record.c skipList.c vaccinationData.c Monitor.c virus.c
HEADER	= bloomFilter.h country.h genericHashTable.h record.h skipList.h vaccinationData.h Monitor.h virus.h
OUT	= Monitor travelMonitor
CC	= gcc
FLAGS	= -g -c

all: $(OBJS)
	$(CC) -g $(OBJS) -o Monitor
	$(CC) -g travelMonitor.c -o travelMonitor

bloomFilter.o: bloomFilter.c bloomFilter.h
	$(CC) $(FLAGS) bloomFilter.c

country.o: country.c country.h
	$(CC) $(FLAGS) country.c

genericHashTable.o: genericHashTable.c genericHashTable.h bloomFilter.h
	$(CC) $(FLAGS) genericHashTable.c

record.o: record.c record.h country.h genericHashTable.h
	$(CC) $(FLAGS) record.c

skipList.o: skipList.c skipList.h
	$(CC) $(FLAGS) skipList.c

vaccinationData.o: vaccinationData.c vaccinationData.h record.h country.h genericHashTable.h
	$(CC) $(FLAGS) vaccinationData.c

Monitor.o: Monitor.c $(HEADER)
	$(CC) $(FLAGS) Monitor.c

virus.o: virus.c virus.h record.h skipList.h bloomFilter.h vaccinationData.h Monitor.h
	$(CC) $(FLAGS) virus.c

clean:
	rm -f $(OBJS) $(OUT) mon* tm_to_mon*