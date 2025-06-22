BIN_DIR = bin
SRC_DIR = src
OBJS	= bloomFilter.o country.o genericHashTable.o record.o skipList.o vaccinationData.o Monitor.o virus.o travelMonitor.o utils.o pipe.o request.o
OBJS1	= bloomFilter.o country.o genericHashTable.o record.o skipList.o vaccinationData.o Monitor.o virus.o utils.o pipe.o
OBJS2	= bloomFilter.o country.o genericHashTable.o record.o skipList.o vaccinationData.o travelMonitor.o virus.o utils.o pipe.o request.o
SOURCE	= bloomFilter.c country.c genericHashTable.c record.c skipList.c vaccinationData.c Monitor.c virus.c utils.c pipe.c request.h
HEADER	= bloomFilter.h country.h genericHashTable.h record.h skipList.h vaccinationData.h Monitor.h virus.h utils.h pipe.h request.c
OUT	= Monitor travelMonitor
CC	= gcc
FLAGS	= -g -c

all: $(OBJS)
	$(CC) -g $(OBJS1) -o Monitor
	$(CC) -g $(OBJS2) -o travelMonitor

bloomFilter.o: bloomFilter.c bloomFilter.h
	$(CC) $(FLAGS) bloomFilter.c $(BIN_DIR)$@

country.o: country.c country.h utils.h
	$(CC) $(FLAGS) country.c

genericHashTable.o: genericHashTable.c genericHashTable.h bloomFilter.h
	$(CC) $(FLAGS) genericHashTable.c

record.o: record.c record.h country.h genericHashTable.h
	$(CC) $(FLAGS) record.c

utils.o: utils.c utils.h
	$(CC) $(FLAGS) utils.c

pipe.o: pipe.c pipe.h
	$(CC) $(FLAGS) pipe.c

request.o: request.c request.h
	$(CC) $(FLAGS) request.c

skipList.o: skipList.c skipList.h
	$(CC) $(FLAGS) skipList.c

vaccinationData.o: vaccinationData.c vaccinationData.h record.h country.h genericHashTable.h
	$(CC) $(FLAGS) vaccinationData.c

Monitor.o: Monitor.c $(HEADER)
	$(CC) $(FLAGS) Monitor.c

travelMonitor.o: travelMonitor.c genericHashTable.h virus.h bloomFilter.h pipe.h travelMonitor.h request.h utils.h
	$(CC) $(FLAGS) travelMonitor.c

virus.o: virus.c virus.h record.h skipList.h bloomFilter.h vaccinationData.h Monitor.h
	$(CC) $(FLAGS) virus.c

clean:
	rm -f $(OBJS) $(OUT) mon* tm_to_mon* log_file.*