#include <iostream>

#include "../../util.h"
#include "fifoFDList.h"

fifoFDList::fifoFDList() {}

fifoFDList::fifoFDList(int r, int w) : readFD(r), writeFD(w) {
    this->next = NULL;
}

fifoFDList::~fifoFDList()
{
    if (this->getNext() != NULL)
        delete this->getNext();
}

fifoFDList* fifoFDList::add(int r, int w)
{
    fifoFDList* temp = this;
    while (temp->getNext() != NULL)
        temp = temp->getNext();

    fifoFDList* new_node = new fifoFDList(r, w);
    checkNew(new_node);
    temp->setNext(new_node);
    return new_node;
}

int fifoFDList::getReadFifo(int m) {
    fifoFDList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp->getReadFD();
}

int fifoFDList::getWriteFifo(int m) {
    fifoFDList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp->getWriteFD();
}