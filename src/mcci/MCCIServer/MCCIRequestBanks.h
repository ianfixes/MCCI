
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

    virtual KeySet get_key(KeySet const key_set) const { return key_set; }
};

typedef SinglePassthruKeyRequestBank<bool>                AllRequestBank;
typedef SinglePassthruKeyRequestBank<MCCI_NODE_ADDRESS_T> HostRequestBank;
typedef SinglePassthruKeyRequestBank<MCCI_VARIABLE_T>     VariableRequestBank;



typedef struct {MCCI_NODE_ADDRESS_T host; MCCI_VARIABLE_T var;} HostVarPair;

inline std::ostream& operator<<(std::ostream &out, HostVarPair const &rhs)
{ return out << "(Host " << rhs.host << ", Var " << rhs.var << ")"; }
  

class HostVariableRequestBank : public RequestBankOneKey<HostVarPair, uint32_t>
{
  public:
    HostVariableRequestBank(unsigned int max_clients, unsigned int size) :
    RequestBankOneKey<HostVarPair, uint32_t>(max_clients, size) { }

    virtual uint32_t get_key(HostVarPair const key_set) const
    {
        return (key_set.host << 16) + key_set.var;
    }
};



typedef struct {MCCI_VARIABLE_T var; MCCI_REVISION_T rev; } VarRevPair;

inline std::ostream& operator<<(std::ostream &out, VarRevPair const &rhs)
{ return out << "(Var " << rhs.var << ", Rev " << rhs.rev << ")"; }
  

class VariableRevisionRequestBank
: public RequestBankTwoKeys<VarRevPair, MCCI_VARIABLE_T, MCCI_REVISION_T>
{
  public:
  VariableRevisionRequestBank(unsigned int max_clients, unsigned int size1, unsigned int size2) :
    RequestBankTwoKeys<VarRevPair, MCCI_VARIABLE_T, MCCI_REVISION_T> (max_clients, size1, size2) { }

    virtual MCCI_VARIABLE_T get_key_1(VarRevPair const key_set) const
    {
        return key_set.var;
    }

    virtual MCCI_REVISION_T get_key_2(VarRevPair const key_set) const
    {
        return key_set.rev;
    }
};

typedef struct {MCCI_NODE_ADDRESS_T host; MCCI_VARIABLE_T var; MCCI_REVISION_T rev; } HostVarRevTuple;

inline std::ostream& operator<<(std::ostream &out, HostVarRevTuple const &rhs)
{ return out << "(Host " << rhs.host << ", Var " << rhs.var << ", Rev " << rhs.rev << ")"; }



class RemoteRevisionRequestBank
: public RequestBankTwoKeys<HostVarRevTuple, uint32_t, MCCI_REVISION_T>
{
  public:
  RemoteRevisionRequestBank(unsigned int max_clients, unsigned int size1, unsigned int size2) :
    RequestBankTwoKeys<HostVarRevTuple, uint32_t, MCCI_REVISION_T> (max_clients, size1, size2) { }

    virtual uint32_t get_key_1(HostVarRevTuple const key_set) const
    {
        return (key_set.host << 16) + key_set.var;
    }
    
    virtual MCCI_REVISION_T get_key_2(HostVarRevTuple const key_set) const
    {
        return key_set.rev;
    }
};

    






