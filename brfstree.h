/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: brfstree.h                                                          */
/* Description:                                                              */
/*   Contains the design details for a BrFS branching tree class.            */
/*****************************************************************************/
#ifndef BRFSTREE_H
#define BRFSTREE_H

// Required include's
#include <queue>
using std::queue;

// Forward Declarations
class State;
class BTree;
struct TreeStats;

// Used for tracking search statistics
struct BrFSTreeStats : public TreeStats
{
    BrFSTreeStats();
    virtual ~BrFSTreeStats();
    virtual void print() const;

    // Variables for BrFSTreeStats
};

class BrFSTree : public BTree
{
  public:
    // Class functions unique to BrFSTree
    BrFSTree(int probType, bool isIntProb);
    virtual ~BrFSTree();

  protected:
    // Variables
    queue<State *> unexploredStates;

    // For tracking BrFS search statistics 
    BrFSTreeStats *brfsStats;

    // Abstract class functions inherited from BTree that will be implemented
    virtual bool isEmpty() const;
    virtual State *getNextState();
    virtual void saveStateForExploration(State *s);

  private:
    // Nothing
};

/*****************************************************************************/
/* BRFSTree inline function definitions                                      */
/*****************************************************************************/
inline bool BrFSTree::isEmpty() const
{
    return unexploredStates.empty();
}

#endif // BRFSTREE_H

