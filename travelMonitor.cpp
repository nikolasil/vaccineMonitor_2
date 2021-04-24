#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <string>

#include "travelMonitor.h"

travelMonitor::travelMonitor(int m, int b, int s, string dir) : numMonitors(m), bufferSize(b), sizeOfBloom(s), input_dir(dir) {
    cout << "numMonitors=" << this->numMonitors << ", bufferSize= " << this->bufferSize << ", sizeOfBloom= " << this->sizeOfBloom << ", input_dir= " << this->input_dir << endl;
}

int travelMonitor::createFIFOs() {
    string directory = "pipes/";

    string name = "fifo1";
    string fullname = directory + name;
    char* fifoName = &fullname[0];

    if (mkfifo(fifoName, 0777) == -1) {
        if (errno != EEXIST) {
            cout << "Problem in creating named pipe " << fifoName << endl;
            return -1;
        }
    }
}

void travelMonitor::createMonitors() {}

void travelMonitor::sendFilesToMonitors() {}

void travelMonitor::receiveBlooms() {}

void travelMonitor::startMenu() {}