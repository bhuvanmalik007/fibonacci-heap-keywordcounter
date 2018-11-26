//
//  main.cpp
//  keywordcounter
//
//  Created by bhuvan malik on 07/11/18.
//  Copyright © 2018 bhuvan malik. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
using namespace std;

//  Class describing the structure of every node present in the fibonacci heap
class node{
    public:
    int count;               // Number of times the searchString has been searched in the search engine
    string searchString;
    bool childCut;
    node *right, *left, *parent, *child;
    unsigned long int degree;
    
    //  Constructor initializing the data members of the class.
    node(string searchString, int count)
    {
        this->left = this;
        this->right = this;
        this->parent = NULL;
        this->child = NULL;
        this->childCut = false;
        this->degree = 0;
        this->count = count;
        this->searchString = searchString;
    }
};

node* maxNode = NULL;               //  Pointer pointing to the max node
// Hash degreeTracker to store search strings and their corresponding pointers pointing to the nodes in the heap.
map< string, node*> hashdegreeTracker;
map< string, node*>::iterator iter;
static unsigned long int nodeCount;             // Keeping track of total number of nodes inside the heap

void makeLeftNode(node* nodeToBeInserted, node* rightNode){
    nodeToBeInserted->right = rightNode;
    rightNode->left->right = nodeToBeInserted; //   Making the old left node point to the new left node of the right node.
    nodeToBeInserted->left = rightNode->left;  //   Making new left node point to the old left node
    rightNode->left = nodeToBeInserted;        // Right node now points to the newly inserted left node.
}

void removeNodeFromSiblingList(node* nodeToBeRemoved){
    nodeToBeRemoved->right->left = nodeToBeRemoved->left;
    nodeToBeRemoved->left->right = nodeToBeRemoved->right;
}

void insertNode(node* nodeToBeInserted, bool existingNode)
{
    //  If no node exists in the heap, make the 'nodeToBeInserted' node the max node
    if(maxNode == NULL)
    {
        maxNode = nodeToBeInserted;
        maxNode->parent = NULL;
        if(existingNode != true)
        {
            nodeCount++;
        }
    }
    else
    {
        //  Adding to the root list, to the left of the max node
        makeLeftNode(nodeToBeInserted, maxNode);
        //  parent = NULL because the node is now in the root level list
        nodeToBeInserted->parent = NULL;
        //  Maintaining node count
        if(existingNode != true)
        {
            nodeCount++;
        }
        //  Updating maxNode pointer if new node > current maxNode
        if(nodeToBeInserted->count > maxNode->count)
        {
            maxNode = nodeToBeInserted;
        }
    }
}

void removeNode(node* updatedNode, node* parentOfUpdatedNode){
    parentOfUpdatedNode->degree--;      // Since parent is losing a child, its degree decreases
    //  If parent's child pointer doesn't point to the node being removed
    if(parentOfUpdatedNode->child != updatedNode){
        // Updating pointers of the neighbours of the node being removed, making them point to each other
        removeNodeFromSiblingList(updatedNode);
    }
    else{
        if(updatedNode->right == updatedNode)       // If only child, parent's child pointer becomes NULL
            parentOfUpdatedNode->child = NULL;
        else{
            // Parent's child pointer points to right sibling of the removed node
            parentOfUpdatedNode->child = updatedNode->right;
            removeNodeFromSiblingList(updatedNode);
        }
    }
    updatedNode->left = updatedNode;
    updatedNode->right = updatedNode;
    updatedNode->childCut = false;
    insertNode(updatedNode,true);       // Insert in top level list
}

void cascadingCut(node* updatedNode)
{
    node* parentOfUpdated = updatedNode->parent;
    if(parentOfUpdated != NULL) // Stopping condition for recursion
    {
        if(updatedNode->childCut == true)
        {
            removeNode(updatedNode,parentOfUpdated);
            //  Recursively call cascadingCut on parents of removed node untill a parent has childCut = false
            cascadingCut(parentOfUpdated);
        }
        else    // Stopping condition for recursion
        {
            updatedNode->childCut = true;
        }
       
    }
}

