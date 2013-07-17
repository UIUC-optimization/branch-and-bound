/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: btree.cpp                                                           */
/* Description:                                                              */
/*   Contains the design details for an abstract branching tree class.       */
/*****************************************************************************/
#include "state.h"
#include "btree.h"
#include "util.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>

/*****************************************************************************/
/* BTree destructor and initialization functions                             */
/*****************************************************************************/
BTree::BTree(int probType, bool isIntProb) :
	globalLowerBound(std::numeric_limits<double>::min()),
	globalUpperBound(std::numeric_limits<double>::max()),
	problemType(probType),
	bestState(NULL),
	keepExploring(true),
	isOptIntegral(isIntProb),
	nextNodeID(0),
	nextEdgeID(0),
	exploredStates(0),
	tulipOutputFile(NULL),
	debug(1),
	useDom(false),
	retainStates(true),
	useFinalTests(false),
	findAllSolns(false),
	outputRate(1000),
	stateComputesBounds(false),
	nodeLimit(0),
	timeLimit(0.0),
	stopAtFirstImprov(false),
	saveNonTerm(false)
{
	/* Do nothing */
}

BTree::~BTree()
{
    if (retainStates) {
        // Need to delete all nodes if they've been retained (either for 
        // dominance or other purposes, such as tracking branching decisions)
        for (auto dcI = domClasses.begin(); dcI != domClasses.end();++dcI){
            DomClass *domClass = dcI->second;
            for (auto s = domClass->begin(); s != domClass->end(); ++s) {
                delete *s;
            }
            delete domClass;
        }
    } 
	delete tStats;
	delete bestState;
	if (tulipOutputFile)
	{
		fprintf(tulipOutputFile, ")\n");
		fclose(tulipOutputFile);
	}
}

/*****************************************************************************/
/* BTree search functions                                                    */
/*****************************************************************************/
void BTree::explore()
{
	runTimeStart = clock() - tStats->totalTime;
	bool finished = true;
    while ((!isEmpty()) && (keepExploring)) 
	{
		double loopStart = clock();
		State* oldBest = bestState;

        try { exploreNextState(); }
		catch (AbortException& e) { keepExploring = false; finished = false;}

		double loopEnd = clock();
		tStats->totalTime += (loopEnd - loopStart);
		if (oldBest != bestState) tStats->timeToOpt = tStats->totalTime;
        
		// Print output information
        if ((debug > 0) && ((tStats->statesExplored % outputRate) == 0)) 
            printProgress();
        
		// Determine whether to keep exploring based on time / node limits
        keepExploring &= 
            ((nodeLimit == 0 || tStats->statesExplored < nodeLimit) &&
             (timeLimit < eps || clock() - runTimeStart < timeLimit) && 
             (!stopAtFirstImprov || tStats->timesBestStateWasUpdated == 0) &&
			 (globalLowerBound < globalUpperBound));
    }

    if (debug > 0) {
        printf("* Finished *\n");
        if ((globalLowerBound < globalUpperBound) && (!isEmpty() || !finished)) {
            printf("Failed to explore entire tree; cannot guarantee "
                   "optimality\n");
        }
        if (debug >= 2) {
            printOptSolution();
        }
        printTreeStats();
        printf("%0.2fs total CPU time\n", (tStats->totalTime) / CLOCKS_PER_SEC);
        printf("%0.2fs time to opt\n", (tStats->timeToOpt) / CLOCKS_PER_SEC);
    }

    return;
}

void BTree::exploreNextState()
{
    State* ns = getNextState();

	if (ns->timeToExplore > tStats->statesExplored)
	{
		saveStateForExploration(ns);
		return;
	}

	if (tulipOutputFile)
	{
		fprintf(tulipOutputFile, "(property 0 string \"viewLabel\" ");
		fprintf(tulipOutputFile, "(node %d \"%d\"))\n", ns->getID(), exploredStates++);
	}

    if (stateIsPrunable(ns)) 
	{
        if (!retainStates) delete ns;
        return;
    }
    // Else if we get to this point, we weren't able to prune state using 
    // standard dominance or bounds. In some cases, we can now compute a more 
    // expensive dominance or bounds test that we ordinarily don't want to do. 
    if (useFinalTests) 
	{
        ns->applyFinalPruningTests(this);
        if (stateIsPrunable(ns)) 
		{
            if (!retainStates) delete ns;
            return;        
        }
    }

    // If we get to this point, we have no choice but to branch on the state. 
//    printf("Exploring state: ");
//    ns->print();
    while (ns->depth >= tStats->numExploredAtLevel.size()) 
        tStats->numExploredAtLevel.push_back(0);

    tStats->numExploredAtLevel[ns->depth]++;
    ++tStats->statesExplored;
    ns->branch(this);

    // If dominance is not used, the state can be deleted after branching 
    // (assuming state does not need to be retained for branching decisions)
    if (!retainStates) delete ns;

     return;
}

