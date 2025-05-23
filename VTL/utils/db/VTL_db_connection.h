#ifndef _VTL_DB_CONNECTION_H
#define _VTL_DB_CONNECTION_H

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <VTL/utils/db/VTL_db_credentals.h>
#include <libpq-fe.h>

PGconn* VTL_connect_to_db(VTL_db_Credentals* p_credentals);


#ifdef __cplusplus
}
#endif


#endif