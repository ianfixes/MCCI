
#include "FibonacciHeap.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;


void print_min(FibonacciHeap<uint, string>* h)
{
        cout << "min=";
        h->minimum()->print_node(cout);
        cout << endl;
}

void doTest()
{
    FibonacciHeap<uint, string> h;
    h.m_debug              = true;
    h.m_debug_remove_min   = false;
    h.m_debug_decrease_key = false;
        
    h.insert(4, "a");
    h.insert(2, "b");
    h.insert(7, "c");
    h.insert(5, "d");
    h.insert(1, "e");
    h.insert(8, "f");
    h.print_roots(cout);
        
    while (!h.empty()) 
    {
        print_min(&h);
        h.remove_minimum(); 
        h.print_roots(cout);
    }
        
    cout << endl << endl;
        
    vector <FibonacciHeapNode<uint, string>*> nodes(6);
    nodes[0] = h.insert(400, "a");
    nodes[1] = h.insert(200, "b");
    nodes[2] = h.insert(70, "c");
    nodes[3] = h.insert(50, "d");
    nodes[4] = h.insert(10, "e");  // minimum
    nodes[5] = h.insert(80, "f");
    h.print_roots(cout);
    print_min(&h);
        
    h.remove_minimum();
    print_min(&h);
    nodes[4]=NULL;   // minimum was removed
    h.print_roots(cout);
        
    for (uint i = 0; i < nodes.size(); ++i) 
    {
        if (!nodes[i]) // minimum - already removed
            continue;
        h.decrease_key(nodes[i], nodes[i]->key()/10);
        print_min(&h);
        h.print_roots(cout);
    }
        
    cout << endl << endl;
	
    h.insert(4, "AA");
    h.insert(2, "BB");
    h.insert(7, "CC");
    h.insert(5, "DD");
    h.insert(1, "EE");
    h.insert(8, "FF");
    h.print_roots(cout);
        
    while (!h.empty()) 
    {
        print_min(&h);
        h.remove_minimum(); 
        h.print_roots(cout);
    }
        
    cout << endl << endl;
}

int main() {
    try
    {
        doTest();
    } 
    catch (string s) 
    {
        cerr << endl << "ERROR: " << s << endl;
    }
}




