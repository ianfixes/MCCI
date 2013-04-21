
#include "FibbonacciHeap.cpp"

int main() {
    try {
        FibonacciHeap<string, uint> h;
        h.m_debug            = true;
        h.m_debugRemoveMin   = false;
        h.m_debugDecreaseKey = false;
        
        h.insert("a",4);
        h.insert("b",2);
        h.insert("c",7);
        h.insert("d",5);
        h.insert("e",1);
        h.insert("f",8);
        h.printRoots(cout);
        
        while (!h.empty()) 
        {
            cout << "min=" << *h.minimum() << endl;
            h.removeMinimum(); 
            h.printRoots(cout);
        }
        
        cout << endl << endl;
        
        vector <FibonacciHeapNode<string,uint>*> nodes(6);
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
        h.printRoots(cout);
        cout << "min=" << *h.minimum() << endl;
        
        h.removeMinimum(); 
        cout << "min=" << *h.minimum() << endl;
        nodes[4]=NULL;
        h.printRoots(cout);
        
        for (uint i = 0; i < nodes.size(); ++i) 
        {
            if (!nodes[i]) // minimum - already removed
                continue;
            h.decreaseKey(nodes[i], nodes[i]->key()/10);
            cout << "min=" << *h.minimum() << endl;
            h.printRoots(cout);
        }
        
        cout << endl << endl;
	
        h.insert("AA",4);
        h.insert("BB",2);
        h.insert("CC",7);
        h.insert("DD",5);
        h.insert("EE",1);
        h.insert("FF",8);
        h.printRoots(cout);
        
        while (!h.empty()) 
        {
            cout << "min=" << *h.minimum() << endl;
            h.removeMinimum(); 
            h.printRoots(cout);
        }
        
        cout << endl << endl;
        
    } 
    catch (string s) 
    {
        cerr << endl << "ERROR: " << s << endl;
    }
}




