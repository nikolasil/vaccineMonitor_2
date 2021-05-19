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
#include <stdio.h>
#include <stdlib.h>

#include "travelMonitor.h" 
#include "util.h" 
#include "DataStructures/bloomFilter/bloomFilter.h"
#include "DataStructures/date/date.h"

void signal_handler(int signo) {
    cout << "TravelMonitor Signal Handler with signo= " << signo << endl;
    if (signo == 2 || signo == 3) { // SIGINT || SIGQUIT
        cout << "SIGINT || SIGQUIT" << endl;
        exit(1);
    }
    else if (signo == 20 || signo == 17 || signo == 18) { // SIGCHLD
        cout << "SIGCHLD" << endl;
    }
    else {
        cout << "Error signal not supposed to be in sigaction" << endl;
    }
}

void travelMonitor::sendSIGUSR1(int monitor) {
    kill(this->monitors->getPID(monitor), SIGUSR1);
}

void travelMonitor::sendSIGINT(int monitor) {
    kill(this->monitors->getPID(monitor), SIGINT);
}

travelMonitor::travelMonitor(int m, int b, int s, string dir) : numMonitors(m), bufferSize(b), sizeOfBloom(s), input_dir(dir) {
    cout << "numMonitors=" << this->numMonitors << ", bufferSize= " << this->bufferSize << ", sizeOfBloom= " << this->sizeOfBloom << ", input_dir= " << this->input_dir << endl;
    this->countryToMonitor = NULL;
    this->monitors = NULL;
    this->viruses = new stringList();
    checkNew(this->viruses);
    this->blooms = new bloomFilterList(this->sizeOfBloom);
    checkNew(this->blooms);

    // this->handler.sa_handler = signal_handler;
    // sigemptyset(&(handler.sa_mask));
    // sigaction(SIGCHLD, &this->handler, NULL);
    // sigaction(SIGINT, &this->handler, NULL);
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
            this->addMonitor(c_pid, i);
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
        this->addFdToMonitor(i, fd1, fd0);
    }
}

