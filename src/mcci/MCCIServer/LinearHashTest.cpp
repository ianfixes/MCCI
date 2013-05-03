
#include "LinearHash.h"
#include <string>
#include <stdio.h>

using namespace std;


void test_hash_size(unsigned int desired)
{
    LinearHash<int, int> lh;

    lh.resize_nearest_prime(desired);
    
    printf("\nnearest_prime(%d) = %d", desired, lh.get_size());
}

int main()
{
    LinearHash<unsigned long, string> lh(101);

    test_hash_size(0);
    test_hash_size(1);
    test_hash_size(2);
    test_hash_size(3);
    test_hash_size(4);
    test_hash_size(127);
    test_hash_size(128);
    test_hash_size(250);
    test_hash_size(251);
    test_hash_size(8192);
    test_hash_size(16380);
    test_hash_size(16384);
    //test_hash_size();

    printf("\n\nAdding 3000, 1000000, and 37");
    lh.insert(3000, "three thousand");
    lh.insert(1000000, "one million");
    lh[37] = "thirty seven";

    printf("\nExpect 3 elements in hash, count() = %d", lh.count());

    printf("\n3001 in hash? %d", lh.has_key(3001));
    printf("\n3000 in hash? %d", lh.has_key(3000));
    printf("\nValue of 3000 is %s", lh[3000].c_str());

    lh.remove(3000);
    printf("\nRemoved 3000.  3000 in hash? %d", lh.has_key(3000));
    printf("\nhash[3000] = %s", lh[3000].c_str());
    
    printf("\n");

    printf("\n37 = %s", lh[37].c_str());
    lh[37] = "I'm not old";
    printf("\n37 = %s", lh[37].c_str());

    printf("\nExpect 2 elements in hash, count() = %d", lh.count());
    
    // step sizes 1, 11, 21 ... 101
    // demonstrating that we only get collisions when step size = hash table size
    for (int step = 0; step < 6; ++step)
    {
        printf("\n\nClearing, to insert 303 elements, step size = %d", (step * 20 + 1));
        lh.clear();
        
        for (int i = 0; i < 303; ++i)
            lh.insert((step * 20 + 1) * i, "hello");
        
        printf("\nCount is now %d, max_collisions is %d", lh.count(), lh.max_collisions());
    }

    printf("\n\nTesting resize with no segfaults... ");
    lh.resize(7);
    printf("seems OK.");


    LinearHash<unsigned int, unsigned int> ilh(2);

    printf("\n\nInt value of uninitialized lookup key 42 is %d (we expect 0)", ilh[42]);

    
    printf("\n\n");
    
    return 0;
}
