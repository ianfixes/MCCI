
#include "LinearHash.h"
#include <string>
#include <stdio.h>

using namespace std;


typedef int footype;
typedef int bartype;

int main()
{
    LinearHash<unsigned long, string> lh(101);


    footype f = 3;
    bartype b;

    b = f;
    
    lh.insert(3000, "three thousand");
    lh.insert(1000000, "one million");
    lh[37] = "thirty seven";

    printf("\nExpect 3 elements in hash, count() = %d", lh.count());

    printf("\n3001 in hash? %d", lh.has_key(3001));
    printf("\n3000 in hash? %d", lh.has_key(3000));
    printf("\nValue of 3000 is %d", lh[3000].c_str());

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
    for (int step = 0; step < 12; ++step)
    {
        printf("\n\nClearing, to insert 303 elements, step size = %d", (step * 10 + 1));
        lh.clear();
        
        for (int i = 0; i < 303; ++i)
            lh.insert((step * 10 + 1) * i, "hello");
        
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
