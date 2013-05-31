
#include "LinearHash.h"
#include <string>
#include <stdio.h>
#include <boost/cstdint.hpp>

using namespace std;


void try_hash_resize(unsigned int desired)
{
    LinearHash<int, int> lh;

    lh.resize_nearest_prime(desired);
    
    printf("\nnearest_prime(%d) = %d", desired, lh.get_size());
}

void test_hash_sizes()
{

    try_hash_resize(0);
    try_hash_resize(1);
    try_hash_resize(2);
    try_hash_resize(3);
    try_hash_resize(4);
    try_hash_resize(127);
    try_hash_resize(128);
    try_hash_resize(250);
    try_hash_resize(251);
    try_hash_resize(8192);
    try_hash_resize(16380);
    try_hash_resize(16384);
    //try_hash_resize();

}

void test_hash_operations()
{
    typedef LinearHash<unsigned long, string> NumberHash;
    NumberHash lh(101);

    printf("\n\nAdding 3000, 1000000, and 37");
    lh.insert(3000, "three thousand");
    lh.insert(1000000, "one million");
    lh[37] = "thirty seven";

    printf("\nExpect 3 elements in hash, count() = %d", lh.count());

    NumberHash::iterator it;
    printf("\nIterating:");
    for (it = lh.begin(); it != lh.end(); ++it)
    {
        printf("\n    %ld\t: '%s'", it->first, it->second.c_str());
    }

    printf("\n3001 in hash? %d", lh.has_key(3001));
    printf("\n3000 in hash? %d", lh.has_key(3000));
    printf("\nValue of 3000 is %s", lh[3000].c_str());

    lh.remove(3000);
    printf("\nRemoved 3000.  3000 in hash? %d", lh.has_key(3000));
    printf("\nhash[3000] = %s", lh[3000].c_str());
    printf("\nauto-added null 3000 after access? %d", lh.has_key(3000));
    if (lh.has_key(3000)) lh.remove(3000);

    
    printf("\n");

    printf("\n37 = %s", lh[37].c_str());
    lh[37] = "I'm not old";
    printf("\n37 = %s", lh[37].c_str());

    printf("\nExpect 2 elements in hash, count() = %d", lh.count());
    
    // step sizes 1, 11, 21 ... 101
    // demonstrating that we only get collisions when step size = hash table size
    for (int step = 0; step < 6; ++step)
    {
        int inc = step * 20 + 1;
        printf("\n\nClearing, to insert 303 elements, step size = %d", inc);
        lh.clear();

        for (int i = 0; i < 303; ++i)
            lh.insert(inc * i, "hello");

        printf("\nContents: ");
        for (LinearHash<unsigned long, string>::iterator it = lh.begin(); it != lh.end(); ++it)
            printf(" %ld", it->first);
              
        
        printf("\nCount is now %d, max_collisions is %d", lh.count(), lh.max_collisions());
    }

    printf("\n\nTesting resize with no segfaults... ");
    lh.resize(7);
    printf("seems OK.");


    LinearHash<unsigned int, unsigned int> ilh(2);

    printf("\n\nInt value of uninitialized lookup key 42 is %d (we expect 0)", ilh[42]);

}


void test_multidim_hash()
{

    typedef LinearHash<short, string> ShortLinearHash;
    typedef LinearHash<long, ShortLinearHash> LongShortLinearHash;

    typedef typename ShortLinearHash::iterator ShortLinearHashIterator;
    typedef typename LongShortLinearHash::iterator LongShortLinearHashIterator;

    LongShortLinearHash ls(5);
    
    for (short s = 10; s < 20; ++s)
    {
        for (long l = 0; l < 1000; l += 100)
        {
            char buf[100];
            sprintf(buf, "(%ld, %d)", l, s);

            ls[l][s] = buf;
        }
    }

    // now print
    for (short s = 10; s < 20; ++s)
    {
        for (long l = 0; l < 1000; l += 100)
        {
            printf("\nLocation ls[%ld][%d] = %s", l, s, ls[l][s].c_str());
        }
    }
}


void test_short_hash()
{
    LinearHash<uint16_t, unsigned int> m_ordinality; // ordinality

    m_ordinality.resize_nearest_prime(2);
    
    uint16_t var_id = 1;
    unsigned int i = 0;
    
    m_ordinality[var_id] = i;

    var_id = 2;
    i = 1;

    m_ordinality[var_id] = i;
}

int main()
{
    
    test_hash_sizes();
    test_hash_operations();
    test_multidim_hash();
    test_short_hash();
    
    printf("\n\n");
    
    return 0;
}
