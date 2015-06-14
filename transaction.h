#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include <list>
#include <string>

#include "hop.h"

class Transaction
{
public:
	Transaction() {	}

	Transaction(int seq)
	{
		tranction_number = seq;
	}

	int get_transaction_number();

	std::list<Hop> hops;      /*the list of hops*/

private:
	int tranction_number;             /*the sequence number of the transaction*/
};

#endif // _TRANSACTION_H_
