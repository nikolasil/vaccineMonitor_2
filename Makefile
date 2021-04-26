CC = g++ -std=c++11
CFLAFS = -g

TRAVEL_MONITOR_FILES = mainTravelMonitor.o travelMonitor.o util.o monitorList.o fifoFDList.o
MONITOR_FILES = mainMonitor.o monitor.o util.o stringList.o

all: clean travelMonitor Monitor

# travelMonitor
travelMonitor: $(TRAVEL_MONITOR_FILES)
	$(CC) $(CFLAFS) -o travelMonitor $(TRAVEL_MONITOR_FILES)

mainTravelMonitor.o:
	$(CC) $(CFLAFS) -c mainTravelMonitor.cpp

travelMonitor.o:
	$(CC) $(CFLAFS) -c travelMonitor.cpp


# Monitor
Monitor: $(MONITOR_FILES)
	$(CC) $(CFLAFS) -o Monitor $(MONITOR_FILES)

mainMonitor.o:
	$(CC) $(CFLAFS) -c mainMonitor.cpp

monitor.o:
	$(CC) $(CFLAFS) -c monitor.cpp


# General Files
stringList.o:
	$(CC) $(CFLAFS) -c DataStructures/stringList/stringList.cpp

monitorList.o:
	$(CC) $(CFLAFS) -c DataStructures/monitorList/monitorList.cpp

fifoFDList.o:
	$(CC) $(CFLAFS) -c DataStructures/fifoFDList/fifoFDList.cpp

util.o:
	$(CC) $(CFLAFS) -c util.cpp


.PHONY: clean
clean:
	rm -f travelMonitor $(TRAVEL_MONITOR_FILES) $(MONITOR_FILES) pipes/*