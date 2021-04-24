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
    string directory = this->input_dir + "/";
    for (int i = 0;i < numMonitors;i++) {
        string num = to_string(i);
        string name = "fifo" + num;
        string fullname = directory + name;
        char* fifoName = &fullname[0];

        if (mkfifo(fifoName, 0777) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                return -1;
            }
        }
    }
}

void travelMonitor::createMonitors() {
    pid_t c_pid = fork();
    if (c_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (c_pid > 0) {
        cout << "printed from parent process " << getpid() << endl;
        wait(nullptr);
    }
    else {
        cout << "printed from child process " << getpid() << endl;
        execlp("./a.out", NULL);
        exit(EXIT_SUCCESS);
    }
}

void travelMonitor::sendFilesToMonitors() {}

void travelMonitor::receiveBlooms() {}

void travelMonitor::startMenu() {}