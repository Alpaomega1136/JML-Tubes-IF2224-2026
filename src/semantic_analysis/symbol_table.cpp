#include "symbol_table.hpp"


vector<TabEntry> tab;
vector<ATabEntry> atab;
vector<BTabEntry> btab;

TabEntry::TabEntry() : identifier(""), link(0), obj(0), type(0), ref(0), nrm(0), lev(0), adr(0) {}

TabEntry::TabEntry(string ident, int link, int obj, int type, int ref, int nrm, int lev, int adr) :
                       identifier(ident), link(link), obj(obj), type(type), ref(ref), nrm(nrm),
                       lev(lev), adr(adr) {}


ATabEntry::ATabEntry() : arrays(0), xtype(0), etype(0), eref(0), low(0), high(0), elsz(0), size(0) {}

ATabEntry::ATabEntry(int arrays, int xtype, int etype, int eref, int low, int high, int elsz, int size) :
                         arrays(arrays), xtype(xtype), etype(etype), eref(eref), low(low), high(high),
                         elsz(elsz), size(size) {}

BTabEntry::BTabEntry() : blocks(0), last(0), lpar(0), psze(0), vsze(0) {}

BTabEntry::BTabEntry(int blocks, int last, int lpar, int psze, int vsze) :
                         blocks(blocks), last(last), lpar(lpar), psze(psze),
                         vsze(vsze) {}