/*****************************************************************************/
/* BTree state processing functions                                          */
/*****************************************************************************/
bool BTree::processState(State* s, bool isRoot)
{
	s->id = nextNodeID++;
	if (tulipOutputFile)
	{
		fprintf(tulipOutputFile, "(nodes %d)\n", s->id);
		if (s->parID != -1) 
			fprintf(tulipOutputFile, "(edge %d %d %d)\n", nextEdgeID++, s->id, s->parID);
		s->writeTulipOutput(tulipOutputFile);
	}

    while (s->depth >= tStats->numIdentifiedAtLevel.size()) 
        tStats->numIdentifiedAtLevel.push_back(0);
    ++tStats->numIdentifiedAtLevel[s->depth];
    ++tStats->statesIdentified;

    // If s is a terminal state, process it and return
    if (s->isTerminalState()) 
	{
        processTerminalState(s);
		if (isRoot) printf("root is terminal!\n");
        return true;
    } 

	if (saveNonTerm) saveBestState(s, true);

    // Else check s for dominance
    if (useDom) 
	{
        applyDominanceRules(s);
        if (stateIsDominated(s)) 
		{
            ++tStats->statesPrunedByDomBeforeInsertion;
            delete s;
            return false;
        } 
    }

    // Else s is not dominated, so compute bounds and check pruning
	if (!stateComputesBounds)
		s->computeBounds(this);

    if (stateExceedsBounds(s)) 
	{
        ++tStats->statesPrunedByBoundsBeforeInsertion;
        delete s;
        return false;
    } 
	
	else if (isRoot) 
	{ 
		// Update global bounds based on root's values
        if (problemType == MIN) globalLowerBound = s->getLB();
        else /* problemType == MAX */ globalUpperBound = s->getUB();

		// If at the root, the lower bound and upper bounds are equal, we know
		// we're done; no need to explore the state
		if ((isOptIntegral && ceil(globalLowerBound) == floor(globalUpperBound)) ||
			(fabs(globalLowerBound - globalUpperBound) < eps))
		{
			delete s;
			return false;
		}
    }

    // Else s is not prunable yet, so compute priority and store in the tree.
    // Store non-terminal, non-prunable state in the appropriate dominance 
    // class, creating a new class if none currently exists for it.
    if (retainStates) 
	{
        auto domClassPos = domClasses.find(s->getDomClassID());
        if (domClassPos == domClasses.end()) 
		{
            DomClass* domClass = new DomClass();
            domClass->push_back(s);
            domClasses[s->getDomClassID()] = domClass;
        } else domClassPos->second->push_back(s);
    }

    while (s->depth >= tStats->numStoredAtLevel.size()) 
        tStats->numStoredAtLevel.push_back(0);
    ++tStats->numStoredAtLevel[s->depth];
    ++tStats->statesStoredInTree;
    saveStateForExploration(s);

    return true;
}

void BTree::processHeuristicState(State *s)
{
    processTerminalState(s, false);
    return;
}

bool BTree::attemptHeuristicCompletion(State *s) 
{
    return (((problemType == MIN) && (s->getLB() < globalUpperBound - eps)) ||
            ((problemType == MAX) && (s->getUB() > globalLowerBound + eps)));
}

void BTree::resetBest()
{
	delete bestState; bestState = NULL;
	keepExploring = true;
	tStats->timesBestStateWasUpdated = 0;
}

