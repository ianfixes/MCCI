
#include "MCCIRequestBank.h"
#include <string>
#include <stdio.h>

using namespace std;


class TestRequestBank : public RequestBankOneKey<int, int>
{
  public:
    TestRequestBank(unsigned int max_clients, unsigned int size) :
    RequestBankOneKey<int, int>(max_clients, size) { }

    virtual int get_key(int const key_set) const { return key_set; }
};


void mybr()
{
    return;
}


void test1()
{
        
    //DblStuffRequestBank b(1, 100, 100);

    TestRequestBank b(5, 100);

    printf("\nbrand-new Requestbank is empty? %d", b.empty());

    // key, client, timeout)
    for (int i=4; i < 30; i+=2)
    {
        printf("Adding %d", i);
        b.add(i, 400 + i, 4008 + i);
        printf("Adding %d", i - 2);
        b.add(i - 2, 400 + i - 2, 4008 + i - 2);
        printf("Removing minimum");
        b.remove_minimum();
        printf("Removing minimum");
        b.remove_minimum();
    }
    
    printf("\nRequestbank is empty after all that? %d", b.empty());


    b.add(8, 3, 3);
    printf("\nRequestbank with 1 entry is empty? %d", b.empty());

    b.remove_minimum();
    printf("\nRequestBank after removing the only entry is empty? %d", b.empty());

    
    
    b.add(8, 500, 5008);
    b.add(6, 500, 5006);
    b.add(7, 500, 5007);
    b.add(5, 500, 5005);
    b.add(3, 500, 5003);
    b.add(9, 500, 5009);


    

    printf("\nShould find 2 client entries for 8");
    TestRequestBank::subscriber_iterator it;
    for (it = b.subscribers_begin(8); it != b.subscribers_end(8); ++it)
    {
        printf("\n\t%d", *it);
    }

    printf("\nRequestbank contains key 2? %d", b.contains(2)); // 0
    printf("\nRequestbank contains key 7? %d", b.contains(7)); // 1
    printf("\nRequestbank contains key 7 for client 400? %d", b.contains(7, 400)); // 0
    printf("\nRequestbank contains key 7 for client 500? %d", b.contains(7, 500)); // 1

    while (!b.empty())
    {
        fprintf(stderr, "\nremoving...");
        b.remove_minimum();
    }
    
    
}

int main()
{
    try
    {
        test1();
    }
    catch (string s)
    {
        printf("\n\nERROR: %s\n", s.c_str());
    }
    
    return 0;

}
