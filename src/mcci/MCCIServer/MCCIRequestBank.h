
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

template<typename Input, typename Key1>
class RequestBank
{

  protected:
    FibonacciHeap<MCCI_TIME_T, Key1> m_timeouts;
    LinearHash<Key1,
               map<MCCI_CLIENT_ID_T,
                   FibonacciHeapNode<MCCI_TIME_T, Key1>*
               > > m_bank;

  public:
    RequestBank(unsigned int size) { m_bank.resize_nearest_prime(size); };
    virtual ~RequestBank() {};
       
    virtual Key1 get_key_1(Input const input) = 0;

    int add(Input const input, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        Key1 k = this->get_key_1(input);

        FibonacciHeapNode<MCCI_TIME_T, Key1>* new_node;

        new_node = new FibonacciHeapNode<MCCI_TIME_T, Key1>();

        
        
        return 0;  // OK
    };
};




template<typename Input, typename Key1, typename Key2>
    class RequestBankTwoKeys : public RequestBank<Input, Key1>
{
  public:
    RequestBankTwoKeys(unsigned int size) : RequestBank<Input, Key1>(size) {};
    virtual ~RequestBankTwoKeys() {};
    
    virtual Key2 get_key_2(Input const input) = 0;

    int add(Input const input, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        Key1 k1 = this->get_key_1(input);
        Key2 k2 = this->get_key_2(input);

        return k1 != 0 && k2 != 0;
    }
};




class HostRequestBank : public RequestBank<MCCI_NODE_ADDRESS_T, MCCI_NODE_ADDRESS_T>
{
  public:
    HostRequestBank(unsigned int size) : RequestBank<MCCI_NODE_ADDRESS_T, MCCI_NODE_ADDRESS_T>(size) { };
    
};



typedef struct
{
    int my1key;
    long my2key;
} DblStuff;


class DblStuffRequestBank: public RequestBankTwoKeys<DblStuff, int, long>
{
  public:
    DblStuffRequestBank(unsigned int size) : RequestBankTwoKeys<DblStuff, int, long>(size){ };
    virtual ~DblStuffRequestBank() { };
    
    int  get_key_1(DblStuff const input) { return input.my1key; };
    long get_key_2(DblStuff const input) { return input.my2key; };

    
};

