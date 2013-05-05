
#pragma once

/*

  RequestBank functions to implement

  - construct (given size)

  - add(client, timeout, key??) // key is the host, var, seq, etc
  
  - update_timeout(client, timeout, key) // SAME THING

  - get_next_expiration()

  - remove_next_expiration()

  - peek(key) // all clients
  
  - remove(key) // all clients

  - get_clients(key)
  
  - get the key (REQUEST) of a given request


 */

#include "MCCITypes.h"
#include "LinearHash.h"
#include "FibonacciHeap.h"
#include <map>

template<typename KeySet>
class RequestBank
{

  protected:
    FibonacciHeap<MCCI_TIME_T, KeySet> m_timeouts;

  public:
    RequestBank(unsigned int size) { };
    virtual ~RequestBank() {};
       
    virtual int add(KeySet const key_set, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout) = 0;

};


template<typename KeySet, typename Key1>
    class RequestBankOneKey : public RequestBank<KeySet>
{

  protected:
    LinearHash<Key1,
               map<MCCI_CLIENT_ID_T,
                   FibonacciHeapNode<MCCI_TIME_T, KeySet>* > > m_bank;



  public:
    RequestBankOneKey(unsigned int size) : RequestBank<KeySet>(size) { m_bank.resize_nearest_prime(size); };
    virtual ~RequestBankOneKey() {};
       
    virtual Key1 get_key(KeySet const key_set) = 0;

    int add(KeySet const key_set, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        Key1 k = this->get_key(key_set);

        FibonacciHeapNode<MCCI_TIME_T, KeySet>* new_node;

        new_node = new FibonacciHeapNode<MCCI_TIME_T, KeySet>();

        
        
        return 0;  // OK
    };
};





template<typename KeySet, typename Key1, typename Key2>
    class RequestBankTwoKeys : public RequestBank<KeySet>
{

  protected:

//    LinearHash<Key2,
//        (map<MCCI_CLIENT_ID_T,
//         (FibonacciHeapNode<MCCI_TIME_T, KeySet>*) > ) > m_bank;


  public:
    RequestBankTwoKeys(unsigned int size) : RequestBank<KeySet>(size) {};
    virtual ~RequestBankTwoKeys() {};
    
    virtual Key1 get_key_1(KeySet const key_set) = 0;
    virtual Key2 get_key_2(KeySet const key_set) = 0;

    int add(KeySet const key_set, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);

        return k1 != 0 && k2 != 0;
    }
};




class HostRequestBank : public RequestBankOneKey<MCCI_NODE_ADDRESS_T, MCCI_NODE_ADDRESS_T>
{
  public:
    HostRequestBank(unsigned int size) : RequestBankOneKey<MCCI_NODE_ADDRESS_T, MCCI_NODE_ADDRESS_T>(size) { };

    MCCI_NODE_ADDRESS_T get_key(MCCI_NODE_ADDRESS_T key_set) { return key_set; };
};



class DblStuff
{
  public:
    int my1key;
    long my2key;

    DblStuff() {};
    DblStuff(int k1, long k2) { my1key = k1; my2key = k2; };
    ~DblStuff() {};
    
    friend ostream& operator << (ostream &os, const DblStuff& ds)
    {
        os << "(" << ds.my1key << ", " << ds.my2key << ")";
        return os;
    }
};


class DblStuffRequestBank: public RequestBankTwoKeys<DblStuff, int, long>
{
  public:
    DblStuffRequestBank(unsigned int size) : RequestBankTwoKeys<DblStuff, int, long>(size){ };
    virtual ~DblStuffRequestBank() { };
    
    int  get_key_1(DblStuff const key_set) { return key_set.my1key; };
    long get_key_2(DblStuff const key_set) { return key_set.my2key; };

    
};