void increaseKey(node* nodeToBeUpdated, bool hasParent){
    if(hasParent){
        node* parentNode = nodeToBeUpdated->parent;
        //  if child is now > parent, remove the child and start cascading cuts
        if(nodeToBeUpdated->count > parentNode->count){
            removeNode(nodeToBeUpdated, parentNode);
            cascadingCut(parentNode);
        }
        
    }
    // if the nodeToBeUpdated has its count > maxNode, update the maxNode
    if(nodeToBeUpdated->count > maxNode->count)
    {
//        cout<<"nodeToBeUpdated->count > maxNode->count\n";
//        cout<<nodeToBeUpdated->searchString<<" "<<nodeToBeUpdated->count<<" > "<<maxNode->searchString<<" "<<maxNode->count<<"\n";
        maxNode = nodeToBeUpdated;
    }
}

void combineParentChild(node* child, node* parent)
{
    parent->degree++;  // As parent now has a new child
    removeNodeFromSiblingList(child);   //  Removing child from root level list
    //  Updating parent and child cut of the child
    child->parent = parent;
    child->childCut = false;
    if(parent->child == NULL)   //  If parent didn't have a child before
    {
        parent->child = child;
        child->left = child;
        child->right = child;
    }
    else     // Else, adding child to list of children
    {
        child->right = parent->child;
        child->left = (parent->child)->left;
        ((parent->child)->left)->right = child;
        (parent->child)->left = child;
        // Updating child pointer to this new child if its count > previous child pointer node
        if(child->count < parent->child->count)
        {
            parent->child = child;
        }
    }
    
}

void pairwiseCombine()
{
    //  Vector for tracking degrees of different trees
    vector<node*> degreeTracker;
    unsigned long int rootNodes = 0;
    if(maxNode == NULL)
    {
        return;
    }
    else
    {
        node* iterator = maxNode;
        //  Traversing the root nodes while counting them
        do{
            ++rootNodes;
            iterator = iterator->right;
        }while(iterator != maxNode);
    }
    
    node* current = maxNode;
    /*  MakeChild and makeParent are for same degree trees and comparing their max counts
     making them child/parent of each other accordingly */
    node* makeChild = NULL;
    node* makeParent = NULL;
    node* nextnode = NULL;
    unsigned long int degree = 0;
    while(rootNodes > 0)
    {
        nextnode = current->right;
        degree = current->degree;
        while(degree >= degreeTracker.size())
        {
            degreeTracker.push_back(NULL);
        }
        //  Checking if same degree node exists in the vector. If so, combining them and also checking if a node exists with same degree as the combined nodes and so on..
        while(degreeTracker[degree] != NULL && degreeTracker[degree] != current)
        {
            //  Determining the child-parent relationships
            if(degreeTracker[degree]->count > current->count)
            {
                makeParent = degreeTracker[degree];
                makeChild = current;
                combineParentChild(makeChild,makeParent);
                if(makeChild == maxNode)
                {
                    maxNode = makeParent;
                }
                current = makeParent;
            }
            // Reverse of previous case
            else
            {
                makeParent = current;
                makeChild = degreeTracker[degree];
                combineParentChild(makeChild, makeParent);
                if(makeChild == maxNode)
                {
                    maxNode = makeParent;
                }
                current = makeParent;
            }
            degreeTracker[degree] = NULL;
            ++degree; // After combining, degree = degree + 1
            while(degree >= degreeTracker.size())
            {
                degreeTracker.push_back(NULL);
            }
        }
        degreeTracker[degree] = current;  // Inserting current in vector for later use in combining
        if(maxNode->count<current->count)
        {
            maxNode = current;
        }
        current = nextnode; //  Updating the current node for next iteration
        --rootNodes;
    }
}


