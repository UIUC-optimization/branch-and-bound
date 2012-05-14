/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: cbfstree.cpp                                                        */
/* Description:                                                              */
/*   Contains the implementation details for a CBFS branching tree class.    */
/*****************************************************************************/
#include "state.h"
#include "btree.h"
#include "cbfstree.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

/*****************************************************************************/
/* CBFSTree constructor, destructor, and initialization functions            */
/*****************************************************************************/
CBFSTree::CBFSTree(int probType, bool isIntProb, int selMethod, int k) :
	BTree(probType, isIntProb),
	selectionMethod(selMethod),
	numToSelect(k),
	numExploredAtCurLevel(0),
	lastLevelExplored(-1)
{
	name = "*** CBFS ***";
    cbfsStats = new CBFSTreeStats();
    tStats = cbfsStats; // Make tStats pointer point to cbfsStats
}

CBFSTree::~CBFSTree()
{
	// If states are not being retained in the dominance classes, we need 
	// to make sure they're deleted from the tree prior to termination
	while (!isEmpty()) {
	   State *ns = getNextState();
	   delete ns;
	}

    // Calls BTree destructor by default
    // Now deletes all allocated priority queues for the different levels
    for (int i = 0; i < unexploredStates.size(); ++i) {
        delete unexploredStates[i];
    }

}

/*****************************************************************************/
/* CBFSTree search functions                                                 */
/*****************************************************************************/
State *CBFSTree::getNextState()
{
    switch (selectionMethod) {
      case STANDARD:
        return getNextStateStandard();
      case RANDOM_FROM_TOP_K:
        return getNextStateRandomFromTopK();
      case K_BEST_AT_LEVEL:
        return getNextStateFromCurLevel();
      default:
        printf("Invalid selection method specified (%d)\n", selectionMethod);
        exit(-1);
    }
}

State *CBFSTree::getNextStateStandard()
{
    state_priority_queue* nextLevelSPQ = nonEmptyLevelSPQs.front();
    nonEmptyLevelSPQs.pop_front();

    State* nextState = nextLevelSPQ->top();
    nextLevelSPQ->pop();

	//printf("Exploring next state at level %d with lower bound %0.9f\n", nextState->getDepth(), -nextState->getPriority());
	//nextState->print();

    while (nextState->isDominated()) 
	{
        if ((*nextLevelSPQ).empty()) 
		{
            if (nonEmptyLevelSPQs.empty()) 
			{
                // No new states to explore except current dominated one, 
                // so return it to tree's exploreNextState function for 
                // proper termination
                lastLevelExplored = nextState->getDepth();
                return nextState;
            }
            nextLevelSPQ = nonEmptyLevelSPQs.front();
            nonEmptyLevelSPQs.pop_front();
        }
        nextState = (*nextLevelSPQ).top();
        (*nextLevelSPQ).pop();
    }

    if (!(*nextLevelSPQ).empty()) {
        // Place level spq at end of list
        nonEmptyLevelSPQs.push_back(nextLevelSPQ);
    }

    if (debug >= 2) {
        printf("Next state taken from level %d\n", (*nextState).getDepth());
    }
    lastLevelExplored = nextState->getDepth();

//    printf("+++  Exploring: ");
//    nextState->print();

    return nextState;
}

State *CBFSTree::getNextStateRandomFromTopK()
{
    state_priority_queue *nextLevelSPQ = nonEmptyLevelSPQs.front();
    nonEmptyLevelSPQs.pop_front();

    vector<State*> bestAtLevel;
    // Get the <numToSelect> best elements at this level.
    for (int i = 0; i < numToSelect; ++i) {
        // If the level's empty, terminate early
        if (nextLevelSPQ->empty()) {
            break;
        }

        // Need to actually remove these elements from the priority queue.
        // This is going to be relatively inefficient, but if it turns out
        // to be useful, we can develop a better data structure to use.
        bestAtLevel.push_back(nextLevelSPQ->top());
        nextLevelSPQ->pop();
    }

    // Keep track of how many things are in the bestAtLevel vector
    int numStates = bestAtLevel.size();

    // Randomly pick an element from the set of best
    int index = (int)(drand48() * numStates);
    State* nextState = bestAtLevel[index];

    // Check to see if this element is dominated
    while (nextState->isDominated()) {
        // No new states to explore except current dominated one, 
        // so return it to tree's exploreNextState function for 
        // proper termination
        if ((numStates == 0) && (nonEmptyLevelSPQs.empty())) {
            lastLevelExplored = nextState->getDepth();
            return nextState;
        }

        // This only happens if the current level is empty; in this case,
        // we move to the next level and start over.
        else if (numStates == 0) {
            ++tStats->statesPrunedByDomBeforeExploration;
            return getNextState();
        }

        // Otherwise, pull another random element from the bestAtLevel vector 
        // If the level isn't empty, go ahead and keep the bestAtLevel
        // vector full.  Need to make sure that this happens before the
        // below call, so that it appears before the element that got
        // removed.
        if (!nextLevelSPQ->empty()) {
            ++tStats->statesPrunedByDomBeforeExploration;
            bestAtLevel.push_back(nextLevelSPQ->top());
            nextLevelSPQ->pop();
            numStates++;
        }

        // This is why we don't use bestAtLevel.size(); remove just shunts
        // the element to the back of the vector instead of actually
        // deleting it, which is fine for what we need to do.
        remove(bestAtLevel.begin(), bestAtLevel.end(), nextState);
        numStates--;

        // Try again
        index = (int) (drand48() * numStates);
        nextState = bestAtLevel[index];
    }

    // Remove the selected element from the best vector, and push everything
    // else back onto the priority queue.
    remove(bestAtLevel.begin(), bestAtLevel.end(), nextState);
    numStates--;

    for (int i = 0; i < numStates; ++i) {
        nextLevelSPQ->push(bestAtLevel[i]); 
    }

    if (!(*nextLevelSPQ).empty()) {
        // Place level spq at end of list
        nonEmptyLevelSPQs.push_back(nextLevelSPQ);
    }

    if (debug >= 2) {
        printf("Next state taken from level %d\n", (*nextState).getDepth());
    }
    lastLevelExplored = nextState->getDepth();
    return nextState;
}

