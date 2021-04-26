#include <iostream>
#include <string>

#include "../../util.h"
#include "monitorList.h"


monitorList::monitorList() : country("") {}

monitorList::monitorList(string d, int m) : country(d), monitor(m) {}

monitorList::~monitorList()
{
    if (this->getNext() != NULL)
        delete this->getNext();
}

monitorList* monitorList::add(string d, int m)
{
    monitorList* new_node = new monitorList(d, m);
    checkNew(new_node);
    new_node->setNext(this);
    return new_node;
}

monitorList* monitorList::search(string d)
{
    monitorList* temp = this;
    while (temp != NULL)
    {
        if (temp->getCountry().compare(d) == 0)
            return temp;

        temp = temp->getNext();
    }
    return temp;
}

void monitorList::print()
{
    monitorList* temp = this;
    while (temp != NULL)
    {
        cout << temp->country << " is assinged to monitor " << temp->monitor << endl;;
        temp = temp->getNext();
    }
    cout << endl;
}