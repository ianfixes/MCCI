
#pragma once

#include "MCCITypes.h"
#include "LinearHash.h"
#include "FibonacciHeap.h"
#include <map>
#include <list>
#include <ostream>

using namespace std;

// declare class to enable declaration of ostream operator
template <typename KeySet> class RequestBank;
template <typename KeySet>
ostream& operator<<(ostream &, const RequestBank<KeySet>&);


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

    friend std::ostream& operator<<(std::ostream &out, LookupSet const &rhs)
    { return out << "(key_set " << rhs.key_set << ", client_id " << rhs.client_id << ")"; }

    // holds the time-sensitive view of the data
    typedef FibonacciHeapNode<MCCI_TIME_T, LookupSet> HeapNode;

    // holds the subscription information
    typedef map<MCCI_CLIENT_ID_T, HeapNode*> SubscriptionMap;

    // for iterating over subscriber information
    typedef typename SubscriptionMap::iterator SubscriptionMapIterator;
    
  private:
    unsigned int* m_outstanding_requests; // FIXME -- convert to vector
    unsigned int m_max_client_id;
    FibonacciHeap<MCCI_TIME_T, LookupSet> m_timeouts;


  public:
    RequestBank(unsigned int max_client_id)
    {
        this->m_outstanding_requests = new unsigned int[max_client_id]();
        this->m_max_client_id = max_client_id;
        //this->m_timeouts.m_debug_remove_min = true;
        //this->m_timeouts.m_debug = true;
    }
    
    virtual ~RequestBank() { delete[] this->m_outstanding_requests; }

    friend std::ostream& operator<<(ostream &out, RequestBank<KeySet> const &rhs)
    { return out << rhs.m_timeouts; }
    
    
    // developer tool to check sanity of a RequestBank
    //virtual bool check_sanity() const = 0;
    
    // add (OR UPDATE) an entry in the request bank
    void add(KeySet const key_set, MCCI_CLIENT_ID_T client_id, MCCI_TIME_T timeout)
    {
        if (client_id > this->m_max_client_id) throw string("Client ID too high");
         
        HeapNode* n = this->get_by_fq(key_set, client_id);

        // early exit for brand new nodes; just add them
        if (NULL == n)
        {
            LookupSet l;
            l.key_set = key_set;
            l.client_id = client_id;

            n = this->m_timeouts.insert(timeout, l);
            if (!n) throw string("Couldn't insert new node");

            this->add_by_fq(key_set, client_id, n);
            
            this->m_outstanding_requests[client_id] += 1; // add what wasn't there

            return;
        }
        
        // assert n->data().key_set == key_set
        // assert n->data().client_id == client_id
        
        this->m_timeouts.alter_key(n, timeout, 0);

        return;  // no action
    }

    // whether there are any requests
    bool empty() const { return this->m_timeouts.empty(); }

    // number of requests
    bool size() const { return this->m_timeouts.size(); }
    
    // get the timeout of the node that will expire first
    MCCI_TIME_T minimum_timeout() const { return this->m_timeouts.minimum()->key(); }

    // remove the request that's expiring first
    void remove_minimum()
    {
        LookupSet l = this->m_timeouts.minimum()->data();
        
        this->remove_by_fq(l.key_set, l.client_id);
        this->m_outstanding_requests[l.client_id] -= 1;
        this->m_timeouts.remove_minimum();
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

        MCCI_CLIENT_ID_T* operator->() const
        { return (MCCI_CLIENT_ID_T* const)&(SubscriptionMapIterator::operator->()->first); }
        
        MCCI_CLIENT_ID_T operator*() const
        { return SubscriptionMapIterator::operator*().first; }
    };

    // iteration points: begin
    subscriber_iterator subscribers_begin(KeySet const key_set) const
    {
        SubscriptionMap* sm = this->get_by_pq(key_set);
        if (sm) return sm->begin();
        return subscriber_iterator();
    }

    // iteration points: begin
    subscriber_iterator subscribers_end(KeySet const key_set) const
    {
        SubscriptionMap* sm = this->get_by_pq(key_set);
        if (sm) return sm->end();
        return subscriber_iterator();
   }
    
  protected:

    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) const = 0;

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const key_set) const = 0;
    
    // assume that this entry is unique and add it to the structure
    virtual void add_by_fq(KeySet const key_set,
                           MCCI_CLIENT_ID_T client_id,
                           HeapNode* const node_ptr) = 0;

    // remove a node from the custom container (not the heap) based on its key
    virtual void remove_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) = 0;

    // remove a partially-qualified set of nodes from the custom container (don't delete HeapNodes)
    virtual void remove_by_pq(KeySet const key_set) = 0;

};


