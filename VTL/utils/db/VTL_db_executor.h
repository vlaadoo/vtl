#ifndef _VTL_DB_EXECUTOR_H
#define _VTL_DB_EXECUTOR_H

#include <stdio.h>
#include <libpq-fe.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <VTL/utils/db/VTL_db_credentals.h>
#include <VTL/utils/db/VTL_db_connection.h>

PGresult* VTL_execute_query(char *query);


#ifdef __cplusplus
}
#endif


#endif