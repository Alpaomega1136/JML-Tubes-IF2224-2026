#pragma once
#include <iostream>
#include <vector>
using namespace std;

class TabEntry {
    private:
        string identifier;
        int link;
        int obj;
        int type;
        int ref;
        int nrm;
        int lev;
        int adr;
    public:
        TabEntry();
        TabEntry(string, int, int, int, int, int, int, int);
};

void insertPredefinedIdent();

class ATabEntry {
    private:
        int arrays;
        int xtype;
        int etype;
        int eref;
        int low;
        int high;
        int elsz;
        int size;
    public:
        ATabEntry();
        ATabEntry(int, int, int, int, int, int, int, int);
};

class BTabEntry {
    private:
        int blocks;
        int last;
        int lpar;
        int psze;
        int vsze;
    public:
        BTabEntry();
        BTabEntry(int, int, int, int, int);
};
