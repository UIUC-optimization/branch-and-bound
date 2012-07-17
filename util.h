// util.h: David R. Morrison, Mar. 2011
// Utility functions

#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <list>
#include <algorithm>
#include <exception>
#include <string>
#include <sstream>
#include <limits>
#include <utility>

using std::vector;

/*** Easy-to-implement custom printf behaviour -- inherit from this class to use ***/

namespace my
{
const int bufferSize = 2048;

class printer
{
public:
	virtual const char* str() const = 0;

protected:
	static char buffer[bufferSize];
};
};

/*** Constant types ***/
const double Tolerance = 0.00000001;
const double Infinity = std::numeric_limits<double>::max();
const int MaxInt = std::numeric_limits<int>::max();

/*** Error handling ***/
class Error : public std::exception
{
public:
	Error(const char* file, const int line) 
		{ _msg << "ERROR (" << file << ":" << line << "): "; }
	Error(const Error& e) { _msg << e._msg.str(); }
	virtual const char* what() const throw() { return (_msg.str() + "\n").c_str(); }
	~Error() throw () {}

	Error& operator<<(const char* s) { _msg << s; return *this; }
	Error& operator<<(const std::string s) { _msg << s; return *this; }
	Error& operator<<(const int i) { _msg << i; return *this; }
	Error& operator<<(const double f) { _msg << f; return *this; }
	Error& operator<<(const bool b) { _msg << (b ? "true" : "false"); return *this; }
	Error& operator<<(const void* p) { _msg << p; return *this; }

private:
	std::stringstream _msg;
};

#define ERROR Error(__FILE__, __LINE__)

/*** Miscellaneous utility functions ***/
template <typename T>
bool contains(std::vector<T>& vec, T el) { return find(vec.begin(), vec.end(), el) != vec.end(); }
template <typename T>
bool contains(std::list<T>& lis, T el) { return find(lis.begin(), lis.end(), el) != lis.end(); }

#endif // UTIL_H



