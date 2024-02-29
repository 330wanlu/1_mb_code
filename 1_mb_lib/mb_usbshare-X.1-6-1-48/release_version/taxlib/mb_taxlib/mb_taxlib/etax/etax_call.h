#ifndef ETAX_CALL_H
#define ETAX_CALL_H

#include "etax_common.h"

int my_etax_confirm_query(HDEV hDev, ETaxFlag flag);
void my_etax_auth_server(HDEV hDev);
int my_etax_query_invoice(HDEV hDev, ETaxFlag flag);
#endif