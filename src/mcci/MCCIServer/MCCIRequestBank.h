
#pragma once

#include "MCCITypes.h"
#include "LinearHash.h"
#include "FibonacciHeap.h"
#include <map>
#include <list>

using namespace std;

/**
   This templated base class defines a set of requests that can be added by a
   given key (key set, implementation depending), and removed both by the key 
   and by the passing of time.  the number of open requests per subscriber is
   tracked.
 */
template<typename KeySet>
class RequestBank
{
  public:
    // convenience struct to hold the fully-qualifying lookup information
    typedef struct
    {
        KeySet key_set;
        MCCI_CLIENT_ID_T client_id;
    } LookupSet;

    // holds the time-sensitive view of the data
    typedef FibonacciHeapNode<MCCI_TIME_T, LookupSet> HeapNode;

    // holds the subscription information
    typedef map<MCCI_CLIENT_ID_T, HeapNode*> SubscriptionMap;

    // for iterating over subscriber information
    typedef typename SubscriptionMap::const_iterator SubscriptionMapIterator;
    
  private:
    unsigned int* m_outstanding_requests;
    FibonacciHeap<MCCI_TIME_T, KeySet> m_timeouts;
    

  public:
    RequestBank(unsigned int max_client_id)
    {
        this->m_outstanding_requests = new unsigned int[max_client_id]();
    }
    
    virtual ~RequestBank() { delete[] this->m_outstanding_requests; }
    
    // developer tool to check sanity of a RequestBank
    //virtual bool check_sanity() const = 0;
    
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
            n = new HeapNode(timeout, l); // ALLOCATE memory

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
        delete min;

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
            delete it->second;  // free the node that was removed
        }

        // remove all custom structure nodes in one shot
        this->remove_by_pq(key_set);
    }
    
    // does this structure contain the given node?
    bool contains(KeySet const key_set, MCCI_CLIENT_ID_T client_id) const
    {
        return this->get_by_fq(key_set, client_id);
    }

    // does this structure contain the given key set?
    bool contains(KeySet const key_set) const
    {
        return this->get_by_pq(key_set);
    }
    
    // number of open requests for a given client
    unsigned int get_outstanding_request_count(MCCI_CLIENT_ID_T client_id) const
    {
        return this->m_outstanding_requests[client_id];
    }

    
    // iterator class that just covers the client ids -- the keys
    class subscriber_iterator : public SubscriptionMapIterator
    {
      public:
        subscriber_iterator() : SubscriptionMapIterator() {}
        subscriber_iterator(SubscriptionMapIterator s) : SubscriptionMapIterator(s) {}

        MCCI_CLIENT_ID_T* operator->()
        { return (MCCI_CLIENT_ID_T* const)&(SubscriptionMapIterator::operator->()->first); }
        
        MCCI_CLIENT_ID_T operator*()
        { return SubscriptionMapIterator::operator*().first; }
    };

    // iteration points: begin
    subscriber_iterator subscribers_begin(KeySet const key_set) const
    {
        return this->get_by_pq(key_set)->begin();
    }

    // iteration points: begin
    subscriber_iterator subscribers_end(KeySet const key_set) const
    {
        return this->get_by_pq(key_set)->end();
    }
    
  protected:

    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) = 0;

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const key_set) = 0;
    
    // assume that this entry is unique and add it to the structure
    virtual int add_by_fq(KeySet const key_set,
                          MCCI_CLIENT_ID_T client_id,
                          HeapNode* const node_ptr) = 0;

    // remove a node from the custom container (not the heap) based on its key
    virtual HeapNode* remove_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) = 0;

    // remove a partially-qualified set of nodes from the custom container (don't delete HeapNodes)
    virtual void remove_by_pq(KeySet const key_set) = 0;

};



// class that stores requests using a single key into a linear hash
template<typename KeySet>
    class RequestBankOneKey : public RequestBank<KeySet>
{

    typedef typename RequestBank<KeySet>::HeapNode HeapNode;
    typedef typename RequestBank<KeySet>::SubscriptionMap SubscriptionMap;
    typedef typename RequestBank<KeySet>::subscriber_iterator subscriber_iterator;
        
  protected:
    typedef LinearHash<KeySet, SubscriptionMap*> LinearHashBank;
    typedef typename LinearHashBank::iterator LinearHashBankIterator;
    
    LinearHashBank m_bank;

  public:
      RequestBankOneKey(unsigned int max_clients, unsigned int size)
          : RequestBank<KeySet>(max_clients)
    {
        m_bank.resize_nearest_prime(size);
    }
    
    virtual ~RequestBankOneKey()
    {
        // free all map objects that exist in LinearHashBank.
        for (LinearHashBankIterator it = m_bank.begin(); it != m_bank.end(); ++it)
            if (NULL != it->second)
                delete it->second;
    }

