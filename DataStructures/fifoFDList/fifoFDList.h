/*
    This files contains the fifoFDList class that is
    used to store the virus and countries names to avoid country duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef FIFOFDLIST_H
#define FIFOFDLIST_H

#include <string>

using namespace std;

class fifoFDList
{
public:
    fifoFDList();
    fifoFDList(int r, int w);
    ~fifoFDList();

    fifoFDList* add(int r, int w);
    int getReadFifo(int m);
    int getWriteFifo(int m);

    // GETTERS
    int getReadFD() { return this->readFD; }
    int getWriteFD() { return this->writeFD; }
    fifoFDList* getNext() { return this->next; }

    // SETTERS
    void setReadFD(int r) { this->readFD = r; }
    void setWriteFD(int w) { this->writeFD = w; }
    void setNext(fifoFDList* n) { this->next = n; }

private:
    int readFD;
    int writeFD;

    fifoFDList* next;
};

#endif