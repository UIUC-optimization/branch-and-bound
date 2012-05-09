/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: cbfstree.h                                                          */
/* Description:                                                              */
/*   Contains the design details for a CBFS branching tree class.            */
/*****************************************************************************/
#ifndef CBFSTREE_H
#define CBFSTREE_H

// Required include's
#include <cmath>

#include <vector>
using std::vector;
#include <list>
using std::list;

// Global constants for CBFSTree
const int STANDARD = 0;
const int RANDOM_FROM_TOP_K = 1;
const int K_BEST_AT_LEVEL = 2;

// Forward Declarations
class State;
class BTree;
struct TreeStats;
class LevelSPQ;

// Used for tracking search statistics
struct CBFSTreeStats : public TreeStats
{
    CBFSTreeStats();
    virtual ~CBFSTreeStats();
    virtual void print() const;

    // Variables for CBFSTreeStats
};

class CBFSTree : public BTree
{
  public:
    // Class functions unique to CBFSTree
    CBFSTree(int probType, bool isIntProb, int selMethod = STANDARD, int k = 1);
    virtual ~CBFSTree();

  protected:
    // Variables
    vector<state_priority_queue *> unexploredStates;
    list<state_priority_queue *> nonEmptyLevelSPQs;

    // For tracking CBFS search statistics 
    CBFSTreeStats *cbfsStats;

    // Abstract class functions inherited from BTree that will be implemented
    virtual bool isEmpty() const;
    virtual State *getNextState();
    virtual void saveStateForExploration(State *s);

    // Class functions specific to CBFSTree
    State *getNextStateStandard();
    State *getNextStateRandomFromTopK();
    State *getNextStateFromCurLevel();

    void printLevelSizes();
    int levelThreshold(int level);

    // Miscellaneous variables
    int selectionMethod;
	int numToSelect;
    int numExploredAtCurLevel;
    int lastLevelExplored;

  private:
    // Nothing
};

/*****************************************************************************/
/* CBFSTree inline function definitions                                      */
/*****************************************************************************/
inline bool CBFSTree::isEmpty() const
{
    return nonEmptyLevelSPQs.empty();
}

inline int CBFSTree::levelThreshold(int level) 
{
    return numToSelect;
    //return ceil(log(level) / log(2)) + 1;
}

#endif // CBFSTREE_H