void BTree::setTulipOutputFile(const char* filename, const char* instName, long seed, bool deep) 
{
	if (!filename) return;
	if (!(tulipOutputFile = fopen(filename, "w")))
		throw ERROR << "Could not open " << filename << " for writing.";

	fprintf(tulipOutputFile, "(tlp \"2.3\"\n");
	fprintf(tulipOutputFile, "(comments \"");
	if (deep) fprintf(tulipOutputFile, "Deep ");
	else fprintf(tulipOutputFile, "Wide ");
	fprintf(tulipOutputFile, "branch-and-bound tree for %s; seed %ld.\")\n", instName, seed);
	fprintf(tulipOutputFile, "(property 0 color \"viewColor\" ");
	fprintf(tulipOutputFile, "(default \"(0,0,0,255)\" \"(0,0,0,255)\"))\n");
	fprintf(tulipOutputFile, "(property 0 string \"viewLabel\" ");
	fprintf(tulipOutputFile, "(default \"0\" \"0\"))\n");
	fprintf(tulipOutputFile, "(property 0 color \"viewLabelColor\" ");
	fprintf(tulipOutputFile, "(default \"(255,255,255,255)\" \"(0,0,0,255)\"))\n");
	fprintf(tulipOutputFile, "(property 0 int \"viewShape\" ");
	fprintf(tulipOutputFile, "(default \"14\" \"4\"))\n");
}

void BTree::processTerminalState(State *s, bool isTreeNode)
{
    if (isTreeNode) {
        ++tStats->terminalStatesIdentified;
    } else {
        ++tStats->heuristicStatesProcessed;
    }

	if (tulipOutputFile)
	{
		fprintf(tulipOutputFile, "(property 0 color \"viewColor\" ");
		fprintf(tulipOutputFile, "(node %d \"(0, 255, 0, 255)\"))\n", s->id);
	}

	saveBestState(s, isTreeNode);
	delete s;
	return;
}

void BTree::applyDominanceRules(State *s)
{
    // Check memory for dominance, marking states that this state dominates 
    // and checking if this state is dominated by any state
    auto domClassPos = domClasses.find(s->getDomClassID());
    if ((domClassPos == domClasses.end()) || (domClassPos->second == NULL)) { 
        // No dominance class exists yet
        return;
    } // else assess dominance for each state in dominance class
    DomClass *domC = domClassPos->second;
    // Determine if state is dominated by or dominates any state
    for (auto dcI = domC->begin(); dcI != domC->end(); ++dcI) {
        s->assessDominance(*dcI);
        if (s->isDominated()) break;
    }
    return;
}

/*****************************************************************************/
/* Pruning tests                                                             */
/*****************************************************************************/
inline
bool BTree::stateIsPrunable(State *s)
{
    // Apply standard dominance test. 
    if (stateIsDominated(s)) {
        ++tStats->statesPrunedByDomBeforeExploration;
        return true;
    }
    // Apply bounds test. 
    if (stateExceedsBounds(s)) {
        ++tStats->statesPrunedByBoundsBeforeExploration;
        return true;
    }
    return false;
}

inline
bool BTree::stateIsDominated(State *s)
{
    if (s->isDominated())
	{
        if (debug >= 3) 
            printf("State at depth %d is dominated\n", s->depth);

		if (tulipOutputFile)
		{
			fprintf(tulipOutputFile, "(property 0 color \"viewColor\" ");
			fprintf(tulipOutputFile, "(node %d \"(255, 0, 255, 255)\"))\n", s->id);
		}
        return true;
    } 

	return false;
}

inline
bool BTree::stateExceedsBounds(State *s)
{
    if (((problemType == MIN) && (s->getLB() >= globalUpperBound - eps)) ||
        ((problemType == MAX) && (s->getUB() <= globalLowerBound + eps))) 
	{
        // NOTE: checks for pruning should be >= and <=, respectively. 
        // Changing them to > and < allows us to identify all optimal 
        // solutions, but slows the search process down. 
        if (debug >= 3) 
		{
            printf("State at depth %d exceeds bounds: ", s->depth);
            if (problemType == MIN) 
                printf("LB(%.2f) > GUB(%.2f)\n", s->getLB(), globalUpperBound);
            else // problemType == MAX
                printf("UB(%.2f) < GLB(%.2f)\n", s->getUB(), globalLowerBound);
        }

		if (tulipOutputFile)
		{
			fprintf(tulipOutputFile, "(property 0 color \"viewColor\" ");
			fprintf(tulipOutputFile, "(node %d \"(255, 0, 0, 255)\"))\n", s->id);
		}
        return true;
    }

	return false;
}