node* removeMaxNode(){
    node* toBeRemoved = maxNode;
    //  If no nodes are there to be removed, return max node=NULL
    if(maxNode == NULL)
    {
        return maxNode;
    }
    //  if maxNode has neither children nor any siblings
    else if(toBeRemoved->child == NULL && toBeRemoved->right == toBeRemoved){
        maxNode = NULL;
    }
    // If maxNode has children
    else if(toBeRemoved->child != NULL){
        node* child = toBeRemoved->child;
        node* firstchild = child;       // firstChild keeps pointing to the first child, will be used as stopping condition
        node* nextNode;
        do{                 // Iteratively inserting all the children of the maxNode into the top level list
            node* nodeToBeInserted = child;
            nextNode = child->right;
            nodeToBeInserted->left = nodeToBeInserted;
            nodeToBeInserted->right = nodeToBeInserted;
            insertNode(nodeToBeInserted, true);
            child = nextNode;
        }while(nextNode != firstchild);
    }
    // If maxNode has siblings at the root level, adjust the sibling pointers
    if(toBeRemoved != toBeRemoved->right){
        removeNodeFromSiblingList(toBeRemoved);
        maxNode = toBeRemoved->right;   // setting a temporary maxNode
        pairwiseCombine();  // Combining all the siblings
    }
    else{
        cout<<"\nERROR\n";
    }
    //  Resetting the removed max node
    toBeRemoved->left = toBeRemoved;
    toBeRemoved->right = toBeRemoved;
    toBeRemoved->parent = NULL;
    toBeRemoved->child = NULL;
    toBeRemoved->childCut = false;
    toBeRemoved->degree = 0;
    return toBeRemoved;
}

int main(int argc, const char * argv[]) {
    ifstream infile(argv[1]);
    std::string line;
    ofstream fout;
    fout.open("output_file.txt");
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        string query, count;
        // if 2 tokens in the line
        if (iss >> query >> count) {
            if(query[0] != '$'){
                cout<<"\ninvalid query: "<<query<<"\n";
                continue;
            }
            stringstream ss(count);
            int integerCount;
            ss >> integerCount;            //  convert the input to an integer
            //  record doesn't exist in hash degreeTracker
            if(hashdegreeTracker.find(query) == hashdegreeTracker.end()){
                node* record = new node(query, integerCount);
                //  Adding new record in hash degreeTracker
                hashdegreeTracker[query] = record;
                //  Inserting in fibonacci heap
                insertNode(record, false);
            }
            //  record exists in hash degreeTracker
            else{
                iter=hashdegreeTracker.find(query);
                node* matchingNode=iter->second;
                //  increasing count of the matching node
                matchingNode->count += integerCount;
                //  Calling increaseKey function
                increaseKey(matchingNode, (matchingNode->parent != NULL));
            }
        }
        // if not 2 tokens in the line
        else{
            //  If stop encountered, exit.
            if(line[0] == 's' || line[0] == 'S'){
                cout<<"'stop' encountered, result successfully written to output_file.txt";
                return 0;
            }
            // output top 'nodesToRemove' searches
            else{
                stringstream streamLine(line);
                int nodesToRemove;
                streamLine>>nodesToRemove;  //convert the input to an integer
                //  Vector to keep track of deleted nodes
                vector<node* > deletedNodesVector;
                unsigned long int counter=0;
                //  Remove top n max nodes
                while(counter < nodesToRemove)
                {
                    //  removeMaxNode returns the max node that was removed from the heap
                    node* maxNode=removeMaxNode();
                    //  push_back function pushes an element into the end of the vector
                    deletedNodesVector.push_back(maxNode);
                    string outputString = maxNode->searchString;
                    //  Removing the '$' symbol from the beginning
                    outputString = outputString.erase(0,1);
                    //  Writing to file
                    fout<<outputString;
                    if((counter + 1) != nodesToRemove)
                        fout<<",";
                    ++counter;
                }
                fout<<endl;
                counter=0;
                //  Re-inserting the removed nodes into the fibonacci heap
                while(counter<nodesToRemove)
                {
                    insertNode(deletedNodesVector[counter],false);
                    counter++;
                }
            }
        } // end of else
    } // end of input stream while loop
    return 0;
} // end of main


