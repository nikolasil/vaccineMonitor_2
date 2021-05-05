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
#include "util.h" 
#include "DataStructures/bloomFilter/bloomFilter.h"

travelMonitor::travelMonitor(int m, int b, int s, string dir) : numMonitors(m), bufferSize(b), sizeOfBloom(s), input_dir(dir) {
    cout << "numMonitors=" << this->numMonitors << ", bufferSize= " << this->bufferSize << ", sizeOfBloom= " << this->sizeOfBloom << ", input_dir= " << this->input_dir << endl;
    this->countryToMonitor = NULL;
    this->fifoFD = NULL;
    this->blooms = new bloomFilterList(this->sizeOfBloom);
    checkNew(this->blooms);
}

void travelMonitor::createFIFOs() {
    string directory = "pipes/";
    for (int i = 0;i < numMonitors;i++) {
        string fullname = "pipes/fifo_tW_mR_" + to_string(i);
        char* fifoName = &fullname[0];

        if (mkfifo(fifoName, 0666) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                exit(EXIT_FAILURE);
            }
        }

        fullname = "pipes/fifo_tR_mW_" + to_string(i);
        fifoName = &fullname[0];

        if (mkfifo(fifoName, 0666) == -1) {
            if (errno != EEXIST) {
                cout << "Problem in creating named pipe " << fifoName << endl;
                exit(EXIT_FAILURE);
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
            string pipe0 = "pipes/fifo_tW_mR_" + to_string(i);
            string pipe1 = "pipes/fifo_tR_mW_" + to_string(i);
            execlp("./Monitor", pipe0.c_str(), pipe1.c_str(), NULL);
        }
    }
}

void travelMonitor::openFifos() {
    for (int i = 0;i < numMonitors;i++) {
        string pipe0 = "pipes/fifo_tW_mR_" + to_string(i);
        string pipe1 = "pipes/fifo_tR_mW_" + to_string(i);
        int fd0 = open(pipe0.c_str(), O_WRONLY);
        int fd1 = open(pipe1.c_str(), O_RDONLY);
        // cout << "i=" << i << ",writefd=" << fd0 << ",readfd=" << fd1 << endl;
        this->addFD(fd1, fd0);
    }
}

void travelMonitor::sendCredentials() {
    for (int i = 0;i < numMonitors;i++) {
        int fd = this->fifoFD->getWriteFifo(i);
        // cout << "i=" << i << ",writefd=" << fd << endl;
        if (write(fd, &i, sizeof(int)) == -1)
            cout << "Error in writting id with errno=" << errno << endl;
        if (write(fd, &this->bufferSize, sizeof(int)) == -1)
            cout << "Error in writting bufferSize with errno=" << errno << endl;
        if (write(fd, &this->sizeOfBloom, sizeof(int)) == -1)
            cout << "Error in writting sizeOfBloom with errno=" << errno << endl;

        sendStr(i, this->input_dir);
    }
}

void travelMonitor::sendCountries() {
    int fd;

    int monitor = 0;
    DIR* input;
    struct dirent* dir;
    string in = this->input_dir;
    char* in2 = &in[0];

    input = opendir(in2);
    if (input)
    {
        while ((dir = readdir(input)) != NULL)
        {
            string country = dir->d_name;
            // cout << country << " " << monitor << endl;
            if (country.compare("..") == 0 || country.compare(".") == 0)
                continue;

            this->addCountryToMonitor(country, monitor);

            sendStr(monitor, country);

            monitor++;
            if (monitor >= this->numMonitors)
                monitor = 0;

        }
        closedir(input);
    }
    for (int i = 0;i < this->numMonitors;i++) {
        fd = this->fifoFD->getWriteFifo(i);
        int end = -1;
        if (write(fd, &end, sizeof(int)) == -1)
            cout << "Error in writting end with errno=" << errno << endl;
    }
}

