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

	uint degree; // number of childern. used in the removeMinimum algorithm.
	bool mark;   // mark used in the decreaseKey algorithm.

	//uint count; // total number of elements in tree, including this. For debug only

    
    
	FibonacciHeapNode<Data,Key>* previous;  // pointers in a circular doubly linked list
	FibonacciHeapNode<Data,Key>* next;
	FibonacciHeapNode<Data,Key>* child; // pointer to the first child in the list of children
	FibonacciHeapNode<Data,Key>* parent;

	FibonacciHeapNode() {};
    
	FibonacciHeapNode(Data d, Key k)
        {
            
            m_key(k), 
                m_data(d),
                degree(0),
                mark(false),
                child(NULL), 
                parent(NULL)
                //,count(1)
                {
                    previous = next = this; // doubly linked circular list
                };
    
	
            bool isSingle() const
        {
            return (this == this->next);
        }

            // inserts a new node after this node
            void insert(FibonacciHeapNode<Data,Key>* other) {
                if (!other)
                    return;

                // For example: given 1->2->3->4->1, insert a->b->c->d->a after node 3:
                //	result: 1->2->3->a->b->c->d->4->1

                this->next->previous = other->previous;
                other->previous->next = this->next;

                this->next = other;
                other->previous = this;
            }

            void remove() {
                this->previous->next = this->next;
                this->next->previous = this->previous;
                this->next = this->previous = this;
            }
	
            void addChild(FibonacciHeapNode<Data,Key>* other) { // Fibonacci-Heap-Link(other,current)
                if (!child)
                    child = other;
                else
                    child->insert(other);
                other->parent = this;
                other->mark = false;
                degree++;
                //count += other->count;
            }

            void removeChild(FibonacciHeapNode<Data,Key>* other) {
                if (other->parent!=this)
                    throw string ("Trying to remove a child from a non-parent");
                if (other->isSingle()) {
                    if (child != other)
                        throw string ("Trying to remove a non-child");
                    child = NULL;
                } else {
                    if (child == other)
                        child = other->next;
                    other->remove(); // from list of children
                }
                other->parent=NULL;
                other->mark = false;
                degree--;
                //count -= other->count;
            }

	
            friend ostream& operator<< (ostream& out, const FibonacciHeapNode& n) {
                return (out << n.m_data << ":" << n.m_key);
            }
	
            void printTree(ostream& out) const {
                out << m_data << ":" << m_key << ":" << degree << ":" << mark;
                if (child) {
                    out << "(";
                    const FibonacciHeapNode<Data,Key>* n=child;
                    do {
                        if (n==this)
                            throw string("Illegal pointer - node is child of itself");
                        n->printTree(out); 
                        out << " ";
                        n = n->next;
                    } while (n!=child);
                    out << ")";
                }
            }

            void printAll(ostream& out) const {
                const FibonacciHeapNode<Data,Key>* n=this;
                do {
                    n->printTree(out); 
                    out << " ";
                    n = n->next;
                } while (n!=this);
                out << endl;
            }
	
          public:
            Key key() const { return m_key; }
            Data data() const { return m_data; }
	
            template <typename D, typename K> friend class FibonacciHeap;
        }; // FibonacciHeapNode



    template <typename Data, typename Key> class FibonacciHeap {
        typedef FibonacciHeapNode<Data,Key>* PNode;
        PNode rootWithMinKey; // a circular d-list of nodes
        uint count;      // total number of elements in heap
        uint maxDegree;  // maximum degree (=child count) of a root in the  circular d-list

    protected:
        PNode insertNode(PNode newNode) {
            //if (debug) cout << "insert " << (*newNode) << endl;
            if (!rootWithMinKey) { // insert the first m_key to the heap:
                rootWithMinKey = newNode;
            } else {
                rootWithMinKey->insert(newNode);  // insert the root of new tree to the list of roots
                if (newNode->key() < rootWithMinKey->key())
                    rootWithMinKey = newNode;
            }
            return newNode;
        }

    public:
        bool debug, debugRemoveMin, debugDecreaseKey;

        FibonacciHeap(): 
            rootWithMinKey(NULL), count(0), maxDegree(0), debug(false), debugRemoveMin(false) {}

        ~FibonacciHeap() { /* TODO: remove all nodes */ }

        bool empty() const {return count==0;}

        PNode minimum() const { 
            if (!rootWithMinKey)
                throw string("no minimum element");
            return rootWithMinKey;
        }

        void printRoots(ostream& out) const {
            out << "maxDegree=" << maxDegree << "  count=" << count << "  roots=";
            if (rootWithMinKey)
                rootWithMinKey->printAll(out);
            else
                out << endl;
        }

        void merge (const FibonacciHeap& other) {  // Fibonacci-Heap-Union
            rootWithMinKey->insert(other.rootWithMinKey);
            if (!rootWithMinKey || (other.rootWithMinKey && other.rootWithMinKey->key() < rootWithMinKey->key()))
                this->rootWithMinKey = other.rootWithMinKey;
            count += other.count;
        }
	
        PNode insert (Data d, Key k) {
            if (debug) cout << "insert " << d << ":" << k << endl;
            count++;
            // create a new tree with a single m_key:
            return insertNode(new FibonacciHeapNode<Data,Key>(d,k));
        }


        void removeMinimum() {  // Fibonacci-Heap-Extract-Min, CONSOLIDATE
            if (!rootWithMinKey)
                throw string("trying to remove from an empty heap");

            if (debug) cout << "removeMinimum" << endl;
            count--;

            /// Phase 1: Make all the removed root's children new roots:
            // Make all children of root new roots:
            if (rootWithMinKey->child) {
                if (debugRemoveMin) {
                    cout << "  root's children: "; 
                    rootWithMinKey->child->printAll(cout);
                }
                PNode c = rootWithMinKey->child;
                do {
                    c->parent = NULL;
                    c = c->next;
                } while (c!=rootWithMinKey->child);
                rootWithMinKey->child = NULL; // removed all children
                rootWithMinKey->insert(c);
            }
            if (debugRemoveMin) {
                cout << "  roots after inserting children: "; 
                printRoots(cout);
            }
		

            /// Phase 2-a: handle the case where we delete the last m_key:
            if (rootWithMinKey->next == rootWithMinKey) {
                if (debugRemoveMin) cout << "  removed the last" << endl;
                if (count!=0)
                    throw string ("Internal error: should have 0 keys");
                rootWithMinKey = NULL;
                return;
            }

            /// Phase 2: merge roots with the same degree:
            vector<PNode> degreeRoots (maxDegree+1); // make room for a new degree
            fill (degreeRoots.begin(), degreeRoots.end(), (PNode)NULL);
            maxDegree = 0;
            PNode currentPointer = rootWithMinKey->next;
            uint currentDegree;
            do {
                currentDegree = currentPointer->degree;
                if (debugRemoveMin) {
                    cout << "  roots starting from currentPointer: "; 
                    currentPointer->printAll(cout);
                    cout << "  checking root " << *currentPointer << " with degree " << currentDegree << endl;
                }

                PNode current = currentPointer;
                currentPointer = currentPointer->next;
                while (degreeRoots[currentDegree]) { // merge the two roots with the same degree:
                    PNode other = degreeRoots[currentDegree]; // another root with the same degree
                    if (current->key() > other->key())
                        swap(other,current); 
                    // now current->key() <= other->key() - make other a child of current:
                    other->remove(); // remove from list of roots
                    current->addChild(other);
                    if (debugRemoveMin) cout << "  added " << *other << " as child of " << *current << endl;
                    degreeRoots[currentDegree]=NULL;
                    currentDegree++;
                    if (currentDegree >= degreeRoots.size())
                        degreeRoots.push_back((PNode)NULL);
                }
                // keep the current root as the first of its degree in the degrees array:
                degreeRoots[currentDegree] = current;
            } while (currentPointer != rootWithMinKey);

            /// Phase 3: remove the current root, and calcualte the new rootWithMinKey:
            delete rootWithMinKey;
            rootWithMinKey = NULL;

            uint newMaxDegree=0;
            for (uint d=0; d<degreeRoots.size(); ++d) {
                if (debugRemoveMin) cout << "  degree " << d << ": ";
                if (degreeRoots[d]) {
                    if (debugRemoveMin) cout << " " << *degreeRoots[d] << endl;
                    degreeRoots[d]->next = degreeRoots[d]->previous = degreeRoots[d];
                    insertNode(degreeRoots[d]);
                    if (d>newMaxDegree)
                        newMaxDegree = d;		
                } else {
                    if (debugRemoveMin) cout << "  no node" << endl;
                }
            }
            maxDegree=newMaxDegree;
        }
	
        void decreaseKey(PNode node, Key newKey) {
            if (newKey >= node->m_key)
                throw string("Trying to decrease key to a greater key");

            if (debug) cout << "decrease key of " << *node << " to " << newKey << endl;
            // Update the key and possibly the min key:
            node->m_key = newKey;

            // Check if the new key violates the heap invariant:
            PNode parent = node->parent;
            if (!parent) { // root node - just make sure the minimum is correct
                if (newKey < rootWithMinKey->key())
                    rootWithMinKey = node;
                return; // heap invariant not violated - nothing more to do
            } else if (parent->key() <= newKey) {
                return; // heap invariant not violated - nothing more to do
            }

            for(;;) {
                parent->removeChild(node);
                insertNode(node);
                if (debugDecreaseKey) {
                    cout << "  removed " << *node << " as child of " << *parent << endl;
                    cout << "  roots after remove: "; 
                    rootWithMinKey->printAll(cout);
                }

                if (!parent->parent) { // parent is a root - nothing more to do
                    break;
                } else if (!parent->mark) {  // parent is not a root and is not marked - just mark it
                    parent->mark = true;
                    break;
                } else {
                    node = parent;
                    parent = parent->parent;
                    continue;
                }
            };
        }

        void remove(PNode node, Key minusInfinity) {
            if (minusInfinity >= minimum()->key())
                throw string("2nd argument to remove must be a key that is smaller than all other keys");
            decreaseKey(node, minusInfinity);
            removeMinimum();
        }

    };  // FibonacciHeap



    int main() {
        try {
            FibonacciHeap<string, uint> h;
            h.debug=true;
            h.debugRemoveMin=false;
            h.debugDecreaseKey = false;

            h.insert("a",4);
            h.insert("b",2);
            h.insert("c",7);
            h.insert("d",5);
            h.insert("e",1);
            h.insert("f",8);
            h.printRoots(cout);

            while (!h.empty()) {
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

            for (uint i=0; i<nodes.size(); ++i) {
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

            while (!h.empty()) {
                cout << "min=" << *h.minimum() << endl;
                h.removeMinimum(); 
                h.printRoots(cout);
            }

            cout << endl << endl;

        } catch (string s) {
            cerr << endl << "ERROR: " << s << endl;
        }
    }




