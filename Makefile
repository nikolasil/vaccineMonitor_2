CC = g++ -std=c++11
CFLAFS = -g

FILES = main.o travelMonitor.o util.o

travelMonitor: clean $(FILES)
	$(CC) $(CFLAFS) -o travelMonitor $(FILES)

main.o:
	$(CC) $(CFLAFS) -c main.cpp

travelMonitor.o:
	$(CC) $(CFLAFS) -c travelMonitor.cpp

util.o:
	$(CC) $(CFLAFS) -c util.cpp


.PHONY: clean
clean:
	rm -f travelMonitor $(FILES)