////////////////////////////////////////////////////////////////////////////////





// class that stores requests using a single key into a linear hash
template<typename KeySet, typename Key>
    class RequestBankOneKey : public RequestBank<KeySet>
{

  public:
    typedef typename RequestBank<KeySet>::HeapNode HeapNode;
    typedef typename RequestBank<KeySet>::SubscriptionMap SubscriptionMap;
    typedef typename RequestBank<KeySet>::subscriber_iterator subscriber_iterator;
        
  protected:
    typedef LinearHash<Key, SubscriptionMap*> LinearHashBank;
    typedef typename LinearHashBank::iterator LinearHashBankIterator;
    
    LinearHashBank m_bank;

  public:
    
    virtual Key get_key(KeySet const key_set) const = 0; //{ return (Key)key_set; };
    
    RequestBankOneKey(unsigned int max_clients, unsigned int size)
      : RequestBank<KeySet>(max_clients)
    {
        this->m_bank.resize_nearest_prime(size);
    }
    
    virtual ~RequestBankOneKey()
    {
        // free all map objects that exist in LinearHashBank.
        LinearHashBankIterator it;
        for (it = this->m_bank.begin(); it != this->m_bank.end(); ++it)
            if (NULL != it->second)
                delete it->second;
    }

    // assume that this entry is unique and add it to the structure
    virtual void add_by_fq(KeySet const key_set,
                           MCCI_CLIENT_ID_T client_id,
                           HeapNode* const node_ptr)
    {
        Key k = this->get_key(key_set);

        // init hash entry if it doesn't exist
        this->m_bank.has_key(k);
        if (!this->m_bank.has_key(k))
        {
            this->m_bank[k] = new SubscriptionMap();
            if (NULL == this->m_bank[k]) throw string("Couldn't allocate new SubscriptionMap");
        }

        (*(this->m_bank[k]))[client_id] = node_ptr;  // add to map
    }
    
    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) const
    {
        Key k = this->get_key(key_set);

        if (!this->m_bank.has_key(k)) return NULL;

        if (NULL == this->m_bank[k])
        {
            stringstream s;
            s << "Improper cleanup is happening, key = " << k;
            throw string(s.str());
        }

        if (this->m_bank[k]->find(client_id) == this->m_bank[k]->end()) return NULL;

        return (*(this->m_bank[k]))[client_id];
    }

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const key_set) const
    {
        Key k = this->get_key(key_set);
        return this->m_bank.has_key(k) ? this->m_bank[k] : NULL;
    }

    // remove a node from the custom container (not the heap) based on its key
    virtual void remove_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id)
    {
        Key k = this->get_key(key_set);
        this->m_bank[k]->erase(client_id);

        // clean up if the subscriber map is empty
        if (this->m_bank[k]->empty()) this->remove_by_pq(key_set);
    }

    // remove a partially-qualified set of nodes from the custom container (don't delete HeapNodes)
    virtual void remove_by_pq(KeySet const key_set)
    {
        Key k = this->get_key(key_set);
        
        delete this->m_bank[k];
        this->m_bank[k] = NULL;
        this->m_bank.remove(k);
    }
};


////////////////////////////////////////////////////////////////////////////////


template<typename KeySet, typename Key1, typename Key2>
    class RequestBankTwoKeys : public RequestBank<KeySet>
{
  public:
    typedef typename RequestBank<KeySet>::HeapNode HeapNode;
    typedef typename RequestBank<KeySet>::SubscriptionMap SubscriptionMap;
    typedef typename RequestBank<KeySet>::subscriber_iterator subscriber_iterator;

