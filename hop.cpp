#include "hop.h"

std::string Hop::get_server()
{
    return server;
}

std::string Hop::get_database()
{
	return database;
}

std::string Hop::get_table()
{
	return table;
}

std::string Hop::get_sql_statement()
{
	return sql_statement;
}

bool Hop::is_write()
{
    return write_type;
}

int Hop::get_sequence_num()
{
    return seq_num_in_transaction;
}

int Hop::get_transaction_num()
{
    return transaction_num;
}

table_access_type Hop::get_table_access_type()
{
    return table_access;
}

int Hop::get_counter()
{
    return counter;
}

void Hop::set_counter(int c)
{
    counter = c;
}

void Hop::set_commute(bool cu)
{
    commute = cu;
}

bool Hop::is_commute()
{
    return commute;
}
