/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: brfstree.cpp                                                        */
/* Description:                                                              */
/*   Contains the implementation details for a BrFS branching tree class.    */
/*****************************************************************************/
#include "state.h"
#include "btree.h"
#include "brfstree.h"

#include <cstdio>
#include <cstdlib>

/*****************************************************************************/
/* BRFSTree constructor, destructor, and initialization functions            */
/*****************************************************************************/
BrFSTree::BrFSTree(int probType, bool isIntProb) :
	BTree(probType, isIntProb)
{
	name = "*** BrFS ***";
    brfsStats = new BrFSTreeStats();
    tStats = brfsStats; // Make tStats pointer point to brfsStats
}

BrFSTree::~BrFSTree()
{
    // Calls BTree destructor by default
	
	// If states are not being retained in the dominance classes, we need 
	// to make sure they're deleted from the tree prior to termination
	if (!retainStates)
	{
		while (!isEmpty()) 
		{
		   State *ns = getNextState();
		   delete ns;
		}
	}
}

/*****************************************************************************/
/* BRFSTree search functions                                                 */
/*****************************************************************************/
State *BrFSTree::getNextState()
{
    State *nextState = unexploredStates.front();
    unexploredStates.pop();
    while ((nextState->isDominated()) && (!unexploredStates.empty())) {
        nextState = unexploredStates.front();
        unexploredStates.pop();
        ++tStats->statesPrunedByDomBeforeExploration;
    }
    return nextState;
}

void BrFSTree::saveStateForExploration(State *s)
{
    unexploredStates.push(s);
    return;
}

/*****************************************************************************/
/* BrFSTreeStats function definitions                                        */
/*****************************************************************************/
BrFSTreeStats::BrFSTreeStats()
{
    // Calls TreeStats constructor by default
    // Initialize
}

BrFSTreeStats::~BrFSTreeStats()
{
    // Calls TreeStats destructor by default
    // Clean-up
}

void BrFSTreeStats::print() const
{
    TreeStats::print();
    // Put extra BrFSTree printing here
    printf("Printing BrFS Tree Stats\n");
    return;
}

