/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: dfstree.cpp                                                         */
/* Description:                                                              */
/*   Contains the implementation details for a DFS branching tree class.     */
/*****************************************************************************/
#include "state.h"
#include "btree.h"
#include "dfstree.h"

#include <cstdio>
#include <cstdlib>

/*****************************************************************************/
/* DFSTree constructor, destructor, and initialization functions             */
/*****************************************************************************/
DFSTree::DFSTree(int probType, bool isIntProb) :
	BTree(probType, isIntProb)
{
	name = "*** DFS ***";
    dfsStats = new DFSTreeStats();
    tStats = dfsStats; // Make tStats pointer point to dfsStats
}

DFSTree::~DFSTree()
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
/* DFSTree search functions                                                  */
/*****************************************************************************/
State *DFSTree::getNextState()
{
    State* nextState = unexploredStates.top();
    unexploredStates.pop();
    while ((nextState->isDominated()) && (!unexploredStates.empty())) {
        nextState = unexploredStates.top();
        unexploredStates.pop();
        ++tStats->statesPrunedByDomBeforeExploration;
    }
    return nextState;
}

void DFSTree::saveStateForExploration(State *s)
{
    unexploredStates.push(s);
    return;
}

/*****************************************************************************/
/* DFSTreeStats function definitions                                        */
/*****************************************************************************/
DFSTreeStats::DFSTreeStats()
{
    // Calls TreeStats constructor by default
    // Initialize
}

DFSTreeStats::~DFSTreeStats()
{
    // Calls TreeStats destructor by default
    // Clean-up
}

void DFSTreeStats::print() const
{
    TreeStats::print();
    // Put extra DFSTree printing here
    printf("Printing DFS Tree Stats\n");
    return;
}

