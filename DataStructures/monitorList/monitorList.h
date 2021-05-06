/*
    This files contains the monitorList class that is
    used to store the virus and countries names to avoid country duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef monitorList_H
#define monitorList_H

#include <string>

using namespace std;

class monitorList
{
public:
    monitorList();
    monitorList(int r, int w);
    ~monitorList();

    monitorList* add(int r, int w);
    int getReadFifo(int m);
    int getWriteFifo(int m);

    // GETTERS
    int getReadFD() { return this->readFD; }
    int getWriteFD() { return this->writeFD; }
    monitorList* getNext() { return this->next; }

    // SETTERS
    void setReadFD(int r) { this->readFD = r; }
    void setWriteFD(int w) { this->writeFD = w; }
    void setNext(monitorList* n) { this->next = n; }

private:
    int readFD;
    int writeFD;

    monitorList* next;
};

#endif