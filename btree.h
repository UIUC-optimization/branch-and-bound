/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: btree.h                                                             */
/* Description:                                                              */
/*   Contains the design details for an abstract branching tree class.       */
/*****************************************************************************/
#ifndef BTREE_H
#define BTREE_H

// Required include's
#include <vector>
using std::vector;
#include <list>
using std::list;
#include <queue>
using std::priority_queue;
#include <unordered_map>
using std::unordered_map;
#include <exception>
using std::exception;
#include <ctime>
#include <limits>
#include <string>
using std::string;

// Global definitions
#define CBFS_ALG 0x01
#define DFS_ALG 0x02
#define BFS_ALG 0x04
#define BrFS_ALG 0x08
#define ALL_ALG 0xFF

// Global constants
const int MIN = 0;
const int MAX = 1;

const double eps = 10e-05;

// Forward Declarations
class State;
struct StateComparator;
//class SortStatePriority;

// Typedef's for various data structures
typedef list<State*> DomClass;
typedef priority_queue<State*, vector<State*>, StateComparator> state_priority_queue;
typedef unordered_map<int, DomClass *> dominance_class_map;

// Used for tracking search statistics
struct TreeStats
{
    TreeStats();
    virtual ~TreeStats();
    virtual void print() const;

    // Variables for TreeStats
    int statesIdentified;
    int statesExplored;
    int statesStoredInTree;
    int terminalStatesIdentified;
    int heuristicStatesProcessed;

    int statesPrunedByBoundsBeforeInsertion;
    int statesPrunedByBoundsBeforeExploration;
    int statesPrunedByDomBeforeInsertion;
    int statesPrunedByDomBeforeExploration;

    int timesBestStateWasUpdated;
    int statesIdentifiedAtLastUpdate;
    int statesExploredAtLastUpdate;
    int statesStoredInTreeAtLastUpdate;

	int numOptimalTerminalStatesIdentified;
    int numOptimalHeuristicStatesProcessed;

	double totalTime;
    double timeToOpt;

	vector<int> numIdentifiedAtLevel;
	vector<int> numExploredAtLevel;
	vector<int> numStoredAtLevel;
};

class AbortException : public std::exception
{
public:
	AbortException() {}
	~AbortException() throw() {}
	virtual const char* what() const throw() { return "Tree search aborted by state.\n"; }
};

class BTree
{
  public:
	BTree(int probType, bool isIntProb);
    virtual ~BTree();

    // Exploring functions
    void explore();

	// Returns true if terminal or inserted into tree.  Returns false if pruned by dominance
	// or bounds.  I'm not sure if this is the best behaviour or not.  TODO
    bool processState(State *s, bool isRoot = false);
    void processHeuristicState(State *s);
    bool attemptHeuristicCompletion(State *s);
	void resetBest();

	// Set various options
    void setDebug(int level) { debug = level; }
    void setDomUsage(bool b) { useDom = b; }
    void setRetainStates(bool b) { retainStates = b; }
    void setFinalTestUsage(bool b) { useFinalTests = b; }
	void setFindAllSolns(bool b) {findAllSolns = b; }
	void setSaveNonTermStates(bool b) {saveNonTerm = b; }
    void setNodeLimit(int lim) { nodeLimit = lim; }
    void setTimeLimit(double lim) { timeLimit = lim * CLOCKS_PER_SEC; }
    void setFirstImprovStop(bool b) { stopAtFirstImprov = b; }
	void setOutputRate(int rate) { outputRate = rate; }
	void setStartTime(double time) { tStats->totalTime = time; tStats->timeToOpt = time;}
	void setGlobalLB(double lb) { globalLowerBound = lb; }
	void setGlobalUB(double ub) { globalUpperBound = ub; }
	void setStateComputesBounds(bool b) { stateComputesBounds = b; }
	void setTulipOutputFile(const char* filename, const char* instName, long seed, bool deep);

	// Getters
    State* getOptSolution() { return bestState; }
	double getGlobalLB() { return globalLowerBound; }
	double getGlobalUB() { return globalUpperBound; }
	const char* getName() { return name.c_str(); }
	int getNumExploredStates() { return tStats->statesExplored; }

    // Public printing functions
    void printOptSolution() const;
    void printTreeStats() const;

  protected:
    // Variables
    double globalLowerBound;
    double globalUpperBound;
	int problemType;
    State* bestState;
    bool keepExploring;
	bool isOptIntegral;
	double runTimeStart;
	int nextNodeID, nextEdgeID;
	int exploredStates;
	FILE* tulipOutputFile;

    // Parameters / Options
    int debug;
    bool useDom;
    bool retainStates;
    bool useFinalTests;
	bool findAllSolns;
	int outputRate;
	bool stateComputesBounds;

    // Termination options
    int nodeLimit; 
    double timeLimit;
    bool stopAtFirstImprov;
	bool saveNonTerm;

	string name;

    // For tracking search statistics
    TreeStats *tStats;

    // This is an unordered map that maps a dominance class ID to a specific 
    // dominance class, implemented as a list of states. The dominance class 
    // ID for a state should be chosen such that any two states that may 
    // potentially dominate each other should have the same ID. 
    dominance_class_map domClasses;

    // Implemented class functions that cannot be overridden by subclasses
    void exploreNextState();
    void processTerminalState(State *s, bool isTreeNode = true);
    void applyDominanceRules(State *s);

    bool stateIsPrunable(State *s);
    bool stateIsDominated(State *s);
    bool stateExceedsBounds(State *s);

	void saveBestState(State* s, bool isTreeNode);

    // Abstract class functions that must be implemented by subclasses
    virtual bool isEmpty() const = 0;
    virtual State* getNextState() = 0;
    virtual void saveStateForExploration(State *s) = 0;

    void printProgress(bool newIncumbent = false) const;

  private:
    // Nothing
};

#endif // BTREE_H

