CC = g++ -std=c++11
CFLAFS = -g

FILES = main.o travelMonitor.o util.o
DELETE_PIPES = pipes/*

travelMonitor: clean $(FILES)
	$(CC) $(CFLAFS) -o travelMonitor $(FILES)

main.o:
	$(CC) $(CFLAFS) -c main.cpp

travelMonitor.o:
	$(CC) $(CFLAFS) -c travelMonitor.cpp

util.o:
	$(CC) $(CFLAFS) -c util.cpp




monitor.o:
	$(CC) $(CFLAFS) -c monitor.cpp


.PHONY: clean
clean:
	rm -f travelMonitor $(FILES) $(DELETE_PIPES)