void travelMonitor::receiveBlooms() {
    for (int i = 0;i < this->numMonitors;i++) {
        int end = 0;
        while (end != -1) {
            string virus = receiveManyStr(i, &end);
            addNewVirus(virus);
            cout << "Got virus=" << virus << endl;
        }
    }
    cout << "got all the viruses" << endl;
    for (int i = 0;i < this->numMonitors;i++) {
        while (1) {
            string virus = receiveStr(i);
            cout << virus << endl;
            if (virus.compare("END SEND BLOOMS") == 0)
                break;
            int bit;
            int fd = this->fifoFD->getReadFifo(i);
            while (1) {
                if (read(fd, &bit, sizeof(int)) == -1)
                    cout << "Error in reading bit with errno=" << errno << endl;
                if (bit == -1)
                    break;
                this->blooms->getBloom(this->viruses->search(virus))->setBit(bit, 1);
            }
            this->blooms->getBloom(this->viruses->search(virus))->print();

        }
    }

}

void travelMonitor::startMenu() {}

void travelMonitor::sendStr(int monitor, string str) {
    int fd = this->fifoFD->getWriteFifo(monitor);
    char* to_tranfer = &str[0];
    int sizeOfStr = strlen(to_tranfer);

    if (write(fd, &sizeOfStr, sizeof(int)) == -1)
        cout << "Error in writting sizeOfStr with errno=" << errno << endl;

    int pos = 0;
    for (int i = 0;i <= strlen(to_tranfer) / this->bufferSize;i++) {
        if (write(fd, &to_tranfer[pos], this->bufferSize) == -1)
            cout << "Error in writting to_tranfer with errno=" << errno << endl;

        pos += this->bufferSize;
    }
}

string travelMonitor::receiveStr(int monitor) {
    int fd = this->fifoFD->getReadFifo(monitor);
    int sizeOfStr;
    if (read(fd, &sizeOfStr, sizeof(int)) == -1)
        cout << "Error in reading sizeOfStr with errno=" << errno << endl;

    string str = "";
    for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
        char buff[this->bufferSize];
        if (read(fd, &buff, this->bufferSize) == -1)
            cout << "Error in reading buff with errno=" << errno << endl;
        buff[this->bufferSize] = '\0';
        str.append(buff);
    }
    return str;
}

string travelMonitor::receiveManyStr(int monitor, int* end) {
    int fd = this->fifoFD->getReadFifo(monitor);
    int sizeOfStr;
    if (read(fd, &sizeOfStr, sizeof(int)) == -1)
        cout << "Error in reading sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr == -1) {
        *end = -1;
        return "";
    }

    string str = "";
    for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
        char buff[this->bufferSize];
        if (read(fd, &buff, this->bufferSize) == -1)
            cout << "Error in reading buff with errno=" << errno << endl;
        buff[this->bufferSize] = '\0';
        str.append(buff);
    }
    return str;
}

void travelMonitor::addCountryToMonitor(string c, int m) {
    if (this->countryToMonitor == NULL)
        this->countryToMonitor = new monitorList(c, m);
    else
        this->countryToMonitor = this->countryToMonitor->add(c, m);
}

void travelMonitor::addNewVirus(string virusName)
{
    if (this->viruses->search(virusName) == NULL) // if we dont have that virus add it to the list of viruses
    {                                         // and make the bloom filter and the skiplist for that virus
        this->viruses = this->viruses->add(virusName);
        this->blooms = this->blooms->add(this->viruses);
    }
}

void travelMonitor::addFD(int r, int w) {
    if (this->fifoFD == NULL)
        this->fifoFD = new fifoFDList(r, w);
    else
        this->fifoFD->add(r, w);
}

void travelMonitor::printAllViruses() {
    cout << "ALL THE viruses" << endl;
    this->viruses->print();
}

void travelMonitor::printCountryToMonitor() {
    this->countryToMonitor->print();
}