// Save the best state we've found so far
void BTree::saveBestState(State* s, bool isTreeNode)
{
    double objVal = s->getObjValue();

    // NOTE: Modified to include round-off tolerance checks
//    if (((problemType == MIN) && (objVal == globalUpperBound)) ||
//        ((problemType == MAX) && (objVal == globalLowerBound))) {
    if (((problemType == MIN) && (fabs(objVal - globalUpperBound) <= eps)) ||
        ((problemType == MAX) && (fabs(objVal - globalLowerBound) <= eps))) 
	{
        // We found a solution of comparable quality to the best known
        if (isTreeNode)
            tStats->numOptimalTerminalStatesIdentified += 1;
        else // State constructed heuristically
            tStats->numOptimalHeuristicStatesProcessed += 1;

        if (bestState == NULL || findAllSolns) // Unconditionally update best state
		{ 
			++tStats->timesBestStateWasUpdated;
            bestState = s->clone();
            if (debug > 0)
                printProgress(true);
        } 
		else return;
    } 
	else if (((problemType == MIN) && (objVal < globalUpperBound - eps)) ||
               ((problemType == MAX) && (objVal > globalLowerBound + eps))) 
	{
        // We found a solution of better quality than the best known
        delete bestState;
        bestState = s->clone();

        if (problemType == MIN)
            globalUpperBound = objVal;
        else // problemType == MAX
            globalLowerBound = objVal;

		if (globalUpperBound < globalLowerBound)
		{
			throw ERROR << "globalUpperBound " << globalUpperBound << " is less than "
				<< "globalLowerBound " << globalLowerBound;
		}

        if (debug > 0) printProgress(true); 

        // Update search statistics
        ++tStats->timesBestStateWasUpdated;
        tStats->statesIdentifiedAtLastUpdate = tStats->statesIdentified;
        tStats->statesStoredInTreeAtLastUpdate = tStats->statesStoredInTree;
        tStats->statesExploredAtLastUpdate = tStats->statesExplored;
        if (isTreeNode) 
		{
            tStats->numOptimalTerminalStatesIdentified = 1;
            tStats->numOptimalHeuristicStatesProcessed = 0;
        } 
		else // State constructed heuristically 
		{ 
            tStats->numOptimalTerminalStatesIdentified = 0;
            tStats->numOptimalHeuristicStatesProcessed = 1;
        }
	}
}


/*****************************************************************************/
/* Printing functions only available in BTree                                */
/*****************************************************************************/
void BTree::printOptSolution() const
{
    if (bestState != NULL) {
        if (tStats->numOptimalTerminalStatesIdentified > 0) {
            printf("The best solution (found in the tree) is:\n");
        } else {
            printf("The best solution (found heuristically) is:\n");
        }
        (*bestState).print();
    } else {
        printf("The best solution is the initial global ");
        if (problemType == MIN) {
            printf("upper bound.\n");
        } else { // (problemType == MAX) 
            printf("lower bound.\n");
        }
    }
    return;
}

void BTree::printTreeStats() const
{
    if (problemType == MIN) {
        printf("The optimal value is: %.2f\n", globalUpperBound);
    } else { // problemType == MAX
        printf("The optimal value is: %.2f\n", globalLowerBound);
    }
    tStats->print();
    return;
}

void BTree::printProgress(bool newIncumbent) const
{
    if (newIncumbent) {
        printf("* ");
    } else {
        printf("  ");
    }
    printf("Explored %10d/%d states: ", tStats->statesExplored, 
                                        tStats->statesStoredInTree);
    if (globalLowerBound > -std::numeric_limits<double>::max()) {
        printf("< %10.2f ", globalLowerBound);
    } else {
        printf("<       -Inf ");
    }
    if (globalUpperBound < std::numeric_limits<double>::max()) {
        printf("| %10.2f >", globalUpperBound);
    } else {
        printf("|        Inf >");
    }
    printf(" (%6.2fs) (TTB %6.2fs)\n", 
            (clock() - runTimeStart) / CLOCKS_PER_SEC, 
            (tStats->timeToOpt) / CLOCKS_PER_SEC);
    return;
}

