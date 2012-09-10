/*****************************************************************************/
/* Author: Jason Sauppe, David Morrison                                      */
/* Date: 2010-06-04                                                          */
/* File: state.h                                                             */
/* Description:                                                              */
/*   Contains the design details for an abstract state in a branching tree.  */
/*****************************************************************************/
#ifndef STATE_H
#define STATE_H

// Required include's
#include <cstdio>
#include <cstdlib>
#include <limits>

// Forward Declarations
class BTree;
const double inf = std::numeric_limits<double>::max();

class State
{
  public:
	friend class BTree;
    // Functions
	State() : 
		id(0),
		parID(-1),
		depth(0), 
		timeToExplore(0),
		dominanceClassID(1), 
		objValue(0.0), 
        lowerBound(-inf), 
		upperBound(inf), 
		//priority(-lowerBound), 
        dominated(false), 
		processed(false) 
	{}
    virtual ~State() {}; // Declaration and definition of destructor
	virtual State* clone() = 0; // "copy constructor"

    // Abstract class functions that must be implemented by subclasses
    virtual void branch(BTree *bt) = 0;
    virtual void computeBounds(BTree *bt) = 0;
    //virtual void computePriority() = 0;
    virtual void assessDominance(State *otherState) = 0;
    virtual bool isTerminalState() = 0;
    virtual void applyFinalPruningTests(BTree *bt) = 0;

    // Implemented class functions that can be overridden by subclasses
    virtual void print() const;
	virtual void writeTulipOutput(FILE* tulipOutputFile) const { return; }

    // Implemented class functions that cannot be overridden by subclasses
    int getDepth() const;
    int getDomClassID() const;
    double getObjValue() const;
    double getLB() const;
    double getUB() const;
	int getID() const { return id; }
	int getParentID() const { return parID; }
	void setID(int i) { id = i; }
    bool isDominated() const;
    bool wasProcessed() const;

	virtual bool operator<(const State& other) = 0;

  protected:
    // Variables
	int id, parID;
    int depth;
	int timeToExplore;
    int dominanceClassID;
    double objValue;
    double lowerBound;
    double upperBound;
    //double priority;
    bool dominated;
    bool processed;
};

struct StateComparator
{
	bool operator()(State* const& x, State* const& y) { return *x < *y; }
};

/*****************************************************************************/
/* State inline function definitions                                         */
/*****************************************************************************/
inline int State::getDepth() const 
{ 
    return depth; 
}

inline int State::getDomClassID() const 
{ 
    return dominanceClassID; 
}

inline double State::getObjValue() const 
{ 
    return objValue; 
}

inline double State::getLB() const 
{ 
    return lowerBound; 
}

inline double State::getUB() const 
{ 
    return upperBound; 
}

/*inline double State::getPriority() const 
{ 
    return priority; 
}*/

inline bool State::isDominated() const 
{ 
    return dominated; 
}

inline bool State::wasProcessed() const
{
    return processed;
}

inline void State::print() const 
{
    printf("D:=%4d, Obj:=%10.2f ", depth, objValue);
    if (lowerBound > -std::numeric_limits<double>::max()) {
        printf("< %10.2f |", lowerBound);
    } else {
        printf("< -INF |");
    }
    if (upperBound < std::numeric_limits<double>::max()) {
        printf(" %10.2f >\n", upperBound);
    } else {
        printf(" INF >\n");
    }
    return;
}

/*****************************************************************************/
/* Class that provides functionality for sorting states                      */
/*****************************************************************************/
/*class SortStatePriority
{
  public:
    // Returns true if s1 < s2 in the total ordering; since priority queue is 
    // a max priority queue, we want s1 < s2 when s2 has the higher priority
    bool operator() (const State *s1, const State *s2) const 
    {
        return (s1->getPriority() < s2->getPriority());
    }
};*/

#endif // STATE_H

