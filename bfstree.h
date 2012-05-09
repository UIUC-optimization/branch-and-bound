/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: bfstree.h                                                           */
/* Description:                                                              */
/*   Contains the design details for a BFS branching tree class.             */
/*****************************************************************************/
#ifndef BFSTREE_H
#define BFSTREE_H

// Forward Declarations
class State;
class BTree;
struct TreeStats;

// Used for tracking search statistics
struct BFSTreeStats : public TreeStats
{
    BFSTreeStats();
    virtual ~BFSTreeStats();
    virtual void print() const;

    // Variables for BFSTreeStats
};

class BFSTree : public BTree
{
  public:
    // Class functions unique to BFSTree
    BFSTree(int probType, bool isIntProb);
    virtual ~BFSTree();

  protected:
    // Variables
    state_priority_queue unexploredStates;

    // For tracking BFS search statistics 
    BFSTreeStats *bfsStats;

    // Abstract class functions inherited from BTree that will be implemented
    virtual bool isEmpty() const;
    virtual State *getNextState();
    virtual void saveStateForExploration(State *s);

  private:
    // Nothing
};

/*****************************************************************************/
/* BFSTree inline function definitions                                       */
/*****************************************************************************/
inline bool BFSTree::isEmpty() const
{
    return unexploredStates.empty();
}

#endif // BFSTREE_H