/*****************************************************************************/
/* TreeStats function definitions                                            */
/*****************************************************************************/
TreeStats::TreeStats()
{
    statesIdentified = 0;
    statesExplored = 0;
    statesStoredInTree = 0;
    terminalStatesIdentified = 0;
    heuristicStatesProcessed = 0;

    statesPrunedByBoundsBeforeInsertion = 0;
    statesPrunedByBoundsBeforeExploration = 0;
    statesPrunedByDomBeforeInsertion = 0;
    statesPrunedByDomBeforeExploration = 0;

    timesBestStateWasUpdated = 0;
    statesIdentifiedAtLastUpdate = 0;
    statesStoredInTreeAtLastUpdate = 0;
    statesExploredAtLastUpdate = 0;

    numOptimalTerminalStatesIdentified = 0;
    numOptimalHeuristicStatesProcessed = 0;
	totalTime = 0;
}

TreeStats::~TreeStats()
{
    // Nothing to clean up
}

void TreeStats::print() const
{
    printf("Search statistics:\n------------------\n");
    printf("%d states were identified\n", statesIdentified);
    printf("%d states were explored\n", statesExplored);
    printf("%d states were stored in the tree\n", statesStoredInTree);
    printf("%d terminal states were identified\n", terminalStatesIdentified);
    printf("%d heuristic states were processed\n", heuristicStatesProcessed);
    printf("%d updates of the best state performed\n\n", 
            timesBestStateWasUpdated);
    printf("Bounds statistics:\n------------------\n");
    printf("%d states were pruned by bounds before insertion\n", 
            statesPrunedByBoundsBeforeInsertion);
    printf("%d states were pruned by bounds before exploration\n\n", 
            statesPrunedByBoundsBeforeExploration);
    printf("Dominance statistics:\n---------------------\n");
    printf("%d states were pruned by dominance rules before insertion\n", 
            statesPrunedByDomBeforeInsertion);
    printf("%d states were pruned by dominance rules before exploration\n\n", 
            statesPrunedByDomBeforeExploration);
    printf("Optimality Statistics:\n----------------------\n");
    printf("%d optimal terminal states found\n", 
            numOptimalTerminalStatesIdentified);
    printf("%d optimal heuristic solutions processed\n\n", 
            numOptimalHeuristicStatesProcessed);
    printf("%d states were identified when the optimal solution was found\n", 
            statesIdentifiedAtLastUpdate);
    printf("%d states had been stored in the tree when the optimal solution "
           "was found\n", statesStoredInTreeAtLastUpdate);
    printf("%d states were explored when the optimal solution was found\n", 
            statesExploredAtLastUpdate);
    printf("%d = %d - %d states remained in tree when optimal solution was "
           "found\n\n", 
            statesStoredInTreeAtLastUpdate - statesExploredAtLastUpdate, 
            statesStoredInTreeAtLastUpdate, statesExploredAtLastUpdate);

    
    if (numIdentifiedAtLevel.size() > 0)
    {
        printf("Identified states at level:\n");
        printf("{%d", numIdentifiedAtLevel[0]);
        int zeroCount = 0;
        for (int i = 1; i < numIdentifiedAtLevel.size(); ++i)
        {
            if (numIdentifiedAtLevel[i] == 0) { ++zeroCount; continue; }
            else
            {
                if (zeroCount > 0)
                    printf(", <%d empty levels>", zeroCount);
                zeroCount = 0;
                printf(", %d", numIdentifiedAtLevel[i]);
            }
        }
        printf("};\n");
    }
    if (numExploredAtLevel.size() > 0)
    {
        printf("Explored states at level:\n");
        printf("{%d", numExploredAtLevel[0]);
        int zeroCount = 0;
        for (int i = 1; i < numExploredAtLevel.size(); ++i)
        {
            if (numExploredAtLevel[i] == 0) { ++zeroCount; continue; }
            else
            {
                if (zeroCount > 0)
                    printf(", <%d empty levels>", zeroCount);
                zeroCount = 0;
                printf(", %d", numExploredAtLevel[i]);
            }
        }
        printf("};\n");
    }
    if (numStoredAtLevel.size() > 0)
    {
        printf("Stored states at level:\n");
        printf("{%d", numStoredAtLevel[0]);
        int zeroCount = 0;
        for (int i = 1; i < numStoredAtLevel.size(); ++i)
        {
            if (numStoredAtLevel[i] == 0) { ++zeroCount; continue; }
            else
            {
                if (zeroCount > 0)
					printf(", <%d empty levels>", zeroCount);
                zeroCount = 0;
                printf(", %d", numStoredAtLevel[i]);
            }
        }
        printf("};\n");
    }
    printf("\n");
	return;
}

