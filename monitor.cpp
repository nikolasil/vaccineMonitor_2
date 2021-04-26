#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <cstring>

#include "monitor.h"
#include "DataStructures/stringList/stringList.h"
using namespace std;

Monitor::Monitor(string r, string w) : readFifo(r), writeFifo(w) {
    readFD = open(this->readFifo.c_str(), O_RDONLY);
    writeFD = open(this->writeFifo.c_str(), O_WRONLY);
}

Monitor::~Monitor() {
    close(readFD);
    close(writeFD);
}

void Monitor::receiveCredentials() {
    if (read(readFD, &this->id, sizeof(int)) == -1)
        cout << "Monitor error in reading id with errno=" << errno << endl;
    if (read(readFD, &this->bufferSize, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading bufferSize with errno=" << errno << endl;
    if (read(readFD, &this->bloomSize, sizeof(int)) == -1)
        cout << "Monitor " << this->id << " error in reading bloomSize with errno=" << errno << endl;
    cout << "Monitor " << this->id << ", bufferSize=" << this->bufferSize << ", bloomSize=" << this->bloomSize << endl;
}

void Monitor::receiveCountries() {
    while (1) {
        int sizeOfCountry;
        if (read(readFD, &sizeOfCountry, sizeof(int)) == -1)
            cout << "Monitor " << this->id << " error in reading sizeOfCountry with errno=" << errno << endl;

        if (sizeOfCountry == -1)
            break;

        string country = "";
        for (int i = 0;i <= sizeOfCountry / this->bufferSize;i++) {
            char buff[bufferSize];
            if (read(readFD, &buff, bufferSize) == -1)
                cout << "Monitor " << this->id << " error in reading buff in receiveCountries with errno=" << errno << endl;
            buff[bufferSize] = '\0';
            country.append(buff);
            // cout << "Monitor " << this->id << " readed=" << buff << endl;
        }

        this->addCountry(country);
        cout << "Monitor " << this->id << " got country=" << country << endl;
    }
}

void Monitor::addCountry(string c) {
    if (this->countries == NULL)
        this->countries = new stringList(c);
    else
        this->countries = this->countries->add(c);
}

void Monitor::addVirus(string v) {
    if (this->viruses == NULL)
        this->viruses = new stringList(v);
    else
        this->viruses = this->viruses->add(v);
}

void Monitor::printAllCountries() {
    cout << "Monitor " << this->id << " has the countries" << endl;
    this->countries->print();
}

void Monitor::printAllViruses() {
    cout << "Monitor " << this->id << " has the viruses" << endl;
    this->viruses->print();
}