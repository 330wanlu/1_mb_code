#ifndef FPXX_JSON_H
#define FPXX_JSON_H
#include "common.h"

int Json2FPXX(char *json_data, HFPXX fpxx);
int FPXX2Json(HFPXX fpxx, char **stp_fpxx_json);
int fpxx_to_json_rednum_data_v102(HFPXX stp_fpxx, uint8 **fpxx_json, int sfxqqd);
#endif