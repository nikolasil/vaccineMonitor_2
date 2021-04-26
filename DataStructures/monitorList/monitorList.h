/*
    This files contains the monitorList class that is
    used to store the virus and countries names to avoid country duplication.

    Every time a country name or a virus name is needed a pointer to the node is made.
*/

#ifndef MONITORLIST_H
#define MONITORLIST_H

#include <iostream>
#include <string>

using namespace std;

class monitorList
{
public:
    monitorList();
    monitorList(string d, int m);
    ~monitorList();

    void print();     // print this and all the next nodes

    monitorList* add(string d, int m);
    monitorList* remove(string d);
    monitorList* search(string d);

    // GETTERS
    string getCountry() { return this->country; }
    monitorList* getNext() { return this->next; }
    int getMonitor() { return this->monitor; }

    // SETTERS
    void setCountry(string s) { this->country = s; }
    void setNext(monitorList* n) { this->next = n; }
    void setMonitor(int m) { this->monitor = m; }

private:
    string country;
    int monitor;
    monitorList* next;
};

#endif