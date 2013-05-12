
#pragma once

/*

  RequestBank functions to implement

  - peek(key) // all clients
  


 */

#include "MCCITypes.h"
#include "LinearHash.h"
#include "FibonacciHeap.h"
#include <map>
#include <list>

using namespace std;

template<typename KeySet>
class RequestBank
{
    typedef struct
    {
        KeySet key_set;
        MCCI_CLIENT_ID_T client_id;
    } LookupSet;
    
    typedef FibonacciHeapNode<MCCI_TIME_T, LookupSet> HeapNode;

    typedef map<MCCI_CLIENT_ID_T, HeapNode*> SubscriptionMap;

    typedef typename SubscriptionMap::iterator SubscriptionMapIterator;
    
  private:
    unsigned int* m_outstanding_requests;
    FibonacciHeap<MCCI_TIME_T, KeySet> m_timeouts;
    

  public:
    RequestBank(unsigned int size, unsigned int max_client_id)
    {
        this->m_outstanding_requests = new unsigned int[size]();
        this->on_init(size, max_client_id);  // delegate
    }
    
    virtual ~RequestBank() { delete[] this->m_outstanding_requests; }
    
    // developer tool to check sanity of a RequestBank
    virtual bool check_sanity() const = 0;
    
    // add (OR UPDATE) an entry in the request bank
    int add(KeySet const key_set, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        HeapNode* n = this->get_node(key_set, client_id);
        
        // early exit for brand new nodes; just add them
        if (NULL == n)
        {
            LookupSet l;
            l.key_set = key_set;
            l.client_id = client_id;
            n = new HeapNode(timeout, l);

            if (!n)
                throw string("Couldn't allocate memory for new node");

            this->m_timeouts.insert_node(n);
            int ret = this->add_by_fq(key_set, client_id, n);
            
            if (0 == ret)
                this->m_outstanding_requests[client_id] += 1; // add what wasn't there

            return ret;
        }
        
        // assert n->data().key_set == key_set
        // assert n->data().client_id == client_id
        
        this->m_timeouts.alter_key(n, timeout, 0);

        return 0;  // no action
    }

    
    // get the timeout of the node that will expire first
    MCCI_TIME_T minimum_timeout() const { return this->m_timeouts.minimum()->key(); }

    // remove the request that's expiring first
    void remove_minimum()
    {
        HeapNode* min   = this->m_timeouts.minimum();
        HeapNode* keyed = this->remove_by_fq(min->data().key_set, min->data().client_id);
        
        if (min != keyed)  // TODO: better assertion format?
            throw string("Failed assertion that node->key == key->node");

        this->m_timeouts.remove(min, 0);
        this->m_outstanding_reqeusts[min->data().client_id] -= 1;
    }

    
    // remove a set of subscribed clients by their key (e.g. when data is delivered)
    void remove_by_key(KeySet const key_set)
    {
        SubscriptionMap* removals;
        
        removals = this->get_by_pq(key_set);

        // remove all the nodes and adjust the open requests listing
        for (SubscriptionMapIterator it = removals->begin(); it != removals->end(); ++it)
        {
            this->m_outstanding_requests[it->first] =- 1;
            this->m_timeouts.remove(it->second, 0);
        }

        // remove all custom structure nodes in one shot
        this->remove_by_pq(key_set);
    }
    
    // does this structure contain the given node?
    bool contains(KeySet const key_set, MCCI_CLIENT_ID_T client_id) const
    {
        return this->get_by_fq(key_set, client_id);
    }

    // number of open requests for a given client
    unsigned int get_outstanding_request_count(MCCI_CLIENT_ID_T client_id) const
    {
        return this->m_outstanding_requests[client_id];
    }


    
    class subscriber_iterator : public std::iterator<std::input_iterator_tag, MCCI_CLIENT_ID_T>
    {

        SubscriptionMapIterator it;
        
      public:
        
