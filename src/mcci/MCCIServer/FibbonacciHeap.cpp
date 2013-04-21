/**
 * template Fibonacci Heap 
 *
 * @ref http://en.wikipedia.org/wiki/Fibonacci_heap
 * @ref http://www.cse.yorku.ca/~aaw/Jason/FibonacciHeapAlgorithm.html
 * @author Erel Segal http://tora.us.fm/rentabrain
 * @date 2010-11-11
 */

#include <iostream>
#include <vector>
using namespace std;

typedef unsigned int uint;


/**
 * The heap is a min-heap sorted by Key.
 */
template <typename Data, typename Key> class FibonacciHeapNode
{
    Key m_key;
    Data m_data;
    
    uint m_degree; // number of childern. used in the removeMinimum algorithm.
    bool m_mark;   // mark used in the decreaseKey algorithm.
    
    //uint m_count; // total number of elements in tree, including this. For debug only
    
    
    FibonacciHeapNode<Data,Key>* m_previous;  // pointers in a circular doubly linked list
    FibonacciHeapNode<Data,Key>* m_next;
    FibonacciHeapNode<Data,Key>* m_child; // pointer to the first child in the list of children
    FibonacciHeapNode<Data,Key>* m_parent;
    
    FibonacciHeapNode() {};
    
    FibonacciHeapNode(Data d, Key k)
    {
        
        m_key      = k;
        m_data     = d;
        m_degree   = 0;
        m_mark     = false;
        m_child    = NULL;
        m_parent   = NULL;
        m_previous = m_next = this; // doubly linked circular list
    } 
	
    bool isSingle() const
    {
        return (this == this->m_next);
    }
        
    // inserts a new node after this node
    void insert(FibonacciHeapNode<Data,Key>* other) 
    {
        if (!other) return;
            
        // For example: given 1->2->3->4->1, insert a->b->c->d->a after node 3:
        //	result: 1->2->3->a->b->c->d->4->1
            
        this->m_next->m_previous = other->m_previous;
        other->m_previous->m_next = this->m_next;
            
        this->m_next = other;
        other->m_previous = this;
    }
        
    void remove() 
    {
        this->m_previous->m_next = this->m_next;
        this->m_next->m_previous = this->m_previous;
        this->m_next = this->m_previous = this;
    }
	
    void addChild(FibonacciHeapNode<Data,Key>* other) 
    { // Fibonacci-Heap-Link(other,current)
        if (!m_child)
            m_child = other;
        else
            m_child->insert(other);
        other->m_parent = this;
        other->m_mark = false;
        m_degree++;
        //m_count += other->m_count;
    }

    void removeChild(FibonacciHeapNode<Data,Key>* other) 
    {
        if (other->m_parent != this)
            throw string ("Trying to remove a child from a non-parent");

        if (other->isSingle()) 
        {
            if (m_child != other)
                throw string ("Trying to remove a non-child");
            m_child = NULL;
        } 
        else 
        {
            if (m_child == other)
                m_child = other->m_next;
            other->remove(); // from list of children
        }
        other->m_parent = NULL;
        other->m_mark = false;
        m_degree--;
        //m_count -= other->m_count;
    }

	
    friend ostream& operator<< (ostream& out, const FibonacciHeapNode& n) 
    {
        return (out << n.m_data << ":" << n.m_key);
    }
	
    void printTree(ostream& out) const 
    {
        out << m_data << ":" << m_key << ":" << m_degree << ":" << m_mark;
        if (m_child) 
        {
            out << "(";
            const FibonacciHeapNode<Data,Key>* n = m_child;

            do 
            {
                if (n == this)
                    throw string("Illegal pointer - node is child of itself");
                n->printTree(out); 
                out << " ";
                n = n->m_next;
            } 
            while (n != m_child);
            out << ")";
        }
    }


    void printAll(ostream& out) const 
    {
        const FibonacciHeapNode<Data,Key>* n = this;
        do 
        {
            n->printTree(out); 
            out << " ";
            n = n->m_next;
        } 
        while (n != this);

        out << endl;
    }
	
public:
    Key key() const { return m_key; }
    Data data() const { return m_data; }
	
    template <typename D, typename K> friend class FibonacciHeap;
}; // FibonacciHeapNode



