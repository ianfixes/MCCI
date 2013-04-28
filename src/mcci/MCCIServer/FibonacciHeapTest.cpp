
#include "FibonacciHeap.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;


void print_min(FibonacciHeap<string, uint>* h)
{
        cout << "min=";
        h->minimum()->print_node(cout);
        cout << endl;
}

void doTest()
{
    FibonacciHeap<string, uint> h;
    h.m_debug              = true;
    h.m_debug_remove_min   = false;
    h.m_debug_decrease_key = false;
        
    h.insert("a",4);
    h.insert("b",2);
    h.insert("c",7);
    h.insert("d",5);
    h.insert("e",1);
    h.insert("f",8);
    h.print_roots(cout);
        
    while (!h.empty()) 
    {
        print_min(&h);
        h.remove_minimum(); 
        h.print_roots(cout);
    }
        
    cout << endl << endl;
        
    vector <FibonacciHeapNode<string, uint>*> nodes(6);
    nodes[0] = 
        h.insert("a",400);
    nodes[1] = 
        h.insert("b",200);
    nodes[2] = 
        h.insert("c",70);
    nodes[3] = 
        h.insert("d",50);
    nodes[4] = 
        h.insert("e",10);
    nodes[5] = 
        h.insert("f",80);
    h.print_roots(cout);
    print_min(&h);
        
    h.remove_minimum();
    print_min(&h);
    nodes[4]=NULL;
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
	
    h.insert("AA",4);
    h.insert("BB",2);
    h.insert("CC",7);
    h.insert("DD",5);
    h.insert("EE",1);
    h.insert("FF",8);
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




