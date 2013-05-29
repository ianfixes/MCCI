
#include "MCCIRequestBank.h"
#include "MCCIRequestBanks.h"
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


typedef struct {short key1; long key2;} KeyPair;

KeyPair new_kp(short key1, long key2)
{
    KeyPair ret;
    ret.key1 = key1;
    ret.key2 = key2;
    return ret;
}


inline std::ostream& operator<<(std::ostream &out, KeyPair const &rhs)
{ return out << "(key1 " << rhs.key1 << ", key2 " << rhs.key2 << ")"; }
  

class Test2KeyRequestBank : public RequestBankTwoKeys<KeyPair, short, long>
{
  public:
    Test2KeyRequestBank(unsigned int max_clients, unsigned int size1, unsigned int size2) :
        RequestBankTwoKeys<KeyPair, short, long>(max_clients, size1, size2) { }

    virtual short get_key_1(KeyPair key_set) const
    {
        return key_set.key1;
    }

    virtual long get_key_2(KeyPair key_set) const
    {
        return key_set.key2;
    }
};


void test1()
{
        
    //DblStuffRequestBank b(1, 100, 100);

    TestRequestBank b(501, 100);

    printf("\nbrand-new Requestbank is empty? %d", b.empty());

    // key, client, timeout)
    for (int i=4; i < 30; i+=2)
    {
        printf("\nAdding %d", i);
        b.add(i, 400 + i, 4008 + i);
        printf("\nAdding %d", i - 2);
        b.add(i - 2, 400 + i - 2, 4008 + i - 2);
        printf("\nRemoving minimum");
        b.remove_minimum();
        printf("\nRemoving minimum");
        b.remove_minimum();
    }
    
    printf("\nRequestbank is empty after all that? %d", b.empty());


    b.add(8, 3, 3);
    printf("\nRequestbank with 1 entry is empty? %d", b.empty());

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


void test2()
{
    Test2KeyRequestBank bb(501, 10, 10);
    
    printf("\nbrand-new Requestbank is empty? %d", bb.empty());

    // key, client, timeout)
    for (int i=4; i < 30; i+=2)
    {
        printf("\nAdding %d", i);
        bb.add(new_kp(i, i * 100), 400 + i, 4008 + i);
        printf("\nAdding %d", i - 2);
        bb.add(new_kp(i - 2, (i - 2) * 100), 400 + i - 2, 4008 + i - 2);
        printf("\nRemoving minimum");
        bb.remove_minimum();
        printf("\nRemoving minimum");
        bb.remove_minimum();
    }
    
    printf("\nRequestbank is empty after all that? %d", bb.empty());

    bb.add(new_kp(8, 800), 3, 3);
    printf("\nRequestbank with 1 entry is empty? %d", bb.empty());


    bb.add(new_kp(8, 800), 500, 5008);
    bb.add(new_kp(6, 600), 500, 5006);
    bb.add(new_kp(7, 700), 500, 5007);
    bb.add(new_kp(5, 500), 500, 5005);
    bb.add(new_kp(3, 300), 500, 5003);
    bb.add(new_kp(9, 900), 500, 5009);


    printf("\nShould find 2 client entries for 8");
    Test2KeyRequestBank::subscriber_iterator it;
    for (it = bb.subscribers_begin(new_kp(8, 800)); it != bb.subscribers_end(new_kp(8, 800)); ++it)
    {
        printf("\n\t%d", *it);
    }


    printf("\nRequestbank contains key 2? %d", bb.contains(new_kp(2, 2))); // 0
    printf("\nRequestbank contains key 7? %d", bb.contains(new_kp(7, 700))); // 1
    printf("\nRequestbank contains key 7 for client 400? %d", bb.contains(new_kp(7, 700), 400)); // 0
    printf("\nRequestbank contains key 7 for client 500? %d", bb.contains(new_kp(7, 700), 500)); // 1

    
    while (!bb.empty())
    {
        printf("\nremoving...");
        bb.remove_minimum();
    }
    

}

void test3()
{

    unsigned int mxc = 100;
    
    printf("\nAll request bank");
    AllRequestBank              m_bank_all(100, 1);

    printf("\nHost request bank");
    HostRequestBank             m_bank_host(mxc, 20);

    printf("\nVariable request bank");
    VariableRequestBank         m_bank_var(mxc, 20);

    printf("\nHost-Variable request bank");
    HostVariableRequestBank     m_bank_hostvar(mxc, 30);

    printf("\nRemote Revision (Host-Variable/Revision) request bank");
    RemoteRevisionRequestBank   m_bank_remote(mxc, 20, 20);

    printf("\nVariable/Revision request bank");
    VariableRevisionRequestBank m_bank_varrev(mxc, 100, 20);
}

int main()
{
    try
    {
        test1();
        test2();
        test3();
    }
    catch (string s)
    {
        printf("\n\nERROR: %s\n", s.c_str());
    }
    
    return 0;

}
