#include <iostream>

#include "../../util.h"
#include "monitorList.h"

monitorList::monitorList() {}

monitorList::monitorList(int r, int w) : readFD(r), writeFD(w) {
    this->next = NULL;
}

monitorList::~monitorList()
{
    if (this->getNext() != NULL)
        delete this->getNext();
}

monitorList* monitorList::add(int r, int w)
{
    monitorList* temp = this;
    while (temp->getNext() != NULL)
        temp = temp->getNext();

    monitorList* new_node = new monitorList(r, w);
    checkNew(new_node);
    temp->setNext(new_node);
    return new_node;
}

int monitorList::getReadFifo(int m) {
    monitorList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp->getReadFD();
}

int monitorList::getWriteFifo(int m) {
    monitorList* temp = this;
    for (int i = 0;i < m;i++)
        temp = temp->getNext();

    return temp->getWriteFD();
}