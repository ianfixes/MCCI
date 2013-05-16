
#pragma once

#include "MCCIRequestBank.h"


/**

  Various RequestBank types employed by MCCI
  
 */


template<typename KeySet>
class SinglePassthruKeyRequestBank : public RequestBankOneKey<KeySet, KeySet>
{
  public:
    SinglePassthruKeyRequestBank(unsigned int max_clients, unsigned int size) :
    RequestBankOneKey<KeySet, KeySet>(max_clients, size) { }

    virtual KeySet get_key(KeySet const key_set) { return key_set; }
};

typedef SinglePassthruKeyRequestBank<MCCI_NODE_ADDRESS_T> HostRequestBank;
typedef SinglePassthruKeyRequestBank<MCCI_VARIABLE_T> VariableRequestBank;


typedef struct {MCCI_NODE_ADDRESS_T host; MCCI_VARIABLE_T var;} HostVarPair;

inline std::ostream& operator<<(std::ostream &out, HostVarPair const &rhs)
{ return out << "(Host " << rhs.host << ", Var " << rhs.var; }
  

class HostVariableRequestBank : public RequestBankOneKey<HostVarPair, uint32_t>
{
  public:
    HostVariableRequestBank(unsigned int max_clients, unsigned int size) :
    RequestBankOneKey<HostVarPair, uint32_t>(max_clients, size) { }

    virtual uint32_t get_key(HostVarPair key_set)
    {
        return (key_set.host << 16) + key_set.var;
    }
};




class DblStuff
{
  public:
    int my1key;
    long my2key;

    DblStuff() {}
    DblStuff(int k1, long k2) { my1key = k1; my2key = k2; }
    ~DblStuff() {}
    
    friend ostream& operator << (ostream &os, const DblStuff& ds)
    {
        os << "(" << ds.my1key << ", " << ds.my2key << ")";
        return os;
    }
};


class DblStuffRequestBank: public RequestBankTwoKeys<DblStuff, int, long>
{
  public:
  DblStuffRequestBank(unsigned int max_clients, unsigned int num_key1s, unsigned int num_key2s)
      : RequestBankTwoKeys<DblStuff, int, long>(max_clients, num_key1s, num_key2s) { }
    virtual ~DblStuffRequestBank() { }
    
    int  get_key_1(DblStuff const key_set) { return key_set.my1key; }
    long get_key_2(DblStuff const key_set) { return key_set.my2key; }

    
};





