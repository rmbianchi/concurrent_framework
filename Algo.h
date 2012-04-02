/**
 * Algo.h
 *
 *  Created on: Mar 8, 2012
 *      Author: rbianchi@cern.ch  (Riccardo-Maria BIANCHI)
 */

#ifndef ALGO_H_
#define ALGO_H_

// include internals
#include "Whiteboard.h"

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
	virtual void publishToWhiteboard() = 0;
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

	Whiteboard* m_wb;
	MyClass mc;

	/*
	 * methods
	 */
	Algo(Whiteboard* wb, const char* name) : m_name(name), m_wb(wb) {printf("Instantiating %s\n", m_name);};
	Algo(Whiteboard* wb, const char* name, std::string typeIN, std::string typeOUT) : m_name(name), dataTypeIN(typeIN), dataTypeOUT(typeOUT), m_wb(wb) {printf("Instantiating %s\n", m_name);};
	Algo(Whiteboard* wb, const char* name, T1 in, T2 out) : m_name(name), dataIN(in), dataOUT(out), m_wb(wb) {};
	virtual ~Algo() {};

	// actual implementations of the virtual methods
	void body() {printf("Algo '%s' - body() - IN: %s - OUT: %s\n", m_name, dataTypeIN.c_str(), dataTypeOUT.c_str()); sleep(2);
				 publishToWhiteboard();
				 printf("\n");
				};
	std::string getDataTypeIN() {return dataTypeIN;};
	std::string getDataTypeOUT() {return dataTypeOUT;};
	const char* getName() {return m_name;};
	void publishToWhiteboard() {printf("Algo '%s' - publishToWhiteboard()\n", m_name);
								m_wb->insert_into_table(dataTypeOUT, mc);
	                            };

};







#endif /* ALGO_H_ */
