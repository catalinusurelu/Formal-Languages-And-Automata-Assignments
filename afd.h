#ifndef AFD_H
#define AFD_H

#include <iostream>
#include "afn.h"

class AFD : public AFN
{
public:
	AFD();
	
	bool contains(const string& str);
	
	friend std::ostream& operator<< (std::ostream &out, AFD &afd);
};


#endif