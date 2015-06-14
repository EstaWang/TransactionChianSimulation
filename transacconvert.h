#ifndef _TRANSACCONVERT_H_
#define _TRANSACCONVERT_H_

#include <string>
#include "transaction.h"

class TransacConvert
{
public:
    static std::string transaction_to_string(Transaction* t);
    static Transaction* string_to_transaction(std::string t_str);
    static std::string hop_to_string(Hop* h);
    static Hop* string_to_hop(std::string h_str);
};

#endif // _TRANSACCONVERT_H_
