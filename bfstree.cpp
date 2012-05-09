/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: bfstree.cpp                                                         */
/* Description:                                                              */
/*   Contains the implementation details for a BFS branching tree class.     */
/*****************************************************************************/
#include "state.h"
#include "btree.h"
#include "bfstree.h"

#include <cstdio>
#include <cstdlib>

/*****************************************************************************/
/* BFSTree constructor, destructor, and initialization functions             */
/*****************************************************************************/
BFSTree::BFSTree(int probType, bool isIntProb) :
	BTree(probType, isIntProb)
{
	name = "*** BFS ***";
    bfsStats = new BFSTreeStats();
    tStats = bfsStats; // Make tStats pointer point to bfsStats
}

BFSTree::~BFSTree()
{
    // Calls BTree destructor by default
	
	// If states are not being retained in the dominance classes, we need 
	// to make sure they're deleted from the tree prior to termination
	while (!isEmpty()) {
	   State *ns = getNextState();
	   delete ns;
	}
}

/*****************************************************************************/
/* BFSTree search functions                                                  */
/*****************************************************************************/
State *BFSTree::getNextState()
{
    State *nextState = unexploredStates.top();
    unexploredStates.pop();
    while ((nextState->isDominated()) && (!unexploredStates.empty())) {
        nextState = unexploredStates.top();
        unexploredStates.pop();
        ++tStats->statesPrunedByDomBeforeExploration;
    }
    return nextState;
}

void BFSTree::saveStateForExploration(State *s)
{
    unexploredStates.push(s);
    return;
}

/*****************************************************************************/
/* BFSTreeStats function definitions                                        */
/*****************************************************************************/
BFSTreeStats::BFSTreeStats()
{
    // Calls TreeStats constructor by default
    // Initialize
}

BFSTreeStats::~BFSTreeStats()
{
    // Calls TreeStats destructor by default
    // Clean-up
}

void BFSTreeStats::print() const
{
    TreeStats::print();
    // Put extra BFSTree printing here
    printf("Printing BFS Tree Stats\n");
    return;
}

