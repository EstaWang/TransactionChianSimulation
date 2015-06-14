#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "transaction.h"

struct t_chain_param
{
    int client_sockfd;
    Transaction* t;
};

struct hop_exe_param
{
    Hop* h;
    int clientfd;
};

#define SERV_1_PORT 7369
#define SERV_2_PORT 7370
#define SERV_3_PORT 7371

#define SERV_1_HOP_PORT 7372
#define SERV_2_HOP_PORT 7373
#define SERV_3_HOP_PORT 7374

#define BACKLOG 10          /*length of request queue*/
#define BUF_SIZE 1048576    /*the size of buffer area*/

#endif // _GLOBAL_H_