        //subscriber_iterator(MCCI_CLIENT_ID_T* x) : p(x) {}
      subscriber_iterator(const subscriber_iterator& mit) : it(mit.it) {}
        
        subscriber_iterator& operator++()
        {
            ++it;
            return *this;
        }

        subscriber_iterator operator++(MCCI_CLIENT_ID_T)
        {
            subscriber_iterator tmp(*this);
            operator++(); return tmp;
        }
        bool operator==(const subscriber_iterator& rhs) { return it == rhs.it; }
        bool operator!=(const subscriber_iterator& rhs) { return it != rhs.it; }
        MCCI_CLIENT_ID_T operator*()
        {
            return it->first;
        }
    };



    subscriber_iterator begin(KeySet const key_set) const
    {
        SubscriptionMapIterator it;
        it = this->get_by_pq(key_set).begin();
        if (!it) return NULL;
        return &(it->first);
    }
    
    subscriber_iterator end(KeySet const key_set) const
    {
        SubscriptionMapIterator it;
        it = this->get_by_pq(key_set).end();
        if (!it) return NULL;
        return (&it->first);
    }

    
  protected:

    // perfom any implementation-specific init (probably involving the size arg)
    virtual void on_init(unsigned int size, unsigned int max_client_id) {}
    
    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) = 0;

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const key_set) = 0;
    
    // return a pointer 
    
    // assume that this entry is unique and add it to the structure
    virtual int add_only(KeySet const key_set,
                         MCCI_CLIENT_ID_T client_id,
                         HeapNode* const node_ptr) = 0;

    // remove a node from the custom container (not the heap) based on its key
    virtual HeapNode* remove_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) = 0;

    // remove a partially-qualified set of nodes from the custom container
    virtual void remove_by_pq(KeySet const key_set) = 0;

};



template<typename KeySet, typename Key1>
    class RequestBankOneKey : public RequestBank<KeySet>
{

  protected:
    LinearHash<Key1,
               map<MCCI_CLIENT_ID_T,
                   FibonacciHeapNode<MCCI_TIME_T, KeySet>* > > m_bank;



  public:
    RequestBankOneKey(unsigned int size, unsigned int max_clients)
        : RequestBank<KeySet>(size, max_clients) { m_bank.resize_nearest_prime(size); }
    virtual ~RequestBankOneKey() {}
       
    virtual Key1 get_key(KeySet const key_set) = 0;

    int add_only(KeySet const key_set, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        Key1 k = this->get_key(key_set);

        FibonacciHeapNode<MCCI_TIME_T, KeySet>* new_node;

        new_node = new FibonacciHeapNode<MCCI_TIME_T, KeySet>();

        
        
        return 0;  // OK
    }
};





template<typename KeySet, typename Key1, typename Key2>
    class RequestBankTwoKeys : public RequestBank<KeySet>
{

  protected:

//    LinearHash<Key2,
//        (map<MCCI_CLIENT_ID_T,
//         (FibonacciHeapNode<MCCI_TIME_T, KeySet>*) > ) > m_bank;


  public:
    RequestBankTwoKeys(unsigned int size, unsigned int max_clients)
      : RequestBank<KeySet>(size, max_clients) {}
    virtual ~RequestBankTwoKeys() {}
    
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
    HostRequestBank(unsigned int size, unsigned int max_clients)
      : RequestBankOneKey<MCCI_NODE_ADDRESS_T, MCCI_NODE_ADDRESS_T>(size, max_clients) { }

    MCCI_NODE_ADDRESS_T get_key(MCCI_NODE_ADDRESS_T key_set) { return key_set; }
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
    DblStuffRequestBank(unsigned int size, unsigned int max_clients)
        : RequestBankTwoKeys<DblStuff, int, long>(size, max_clients){ }
    virtual ~DblStuffRequestBank() { }
    
    int  get_key_1(DblStuff const key_set) { return key_set.my1key; }
    long get_key_2(DblStuff const key_set) { return key_set.my2key; }

    
};