  protected:
    typedef LinearHash<Key2, SubscriptionMap*> LinearHashKey2;
    typedef LinearHash<Key1, LinearHashKey2> LinearHashKey1;

    typedef typename LinearHashKey2::iterator LinearHashKey2Iterator;
    typedef typename LinearHashKey1::iterator LinearHashKey1Iterator;
    
    LinearHashKey1 m_bank;
    unsigned int m_size_key1;
    unsigned int m_size_key2;

  public:
    RequestBankTwoKeys(unsigned int max_clients, unsigned int num_key1, unsigned int num_key2)
        : RequestBank<KeySet>(max_clients)
    {
        this->m_size_key1 = num_key1;
        this->m_size_key2 = num_key2;
        this->m_bank.resize_nearest_prime(this->m_size_key1);
    }
    
    virtual Key1 get_key_1(KeySet const key_set) const = 0;
    virtual Key2 get_key_2(KeySet const key_set) const = 0;

    virtual ~RequestBankTwoKeys()
    {
        LinearHashKey1Iterator it1;
        LinearHashKey2Iterator it2;
        
        // free all map objects that exist in LinearHash.
        for (it1 = this->m_bank.begin(); it1 != this->m_bank.end(); ++it1)
            // TODO: it1->second == m_bank[it1->first] ?
            for (it2 = this->m_bank[it1->first].begin(); it2 != this->m_bank[it1->first].end(); ++it2)
                if (NULL != it2->second)
                    delete it2->second;
    }

    // assume that this entry is unique and add it to the structure
    virtual void add_by_fq(KeySet const key_set,
                           MCCI_CLIENT_ID_T client_id,
                           HeapNode* const node_ptr)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);

        // init hash entry if it doesn't exist
        if (!this->m_bank.has_key(k1)) this->m_bank[k1].resize_nearest_prime(this->m_size_key2);
        if (!this->m_bank[k1].has_key(k2)) this->m_bank[k1][k2] = new SubscriptionMap();
        if (NULL == this->m_bank[k1][k2]) throw string("Couldn't allocate new SubscriptionMap");
        (*(this->m_bank[k1][k2]))[client_id] = node_ptr;  // add to map
    }
    
    // return a pointer to a heap node based on the fully-qualified information, NULL if d.n.e.
    // (fully-qualified information means key set and client id)
    virtual HeapNode* get_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id) const 
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);
        if (!this->m_bank.has_key(k1)) return NULL;
        if (!this->m_bank[k1].has_key(k2)) return NULL;
        if (NULL == this->m_bank[k1][k2]) throw string("k1 and k2 point to NULL");
        if (this->m_bank[k1][k2]->find(client_id) == this->m_bank[k1][k2]->end()) return NULL;
        return (*(this->m_bank[k1][k2]))[client_id];
    }

    // return a pointer to a client_id -> heapnode map based on the partially-qualified info
    virtual SubscriptionMap* get_by_pq(KeySet const key_set) const
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);

        if (!this->m_bank.has_key(k1)) return NULL;
        if (!this->m_bank[k1].has_key(k2)) return NULL;
        return this->m_bank[k1][k2];
    }

    // remove a node from the custom container (not the heap) based on its key
    virtual void remove_by_fq(KeySet const key_set, MCCI_CLIENT_ID_T client_id)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);
        SubscriptionMap* m = this->m_bank[k1][k2];
        
        m->erase(client_id);
        
        if (m->empty())
        {
            delete m;
            this->m_bank[k1][k2] = NULL;
            this->m_bank[k1].remove(k2);

            if (this->m_bank[k1].empty())
            {
                this->m_bank.remove(k1);
            }
        }
    }

    // remove a partially-qualified set of nodes from the custom container (don't delete HeapNodes)
    virtual void remove_by_pq(KeySet const key_set)
    {
        Key1 k1 = this->get_key_1(key_set);
        Key2 k2 = this->get_key_2(key_set);

        delete this->m_bank[k1][k2];
        this->m_bank[k1][k2] = NULL;
    }
};




