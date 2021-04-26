#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <typeinfo>
#include <cstring>
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

        if (mkfifo(fifoName, 0777) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                return -1;
            }
        }
        name = "fifo_tR_mW_" + num;
        fullname = directory + name;
        fifoName = &fullname[0];

        if (mkfifo(fifoName, 0777) == -1) {
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
            cout << pipe0 << " " << pipe1 << endl;
            execlp("./monitor", "monitor.out", pipe0.c_str(), pipe1.c_str(), NULL);
        }
    }
}

void travelMonitor::openFifos() {
    for (int i = 0;i < numMonitors;i++) {
        string num = to_string(i);
        string pipe0 = "pipes/fifo_tW_mR_" + num;
        string pipe1 = "pipes/fifo_tR_mW_" + num;
        int fd0 = open(pipe0.c_str(), O_WRONLY);
        int fd1 = open(pipe1.c_str(), O_RDONLY);
        // list with fd
    }
}

void travelMonitor::sendFilesToMonitors() {
    // send to all monitors the buffer size
    // size of bloom
    int fd;
    for (int i = 0;i < this->numMonitors;i++) {
        string num = to_string(i);
        string pipe0 = "pipes/fifo_tW_mR_" + num;
        string pipe1 = "pipes/fifo_tR_mW_" + num;

        if (write(fd, &this->bufferSize, sizeof(int)) == -1)
            cout << "Error in writting" << endl;
        if (write(fd, &this->sizeOfBloom, sizeof(int)) == -1)
            cout << "Error in writting" << endl;

    }
    int monitor = 0;
    DIR* input;
    struct dirent* dir;
    string in = this->input_dir;
    char* in2 = &in[0];

    input = opendir(in2);
    if (input)
    {
        cout << " -- Starting to give countries -- " << endl;
        while ((dir = readdir(input)) != NULL)
        {
            string country = dir->d_name;
            if (country.compare("..") == 0 || country.compare(".") == 0)
                continue;
            string num = to_string(monitor);
            string pipe0 = "pipes/fifo_tW_mR_" + num;


            char* to_tranfer = &country[0];

            cout << "Sending to monitor " << monitor << " the country " << to_tranfer << endl;
            int size = strlen(to_tranfer);
            cout << size << endl;

            if (write(fd, &size, sizeof(int)) == -1)
                cout << "Error in writting" << endl;


            int pos = 0;
            for (int i = 0;i <= strlen(to_tranfer) / this->bufferSize;i++) {

                if (write(fd, &to_tranfer[pos], this->bufferSize) == -1)
                    cout << "Error in writting" << endl;

                pos += this->bufferSize;
            }
            monitor++;
            if (monitor >= this->numMonitors)
                monitor = 0;

        }
        cout << " -- End giving countries -- " << endl;
        closedir(input);
    }
    for (int i = 0;i < this->numMonitors;i++) {
        string num = to_string(i);
        string pipe0 = "pipes/fifo_tW_mR_" + num;
        int end = -1;
        if (write(fd, &end, sizeof(int)) == -1)
            cout << "Error in writting" << endl;

    }
}

void travelMonitor::receiveBlooms() {}

void travelMonitor::startMenu() {}