State *CBFSTree::getNextStateFromCurLevel()
{
    state_priority_queue *nextLevelSPQ = nonEmptyLevelSPQs.front();

    State *nextState = (*nextLevelSPQ).top();
    (*nextLevelSPQ).pop();

    while (nextState->isDominated()) {
        if ((*nextLevelSPQ).empty()) {
            nonEmptyLevelSPQs.pop_front(); // Remove the empty level SPQ
            numExploredAtCurLevel = 0;
            if (nonEmptyLevelSPQs.empty()) {
                // No new states to explore except current dominated one, 
                // so return it to tree's exploreNextState function for 
                // proper termination
                lastLevelExplored = nextState->getDepth();
                return nextState;
            }
            nextLevelSPQ = nonEmptyLevelSPQs.front();
        }
        nextState = (*nextLevelSPQ).top();
        (*nextLevelSPQ).pop();
    }
    ++numExploredAtCurLevel;
    if (debug >= 2) {
        printf("Next state taken from level %d (%d explored at this level)\n", 
            nextState->getDepth(), numExploredAtCurLevel);
    }

    if ((*nextLevelSPQ).empty()) {
        nonEmptyLevelSPQs.pop_front();
        numExploredAtCurLevel = 0;
    } else if (numExploredAtCurLevel >= levelThreshold(nextState->getDepth())) {
        nonEmptyLevelSPQs.pop_front();
        numExploredAtCurLevel = 0;
        nonEmptyLevelSPQs.push_back(nextLevelSPQ);
    } // else leave level SPQ at front of queue

    lastLevelExplored = nextState->getDepth();
    return nextState;
}

void CBFSTree::saveStateForExploration(State *s)
{
    int level = s->getDepth();
    // Check to see if we need to add new level priority queues to store state
    while (unexploredStates.size() <= level) 
        unexploredStates.push_back(new state_priority_queue());

    // Now insert state into the appropriate priority queue, first checking if 
    // the level was empty before the insert operation
    bool wasPreviouslyEmpty = (unexploredStates[level])->empty();
    (unexploredStates[level])->push(s);

    // Now check to see if we need to add the priority queue to the list of 
    // nonempty level state priority queues (may preempt all queues in list)
    if (wasPreviouslyEmpty) 
	{
        if (level == lastLevelExplored) 
		{
            // We're trying to re-add a state that was just explored, so we 
            // want this level's SPQ to be placed at the end of the list
            nonEmptyLevelSPQs.push_back(unexploredStates[level]);
            return;
        } // Else we need to do a bit more work

        bool isPrevLevelEmpty = (level > 0) ? (unexploredStates[level - 1])->empty() : true;
        if ((selectionMethod != K_BEST_AT_LEVEL) || (isPrevLevelEmpty) ||  
            (numExploredAtCurLevel >= levelThreshold(level - 1))) 
		{
            // Simply pre-empt the rest of the stuff in the queue if we're 
            // using normal selections, or if the previous level is empty or 
            // if we've explored enough at that level
            nonEmptyLevelSPQs.push_front(unexploredStates[level]);
        } 
		else 
		{
            // Otherwise previous level is still sitting at the front of the 
            // non-empty level SPQ's, so we need to do some reordering.
            state_priority_queue *prevLevelSPQ = nonEmptyLevelSPQs.front();
            nonEmptyLevelSPQs.pop_front();
            nonEmptyLevelSPQs.push_front(unexploredStates[level]);
            nonEmptyLevelSPQs.push_front(prevLevelSPQ);
        }
    }

    return;
}

/*****************************************************************************/
/* CBFSTree miscellaneous function definitions                               */
/*****************************************************************************/
void CBFSTree::printLevelSizes()
{
    for (int i = 0; i < unexploredStates.size(); ++i) {
        printf("%d: %d states\n", i, unexploredStates[i]->size());
    }
    return;
}

/*****************************************************************************/
/* CBFSTreeStats function definitions                                        */
/*****************************************************************************/
CBFSTreeStats::CBFSTreeStats()
{
    // Calls TreeStats constructor by default
    // Initialize
}

CBFSTreeStats::~CBFSTreeStats()
{
    // Calls TreeStats destructor by default
    // Clean-up
}

void CBFSTreeStats::print() const
{
    TreeStats::print();
    // Put extra CBFSTree printing here
    printf("Printing CBFS Tree Stats\n");
    return;
}