void travelMonitor::sendCredentials() {
    for (int i = 0;i < numMonitors;i++) {
        int fd = this->monitors->getWriteFifo(i);
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
    int monitor = 0;
    int count;

    struct dirent** coutriesDir;
    count = scandir(this->input_dir.c_str(), &coutriesDir, NULL, alphasort);
    if (count < 0)
        perror("scandir");
    else {
        for (int i = 0;i < count;i++) {
            string country = coutriesDir[i]->d_name;
            if (country.compare("..") == 0 || country.compare(".") == 0)
                continue;

            cout << country << "->" << monitor << endl;
            this->addCountryToMonitor(country, monitor);

            cout << "Sending to monitor " << monitor << " the country " << country << endl;
            sendStr(monitor, country);

            monitor++;
            if (monitor >= this->numMonitors)
                monitor = 0;
            free(coutriesDir[i]);
        }
        free(coutriesDir);
    }

    int fd;
    for (int i = 0;i < this->numMonitors;i++) {
        fd = this->monitors->getWriteFifo(i);
        int end = -1;
        if (write(fd, &end, sizeof(int)) == -1)
            cout << "Error in writting end with errno=" << errno << endl;
    }
}

void travelMonitor::receiveBlooms() {
    fd_set fileDecriptorSet;

    int totalRead = 0;
    int flag = 0;
    while (!flag) {
        FD_ZERO(&fileDecriptorSet);
        for (int i = 0;i < this->numMonitors;i++) {
            FD_SET(this->monitors->getReadFifo(i), &fileDecriptorSet);
        }
        select(this->monitors->getReadFifo(this->numMonitors - 1) + 1, &fileDecriptorSet, NULL, NULL, NULL);

        for (int i = 0;i < this->numMonitors;i++) {
            if (FD_ISSET(this->monitors->getReadFifo(i), &fileDecriptorSet)) {
                cout << "Monitor" << i << " is ready!" << endl;
                int end = 0;
                while (1) {
                    string virus = receiveManyStr(i, &end);
                    if (end == -1 || virus == "")
                        break;
                    addNewVirus(virus);
                    cout << "Got virus=" << virus << " from Monitor " << i << endl;
                }
                cout << "Updating blooms from Monitor " << i << endl;
                while (1) {

                    string virus = receiveStr(i);

                    if (virus.compare("END BLOOMS") == 0) {
                        cout << "Done updating blooms" << endl;
                        break;
                    }

                    int pos = 0;
                    int fd = this->monitors->getReadFifo(i);
                    char* bloomArray = this->blooms->getBloom(this->viruses->search(virus))->getArray();
                    for (int i = 0;i <= this->sizeOfBloom / this->bufferSize;i++) {
                        char buff[this->bufferSize];
                        if (read(fd, &buff, this->bufferSize) == -1)
                            cout << "Error in reading bit with errno=" << errno << endl;

                        for (int i = 0; i < this->bufferSize;i++)
                            bloomArray[pos + i] = bloomArray[pos + i] | buff[i];

                        pos += this->bufferSize;
                    }
                    cout << virus << " ";
                    this->blooms->getBloom(this->viruses->search(virus))->print();
                }
                totalRead++;
            }
            if (totalRead == this->numMonitors) {
                flag = 1;
                break;
            }
        }
    }

    sendDone();
}

void travelMonitor::sendDone() {
    for (int i = 0;i < numMonitors;i++)
        sendStr(i, "DONE");
}

void travelMonitor::startMenu() {
    do
    {
        cout << endl;
        cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
        cout << "Available commands" << endl;
        cout << "   /travelRequest citizenID date countryFrom countryTo virusName" << endl;
        cout << "   /travelStats virusName date1 date2 [country]" << endl;
        cout << "   /addVaccinationRecords country" << endl;
        cout << "   /searchVaccinationStatus citizenID" << endl;
        cout << "   /exit" << endl;
        cout << endl;
        string input = getInput("Enter command: ");
        int length;
        string* command = readString(input, &length);
        if (length > 0)
        {
            if (command[0].compare("/travelRequest") == 0)
                this->travelRequest(command, length);

            else if (command[0].compare("/travelStats") == 0)
                this->travelStats(command, length);

            else if (command[0].compare("/addVaccinationRecords") == 0)
                this->addVaccinationRecords(command, length);

            else if (command[0].compare("/searchVaccinationStatus") == 0)
                this->searchVaccinationStatus(command, length);

            else if (command[0].compare("/exit") == 0)
            {
                this->terminate();
                delete[] command;
                break;
            }
            else
                cout << "Invalid command!" << endl;
        }
        delete[] command;
    } while (1);
}


void travelMonitor::travelRequest(string* arguments, int length) {
    cout << endl;
    cout << "- Selected: /travelRequest" << endl;
    if (length == 6)
    {
        int id;
        if (arguments[1].find_first_not_of("0123456789") != -1)
        {
            cout << "ERROR citizenID must be given as first argument && it must be a 4 digit integer" << endl;
            return;
        }
        else if (stoi(arguments[1]) > 9999 && stoi(arguments[1]) <= 0)
        {
            cout << "ERROR citizenID must be given as first argument && it must be a 4 digit integer" << endl;
            return;
        }
        id = stoi(arguments[1]);
        date checkDate = date((arguments[2]));
        if (!checkDate.isValid()) {
            cout << "ERROR date must be given as second argument && it must be in in correct format (dd-mm-yyyy)" << endl;
            return;
        }
        string countryFrom = arguments[3];
        string countryTo = arguments[4];
        string virusName = arguments[5];

        cout << "- citizenID: " << id << endl;
        cout << "- date: ";
        checkDate.print();
        cout << endl;
        cout << "- countryFrom: " << countryFrom << endl;
        cout << "- countryTo: " << countryTo << endl;
        cout << "- virusName: " << virusName << endl;
        cout << endl;

        int res;
        stringList* v = this->viruses->search(virusName);
        if (v != NULL)
            res = this->blooms->getBloom(v)->check(id);
        else
        {
            cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
            return;
        }

        if (res)
        {
            string s = "/travelRequest ";
            s.append(to_string(id) + " ");
            s.append(checkDate.getConcatenate() + " ");
            s.append(countryFrom + " ");
            s.append(countryTo + " ");
            s.append(virusName);
            if (countryToMonitor->search(countryFrom) != NULL) {
                sendStr(countryToMonitor->search(countryFrom)->getMonitor(), s);
            }
            else {
                cout << "countryFrom " << countryFrom << " dont exists" << endl;
            }
            return;
        }
        else
        {
            cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
            return;
        }
    }
    else
    {
        cout << "ERROR 5 Arguments must be given" << endl;
        cout << "ERROR Total arguments given was: " << length - 1 << endl;
        return;
    }
}

void travelMonitor::travelStats(string* arguments, int length) {

}

void travelMonitor::addVaccinationRecords(string* arguments, int length) {

}

void travelMonitor::searchVaccinationStatus(string* arguments, int length) {

}

void travelMonitor::terminate() {

}

void travelMonitor::sendStr(int monitor, string str) {
    int fd = this->monitors->getWriteFifo(monitor);
    char* to_tranfer = &str[0];
    int sizeOfStr = strlen(to_tranfer);

    if (write(fd, &sizeOfStr, sizeof(int)) == -1)
        cout << "Error in writting sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr > this->bufferSize) {
        int pos = 0;
        for (int i = 0;i <= strlen(to_tranfer) / this->bufferSize;i++) {
            if (write(fd, &to_tranfer[pos], this->bufferSize) == -1)
                cout << "Error in writting to_tranfer with errno=" << errno << endl;

            pos += this->bufferSize;
        }
    }
    else
        if (write(fd, &to_tranfer[0], sizeOfStr) == -1)
            cout << "Error in writting to_tranfer with errno=" << errno << endl;

}