template <typename Data, typename Key> class FibonacciHeap 
{
    typedef FibonacciHeapNode<Data,Key>* PNode;
    PNode m_rootWithMinKey; // a circular d-list of nodes
    uint m_count;      // total number of elements in heap
    uint m_maxDegree;  // maximum degree (=child count) of a root in the  circular d-list
    
protected:
    PNode insertNode(PNode newNode) 
    {
        //if (m_debug) cout << "insert " << (*newNode) << endl;
        if (!m_rootWithMinKey) 
        { 
            // insert the first m_key to the heap:
            m_rootWithMinKey = newNode;
        } 
        else 
        {
            m_rootWithMinKey->insert(newNode);  // insert the root of new tree to the list of roots
            if (newNode->key() < m_rootWithMinKey->key())
                m_rootWithMinKey = newNode;
        }
        return newNode;
    }

public:
    bool m_debug, m_debugRemoveMin, m_debugDecreaseKey;
    

    FibonacciHeap()
    {
        m_rootWithMinKey   = NULL;
        m_count            = 0;
        m_maxDegree        = 0;
        m_debug            = false;
        m_debugRemoveMin   = false;
        m_debugDecreaseKey = false;
    }
    
    ~FibonacciHeap() { /* TODO: remove all nodes */ }

    bool empty() const {return m_count == 0;}

    PNode minimum() const 
    { 
        if (!m_rootWithMinKey)
            throw string("no minimum element");
        return m_rootWithMinKey;
    }

    void printRoots(ostream& out) const 
    {
        out << "m_maxDegree=" << m_maxDegree << "  m_count=" << m_count << "  roots=";
        if (m_rootWithMinKey)
            m_rootWithMinKey->printAll(out);
        else
            out << endl;
    }

    void merge (const FibonacciHeap& other) 
    {  // Fibonacci-Heap-Union
        m_rootWithMinKey->insert(other.m_rootWithMinKey);
        if (!m_rootWithMinKey || 
            (other.m_rootWithMinKey &&
             other.m_rootWithMinKey->key() < m_rootWithMinKey->key()))
            this->m_rootWithMinKey = other.m_rootWithMinKey;
        m_count += other.m_count;
    }
	
    PNode insert (Data d, Key k) 
    {
        if (m_debug) cout << "insert " << d << ":" << k << endl;
        m_count++;
        // create a new tree with a single m_key:
        return insertNode(new FibonacciHeapNode<Data,Key>(d,k));
    }


    void removeMinimum() 
    {  // Fibonacci-Heap-Extract-Min, CONSOLIDATE
        if (!m_rootWithMinKey)
            throw string("trying to remove from an empty heap");

        if (m_debug) cout << "removeMinimum" << endl;
        m_count--;

        /// Phase 1: Make all the removed root's children new roots:
        // Make all children of root new roots:
        if (m_rootWithMinKey->m_child) 
        {
            if (m_debugRemoveMin) 
            {
                cout << "  root's children: "; 
                m_rootWithMinKey->m_child->printAll(cout);
            }
            PNode c = m_rootWithMinKey->m_child;
            do {
                c->m_parent = NULL;
                c = c->m_next;
            } while (c != m_rootWithMinKey->m_child);
            m_rootWithMinKey->m_child = NULL; // removed all children
            m_rootWithMinKey->insert(c);
        }

        if (m_debugRemoveMin) 
        {
            cout << "  roots after inserting children: "; 
            printRoots(cout);
        }
		

        /// Phase 2-a: handle the case where we delete the last m_key:
        if (m_rootWithMinKey->m_next == m_rootWithMinKey) 
        {
            if (m_debugRemoveMin) cout << "  removed the last" << endl;
            if (m_count != 0)
                throw string ("Internal error: should have 0 keys");
            m_rootWithMinKey = NULL;
            return;
        }

        /// Phase 2: merge roots with the same degree:
        vector<PNode> degreeRoots (m_maxDegree + 1); // make room for a new degree
        fill (degreeRoots.begin(), degreeRoots.end(), (PNode)NULL);
        m_maxDegree = 0;
        PNode currentPointer = m_rootWithMinKey->m_next;
        uint currentDegree;
        do 
        {
            currentDegree = currentPointer->m_degree;
            if (m_debugRemoveMin) 
            {
                cout << "  roots starting from currentPointer: "; 
                currentPointer->printAll(cout);
                cout << "  checking root " 
                     << *currentPointer 
                     << " with degree " 
                     << currentDegree << endl;
            }

            PNode current = currentPointer;
            currentPointer = currentPointer->m_next;
            while (degreeRoots[currentDegree]) 
            { // merge the two roots with the same degree:
                PNode other = degreeRoots[currentDegree]; // another root with the same degree
                if (current->key() > other->key())
                    swap(other,current); 
                // now current->key() <= other->key() - make other a child of current:
                other->remove(); // remove from list of roots
                current->addChild(other);
                if (m_debugRemoveMin) cout << "  added " << *other << " as child of " << *current << endl;
                degreeRoots[currentDegree]=NULL;
                currentDegree++;
                if (currentDegree >= degreeRoots.size())
                    degreeRoots.push_back((PNode)NULL);
            }
            // keep the current root as the first of its degree in the degrees array:
            degreeRoots[currentDegree] = current;

        } 
        while (currentPointer != m_rootWithMinKey);

        /// Phase 3: remove the current root, and calcualte the new m_rootWithMinKey:
        delete m_rootWithMinKey;
        m_rootWithMinKey = NULL;

        uint newMaxDegree=0;
        for (uint d=0; d<degreeRoots.size(); ++d) 
        {
            if (m_debugRemoveMin) cout << "  degree " << d << ": ";
            if (degreeRoots[d]) 
            {
                if (m_debugRemoveMin) cout << " " << *degreeRoots[d] << endl;
                degreeRoots[d]->m_next = degreeRoots[d]->m_previous = degreeRoots[d];
                insertNode(degreeRoots[d]);
                if (d>newMaxDegree)
                    newMaxDegree = d;		
            } 
            else 
            {
                if (m_debugRemoveMin) cout << "  no node" << endl;
            }
        }
        m_maxDegree = newMaxDegree;
    }
	
    void decreaseKey(PNode node, Key newKey) 
    {
        if (newKey >= node->m_key)
            throw string("Trying to decrease key to a greater key");

        if (m_debug) cout << "decrease key of " << *node << " to " << newKey << endl;
        // Update the key and possibly the min key:
        node->m_key = newKey;

        // Check if the new key violates the heap invariant:
        PNode parent = node->m_parent;
        if (!parent) 
        { // root node - just make sure the minimum is correct
            if (newKey < m_rootWithMinKey->key())
                m_rootWithMinKey = node;
            return; // heap invariant not violated - nothing more to do
        } 
        else if (parent->key() <= newKey) 
        {
            return; // heap invariant not violated - nothing more to do
        }

        for(;;) 
        {
            parent->removeChild(node);
            insertNode(node);
            if (m_debugDecreaseKey) 
            {
                cout << "  removed " << *node << " as child of " << *parent << endl;
                cout << "  roots after remove: "; 
                m_rootWithMinKey->printAll(cout);
            }

            if (!parent->m_parent) 
            { // parent is a root - nothing more to do
                break;
            } 
            else if (!parent->m_mark) 
            {  // parent is not a root and is not marked - just mark it
                parent->m_mark = true;
                break;
            } 
            else 
            {
                node = parent;
                parent = parent->m_parent;
                continue;
            }
        };
    }

    void remove(PNode node, Key minusInfinity) 
    {
        if (minusInfinity >= minimum()->key())
            throw string("2nd argument to remove must be a key that is smaller than all other keys");
        decreaseKey(node, minusInfinity);
        removeMinimum();
    }

};  // FibonacciHeap