    // assume that this entry is unique and add it to the structure
    virtual int add_by_fq(KeySet const k,
                          MCCI_CLIENT_ID_T client_id,
                          HeapNode* const node_ptr)
    {
        // init hash entry if it doesn't exist 
        if (!m_bank.has_key(k)) m_bank[k] = new SubscriptionMap();
        if (NULL == m_bank[k]) throw string("Couldn't allocate new SubscriptionMap");
        (*m_bank[k])[client_id] = node_ptr;  // add to map
        return 0;  // OK
    }
    
    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const k, MCCI_CLIENT_ID_T client_id)
    {
        if (!m_bank.has_key(k)) return NULL;
        if (m_bank[k]->find(client_id) == m_bank[k]->end()) return NULL;
        return (*m_bank[k])[client_id];
    }

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const k)
    {
        return m_bank[this->get_key(k)];
    }

    // remove a node from the custom container (not the heap) based on its key
    virtual HeapNode* remove_by_fq(KeySet const k, MCCI_CLIENT_ID_T client_id)
    {
        SubscriptionMap* m = m_bank[k];
        HeapNode* ret = (*m_bank[k])[client_id];
        
        m->erase(client_id);
        
        if (m->empty())
        {
            delete m;
            m_bank[k] = NULL;
        }
        
        return ret;
    }

    // remove a partially-qualified set of nodes from the custom container (don't delete HeapNodes)
    virtual void remove_by_pq(KeySet const k)
    {
        delete m_bank[k];
        m_bank[k] = NULL;
    }
};


typedef RequestBankOneKey<MCCI_NODE_ADDRESS_T> HostRequestBank;
typedef RequestBankOneKey<MCCI_VARIABLE_T> VariableRequestBank;




template<typename KeySet, typename Key1, typename Key2>
    class RequestBankTwoKeys : public RequestBank<KeySet>
{
    typedef typename RequestBank<KeySet>::HeapNode HeapNode;
    typedef typename RequestBank<KeySet>::SubscriptionMap SubscriptionMap;
    typedef typename RequestBank<KeySet>::subscriber_iterator subscriber_iterator;

    typedef LinearHash<Key2, SubscriptionMap*> LinearHashKey2;
    typedef LinearHash<Key1, LinearHashKey2> LinearHashKey1;

    typedef typename LinearHashKey2::iterator LinearHashKey2Iterator;
    typedef typename LinearHashKey1::iterator LinearHashKey1Iterator;
    
  protected:
    LinearHashKey1 m_bank;
    unsigned int m_num_key1;
    unsigned int m_num_key2;

  public:
    RequestBankTwoKeys(unsigned int max_clients, unsigned int num_key1, unsigned int num_key2)
        : RequestBank<KeySet>(max_clients)
    {
        this->m_num_key1 = num_key1;
        this->m_num_key2 = num_key2;
        this->m_bank.resize_nearest_prime(this->m_num_key1);
    }
    
    virtual Key1 get_key_1(KeySet const key_set) = 0;
    virtual Key2 get_key_2(KeySet const key_set) = 0;

    virtual ~RequestBankTwoKeys()
    {
        LinearHashKey1Iterator it1;
        LinearHashKey2Iterator it2;
        
        // free all map objects that exist in LinearHash.
        for (it1 = m_bank.begin(); it1 != m_bank.end(); ++it1)
            for (it2 = m_bank[it1->first].begin(); it2 != m_bank[it1->first].end(); ++it2)
                if (NULL != it2->second)
                    delete it2->second;
    }

    virtual void on_init(unsigned int size) {
    m_bank.resize_nearest_prime(size); }

    // assume that this entry is unique and add it to the structure
    virtual int add_by_fq(KeySet const key_set,
                          MCCI_CLIENT_ID_T client_id,
                          HeapNode* const node_ptr)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);

        // init hash entry if it doesn't exist 
        if (!m_bank.has_key(k1)) m_bank[k1].resize_nearest_prime(this->m_num_key2);
        if (!m_bank[k1].has_key(k2)) m_bank[k1][k2] = new SubscriptionMap();
        if (NULL == m_bank[k1][k2]) throw string("Couldn't allocate new SubscriptionMap");
        (*m_bank[k1][k2])[client_id] = node_ptr;  // add to map
        return 0;  // OK
    }
    
    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);
        if (!m_bank.has_key(k1)) return NULL;
        if (!m_bank[k1].has_key(k2)) return NULL;
        if (m_bank[k1][k2]->find(client_id) == m_bank[k1][k2]->end()) return NULL;
        return (*m_bank[k1][k2])[client_id];
    }

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const key_set)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);
        return m_bank[k1][k2];
    }

    // remove a node from the custom container (not the heap) based on its key
    virtual HeapNode* remove_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);
        SubscriptionMap* m = m_bank[k1][k2];
        HeapNode* ret = (*m_bank[k1][k2])[client_id];
        
        m->erase(client_id);
        
        if (m->empty())
        {
            delete m;
            m_bank[k1][k2] = NULL;
            m_bank[k1].remove(k2);
        }

        if (m_bank[k1].empty())
        {
            m_bank.remove(k1);
        }
        
        return ret;
    }

    // remove a partially-qualified set of nodes from the custom container (don't delete HeapNodes)
    virtual void remove_by_pq(KeySet const key_set)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);

        delete m_bank[k1][k2];
        m_bank[k1][k2] = NULL;
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

