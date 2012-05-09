/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: dfstree.h                                                           */
/* Description:                                                              */
/*   Contains the design details for a DFS branching tree class.             */
/*****************************************************************************/
#ifndef DFSTREE_H
#define DFSTREE_H

// Required include's
#include <stack>
using std::stack;

// Forward Declarations
class State;
class BTree;
struct TreeStats;

// Used for tracking search statistics
struct DFSTreeStats : public TreeStats
{
    DFSTreeStats();
    virtual ~DFSTreeStats();
    virtual void print() const;

    // Variables for DFSTreeStats
};

class DFSTree : public BTree
{
  public:
    // Class functions unique to DFSTree
    DFSTree(int probType, bool isIntProb);
    virtual ~DFSTree();

  protected:
    // Variables
    stack<State *> unexploredStates;

    // For tracking DFS search statistics 
    DFSTreeStats *dfsStats;

    // Abstract class functions inherited from BTree that will be implemented
    virtual bool isEmpty() const;
    virtual State *getNextState();
    virtual void saveStateForExploration(State *s);

  private:
    // Nothing
};

/*****************************************************************************/
/* DFSTree inline function definitions                                       */
/*****************************************************************************/
inline bool DFSTree::isEmpty() const
{
    return unexploredStates.empty();
}

#endif // DFSTREE_H

