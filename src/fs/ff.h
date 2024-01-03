#pragma once

#include "f_util.h"

typedef struct FIL 
{
    // FILE* fs;
    void* dummy;
} FIL;

FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);
FRESULT f_lseek (FIL* fp, FSIZE_t ofs);
FRESULT f_close (FIL* fp);
const char *FRESULT_str(FRESULT i);