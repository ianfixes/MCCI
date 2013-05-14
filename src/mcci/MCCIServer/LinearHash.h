
#pragma once

#include <string>
#include <map>
#include <stdio.h>

using namespace std;


/* A table of prime numbers, each less than a power of 2 */
static unsigned int LINEAR_HASH_TABLE_PRIMES[] = {
    1,
    2,
    4 - 1,
    8 - 1,
    16 - 3,
    32 - 1,
    64 - 3,
    128 - 1,
    256 - 5,
    512 - 3,
    1024 - 3,
    2048 - 9,
    4096 - 3,
    8192 - 1,
    16384 - 3,
    32768 - 19,
    65536 - 15,
};




/**
   A simple hash table that uses f(i) = i for the hash function (so does Sun's hash table implementation)

   This hash table is designed for speed and for integer keys (key should be some variant of int/long/etc)

   It is assumed that contiguous blocks of integer keys will be hashed.
   
 */
template <typename Key, typename Data> class LinearHash
{

  public:
    // to deal with collisions, we use a map (implemented as a RB tree, usually)
    typedef map<Key, Data> Container;
    typedef typename Container::iterator ContainerIterator;

    
  protected:
    
    // internal storage is an array of trees
    Container* m_container;

    // the number of trees in our hash
    unsigned int m_size;

    
  public:

    LinearHash()
    {
        this->m_size = 0;
        this->resize(1);
    };
    

    LinearHash(unsigned int size)
    {
        this->m_size = 0;
        this->resize(size);
    };

    
    ~LinearHash()
    {
        delete[] this->m_container; // TODO: check proper delete syntax
    };


    //resize, destructively, to exact size
    void resize(unsigned int size)
    {
        if (!size)
        {
            throw string("Tried to set hash size to 0");
        }
        
        if (this->m_size) delete[] this->m_container;

        this->m_size = size;
        this->m_container = new Container[this->m_size]();
        
    }


    // resize to a prime number size according to desired storage
    void resize_nearest_prime(unsigned int desired_size)
    {
        unsigned int i;
        if (desired_size < 2)
        {
            this->resize(1);
        }
        else
        {
            for (i = 1; i < 16 && LINEAR_HASH_TABLE_PRIMES[i] <= desired_size; ++i);
        
            this->resize(LINEAR_HASH_TABLE_PRIMES[i - 1]);
        }
    }
    
    
    // return the size of the hash table
    unsigned int get_size() const
    {
        return this->m_size;
    }


    // return the number of elements in the hash table
    unsigned int count() const
    {
        unsigned int sum = 0;

        // TODO: track it differently so its O(1)
        for (unsigned int i = 0; i < this->m_size; ++i)
            sum += this->m_container[i].size();

        return sum;
    }


    // return whether the table is empty
    bool empty() const
    {
        return 0 == this->count();
    }
    
    // get the maximum key collisions on any given bucket
    unsigned int max_collisions() const
    {
        unsigned int max = 0;

        for (unsigned int i = 0; i < this->m_size; ++i)
            if (max < this->m_container[i].size())
                max = this->m_container[i].size();

        return max;
    }

    
    // explicitly insert an element into the hash
    void insert(Key k, Data d)
    {
        this->m_container[k % this->m_size][k] = d;
    }

    
    // explicitly remove a key from the hash
    void remove(Key k)
    {
        this->m_container[k % this->m_size].erase(k);
    }

    
    // check existence of a hashed value
    bool has_key(Key k) const
    {
        unsigned int idx = k % this->m_size;
        return this->m_container[idx].end() != this->m_container[idx].find(k);
    }

    
    // array-style access to the hash
    Data& operator[] (Key k)
    {
        return this->m_container[k % this->m_size][k];
    }


    // remove all elements from the hash
    void clear()
    {
        for (int i = 0; i < this->m_size; ++i)
            this->m_container[i].clear();
    }


    class iterator : public std::iterator<std::input_iterator_tag, pair<Key, Data> >
    {
        LinearHash<Key, Data>* h;
        unsigned int idx;
        ContainerIterator ci;

      public:

        iterator() {};
        
        iterator(LinearHash<Key, Data>* h, unsigned int idx, ContainerIterator ci)
        {
            this->h = h;
            this->idx = idx;
            this->ci = ci;
        }

        iterator(const iterator& it)
        {
            this->operator=(it);
        }

        iterator& operator=(const iterator &rhs)
        {
            this->h = rhs.h;
            this->idx = rhs.idx;
            this->ci = rhs.ci;

            return *this;
        }
        
        iterator& operator++()
        {
            // increment individual tree pointer, or jump to next un-vacant tree
            if (this->h->m_container[this->idx].end() != ++(this->ci)) return *this;

            for (++(this->idx); this->idx < this->h->m_size; ++(this->idx))
            {
                //fprintf(stderr, "\ntrying tree %d of %d", this->idx, this->h->m_size - 1);
                if (!this->h->m_container[this->idx].empty())
                {
                    this->ci = this->h->m_container[this->idx].begin();
                    return *this;
                }
            }

            // point to end if we don't find anything
            this->ci = this->h->m_container[this->h->m_size - 1].end();
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            this->operator++();
            return tmp;
        }

        bool operator==(const iterator& rhs)
        {
            return rhs.h == this->h && rhs.idx == this->idx && rhs.ci == this->ci;
        }

        bool operator!=(const iterator& rhs)
        {
            return rhs.h != this->h || rhs.idx != this->idx || (!(rhs.ci == this->ci));
        }

        pair<const Key, Data> operator*() { return *(this->ci); }

        pair<const Key, Data>* operator->() { return &(*(this->ci)); }

        
    };

    // iteration points: begin
    iterator begin()
    {
        for (unsigned int i = 0; i < this->m_size; ++i)
            if (!this->m_container[i].empty())
                return iterator(this, i, this->m_container[i].begin());

        return this->end();
    }

    // iteration points: end
    iterator end()
    {
        unsigned int last = this->m_size - 1;
        return iterator(this, last + 1, this->m_container[last].end());
    }

};
