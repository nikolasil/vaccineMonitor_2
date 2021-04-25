#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

#include "travelMonitor.h"

travelMonitor::travelMonitor(int m, int b, int s, string dir) : numMonitors(m), bufferSize(b), sizeOfBloom(s), input_dir(dir) {
    cout << "numMonitors=" << this->numMonitors << ", bufferSize= " << this->bufferSize << ", sizeOfBloom= " << this->sizeOfBloom << ", input_dir= " << this->input_dir << endl;
}

int travelMonitor::createFIFOs() {
    string directory = "pipes/";
    for (int i = 0;i < numMonitors;i++) {
        string num = to_string(i);
        string name = "fifo_tW_mR_" + num;
        string fullname = directory + name;
        char* fifoName = &fullname[0];

        if (mkfifo(fifoName, 0666) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                return -1;
            }
        }
        name = "fifo_tR_mW_" + num;
        fullname = directory + name;
        fifoName = &fullname[0];

        if (mkfifo(fifoName, 0666) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                return -1;
            }
        }
    }
}

void travelMonitor::createMonitors() {
    for (int i = 0;i < numMonitors;i++) {
        pid_t c_pid = fork();
        if (c_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (c_pid > 0) {
            cout << "Monitor " << i << " Created" << endl;
        }
        else {
            string num = to_string(i);
            string pipe0 = "pipes/fifo_tW_mR_" + num;
            string pipe1 = "pipes/fifo_tR_mW_" + num;

            execlp("./monitor", "monitor.out", pipe0.c_str(), pipe1.c_str(), NULL);
            exit(EXIT_SUCCESS);
        }
    }
}

void travelMonitor::sendFilesToMonitors() {
    for (int i = 0;i < numMonitors;i++) {
        string num = to_string(i);
        string pipe0 = "pipes/fifo_tW_mR_" + num;
        cout << "travelMonitor opening " << pipe0 << endl;
        int fd = open(pipe0.c_str(), O_WRONLY);
        cout << "travelMonitor opened " << pipe0 << endl;
        char a = 'n';
        if (write(fd, &a, 1) == -1) {
            cout << "Error in writting" << endl;
        }
        close(fd);
    }
}

void travelMonitor::receiveBlooms() {}

void travelMonitor::startMenu() {}