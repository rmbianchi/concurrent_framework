/**
 * Algo.h
 *
 *  Created on: Mar 8, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 */

#ifndef ALGO_H_
#define ALGO_H_

// include c++
# include <iostream>
# include <cstdio>


/**
 * Algorithm virtual base class
 */
class AlgoBase {
public:

	virtual void body() = 0;
	virtual std::string getDataTypeIN() = 0;
	virtual std::string getDataTypeOUT() = 0;
	virtual const char* getName() = 0;
};


/**
 * Algorithm template class
 */
template <class T1, class T2>
class Algo : public AlgoBase {
public:

	/*
	 * variables
	 */
	const char* m_name;
	T1 dataIN;
	T2 dataOUT;
	std::string dataTypeIN;
	std::string dataTypeOUT;

	/*
	 * methods
	 */
	Algo(const char* name) : m_name(name) {printf("Instantiating %s\n", m_name);};
	Algo(const char* name, std::string typeIN, std::string typeOUT) : m_name(name), dataTypeIN(typeIN), dataTypeOUT(typeOUT) {printf("Instantiating %s\n", m_name);};
	Algo(const char* name, T1 in, T2 out) : m_name(name), dataIN(in), dataOUT(out) {};
	virtual ~Algo() {};

	// actual implementations of the virtual methods
	void body() {printf("body() of Algo '%s' - IN: %s - OUT: %s\n", m_name, dataTypeIN.c_str(), dataTypeOUT.c_str()); sleep(2);};
	std::string getDataTypeIN() {return dataTypeIN;};
	std::string getDataTypeOUT() {return dataTypeOUT;};
	const char* getName() {return m_name;};
};



#endif /* ALGO_H_ */
