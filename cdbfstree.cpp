/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2011-11-04                                                          */
/* File: cdbfstree.cpp                                                       */
/* Description:                                                              */
/*   Contains the implementation details for a CDBFS branching tree class.   */
/*****************************************************************************/
#include "state.h"
#include "btree.h"
#include "cdbfstree.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

/*****************************************************************************/
/* CDBFSTree constructor, destructor, and initialization functions           */
/*****************************************************************************/
CDBFSTree::CDBFSTree(int probType, bool isIntProb) :
	BTree(probType, isIntProb),
	nextBest(NULL),
	numUnexploredStates(0)
{
	name = "*** CDBFS ***";
    cdbfsStats = new CDBFSTreeStats();
    tStats = cdbfsStats; // Make tStats pointer point to cdbfsStats
}

CDBFSTree::~CDBFSTree()
{
    // Calls BTree destructor by default
    // Now deletes all allocated priority queues for the different levels
    for (int i = 0; i < unexploredStates.size(); ++i) {
        delete unexploredStates[i];
    }

	// If states are not being retained in the dominance classes, we need 
	// to make sure they're deleted from the tree prior to termination
	while (!isEmpty()) {
	   State *ns = getNextState();
	   delete ns;
	}
}

/*****************************************************************************/
/* CDBFSTree search functions                                                */
/*****************************************************************************/
State *CDBFSTree::getNextState()
{

    if (nextBest != NULL) {
        State *temp = nextBest;
        nextBest = NULL;
        lastLevelExplored = temp->getDepth();
        --numUnexploredStates;
//        printf("--- continuing dive: ");
//        temp->print();
        return temp;
    }

    // Else we need to start a dive, so grab state from the next level SPQ
    int nextLevel = (lastDiveLevel + 1) % unexploredStates.size();
    state_priority_queue *nextLevelSPQ = unexploredStates[nextLevel];
    while (nextLevelSPQ->empty()) {
        nextLevel = (nextLevel >= unexploredStates.size()-1) ? 0 : nextLevel+1;
        nextLevelSPQ = unexploredStates[nextLevel];
    }
    State *nextState = (*nextLevelSPQ).top();
    (*nextLevelSPQ).pop();
    --numUnexploredStates;

    while (nextState->isDominated()) {
        // Just exhaust at this level
        if ((*nextLevelSPQ).empty()) {
            // No new states to explore (at this level) except current 
            // dominated one, so return it to tree's exploreNextState function 
            // for proper termination
            lastDiveLevel = nextState->getDepth();
            lastLevelExplored = nextState->getDepth();
            return nextState;
        } // else
        nextState = (*nextLevelSPQ).top();
        (*nextLevelSPQ).pop();
        --numUnexploredStates;
    }

    if (debug >= 2) {
        printf("Next state taken from level %d\n", (*nextState).getDepth());
    }
    lastDiveLevel = nextState->getDepth();
    lastLevelExplored = nextState->getDepth();

//    printf("+++  Beginning dive: ");
//    nextState->print();

    return nextState;
}

void CDBFSTree::saveStateForExploration(State *s)
{
    ++numUnexploredStates;
    int level = (*s).getDepth();
    // Check to see if we need to add new level priority queues to store state
    while (unexploredStates.size() <= level) {
        unexploredStates.push_back(new state_priority_queue());
    }
    if (level == lastLevelExplored) {
        // We're trying to re-add a state that was just explored, so just 
        // add it back to the level SPQ
        (*(unexploredStates[level])).push(s);
    } else {
        // This is a state at the next level
        if (nextBest == NULL) {
            nextBest = s;
        } else if (nextBest->getPriority() < s->getPriority()) {
            // Move prev. best to level SPQ, make s best. Note that this 
            // assumes that s and nextBest are at the same level.
            int nbLevel = nextBest->getDepth();
            (*(unexploredStates[nbLevel])).push(nextBest);
            nextBest = s;
        } else {
            // We need to add s but it's not the best so just add to level SPQ
            (*(unexploredStates[level])).push(s);
        }
    }
    return;
}

/*****************************************************************************/
/* CDBFSTree miscellaneous function definitions                              */
/*****************************************************************************/
void CDBFSTree::printLevelSizes()
{
    for (int i = 0; i < unexploredStates.size(); ++i) {
        printf("%d: %d states\n", i, unexploredStates[i]->size());
    }
    return;
}

/*****************************************************************************/
/* CDBFSTreeStats function definitions                                       */
/*****************************************************************************/
CDBFSTreeStats::CDBFSTreeStats()
{
    // Calls TreeStats constructor by default
    // Initialize
}

CDBFSTreeStats::~CDBFSTreeStats()
{
    // Calls TreeStats destructor by default
    // Clean-up
}

void CDBFSTreeStats::print() const
{
    TreeStats::print();
    // Put extra CDBFSTree printing here
    printf("Printing CDBFS Tree Stats\n");
    return;
}

