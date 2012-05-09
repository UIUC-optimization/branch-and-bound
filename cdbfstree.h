/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2011-11-04                                                          */
/* File: cdbfstree.h                                                         */
/* Description:                                                              */
/*   Contains the design details for a CDBFS branching tree class.           */
/*****************************************************************************/
#ifndef CDBFSTREE_H
#define CDBFSTREE_H

// Required include's
#include <cmath>

#include <vector>
using std::vector;
#include <list>
using std::list;

// Forward Declarations
class State;
class BTree;
struct TreeStats;
class LevelSPQ;

// Used for tracking search statistics
struct CDBFSTreeStats : public TreeStats
{
    CDBFSTreeStats();
    virtual ~CDBFSTreeStats();
    virtual void print() const;

    // Variables for CDBFSTreeStats
};

class CDBFSTree : public BTree
{
  public:
    // Class functions unique to CDBFSTree
    CDBFSTree(int probType, bool isIntProb);
    virtual ~CDBFSTree();

  protected:
    // Variables
    vector<state_priority_queue *> unexploredStates;
    list<state_priority_queue *> nonEmptyLevelSPQs;

    // For tracking CDBFS search statistics 
    CDBFSTreeStats *cdbfsStats;

    // Abstract class functions inherited from BTree that will be implemented
    virtual bool isEmpty() const;
    virtual State *getNextState();
    virtual void saveStateForExploration(State *s);

    // Class functions specific to CDBFSTree
    void printLevelSizes();

    // Miscellaneous variables
    State *nextBest;
    int lastDiveLevel;
    int lastLevelExplored;
    int numUnexploredStates;

  private:
    // Nothing
};

/*****************************************************************************/
/* CDBFSTree inline function definitions                                      */
/*****************************************************************************/
inline bool CDBFSTree::isEmpty() const
{
    return (numUnexploredStates == 0);
}

#endif // CDBFSTREE_H