string travelMonitor::receiveStr(int monitor) {
    int fd = this->monitors->getReadFifo(monitor);
    int sizeOfStr;
    if (read(fd, &sizeOfStr, sizeof(int)) == -1)
        cout << "Error in reading sizeOfStr with errno=" << errno << endl;

    string str = "";
    if (sizeOfStr > this->bufferSize) {
        for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
            char buff[this->bufferSize + 1];
            if (read(fd, &buff[0], this->bufferSize) == -1)
                cout << "Error in reading buff with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(fd, &buff[0], sizeOfStr) == -1)
            cout << "Error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

string travelMonitor::receiveManyStr(int monitor, int* end) {
    int fd = this->monitors->getReadFifo(monitor);
    int sizeOfStr;
    if (read(fd, &sizeOfStr, sizeof(int)) == -1)
        cout << "Error in reading sizeOfStr with errno=" << errno << endl;

    if (sizeOfStr == -1) {
        *end = -1;
        return "";
    }

    string str = "";
    if (sizeOfStr > this->bufferSize) {
        for (int i = 0;i <= sizeOfStr / this->bufferSize;i++) {
            char buff[this->bufferSize + 1];
            if (read(fd, &buff[0], this->bufferSize) == -1)
                cout << "Error in reading buff with errno=" << errno << endl;
            buff[this->bufferSize] = '\0';
            str.append(buff);
        }
    }
    else {
        char buff[sizeOfStr + 1];
        if (read(fd, &buff[0], sizeOfStr) == -1)
            cout << "Error in reading buff with errno=" << errno << endl;
        buff[sizeOfStr] = '\0';
        str.append(buff);
    }
    return str;
}

void travelMonitor::addCountryToMonitor(string c, int m) {
    if (this->countryToMonitor == NULL)
        this->countryToMonitor = new monitorCountryPairList(c, m);
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

void travelMonitor::addMonitor(int pid, int id) {
    if (this->monitors == NULL)
        this->monitors = new monitorList(pid, id);
    else
        this->monitors->add(pid, id);
}

void travelMonitor::addFdToMonitor(int m, int r, int w) {
    this->monitors->addFD(m, r, w);
}

void travelMonitor::printAllViruses() {
    cout << "ALL THE viruses: ";
    this->viruses->print();
    cout << endl;
}

void travelMonitor::printCountryToMonitor() {
    this->countryToMonitor->print();
}