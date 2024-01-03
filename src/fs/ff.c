#include <stdio.h>
#include "f_util.h"
#include "ff.h"

inline FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
    printf("f_read %u", btr);
    // std::cout << "f_read" << std::endl;
    return FR_NOT_OK;
}

inline FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode)
{
    printf("f_open %s &d", path, mode);
    return FR_OK;
}

inline FRESULT f_lseek (FIL* fp, FSIZE_t ofs)
{
    return FR_OK;
}

inline FRESULT f_close (FIL* fp)
{
    return FR_OK;
}

inline const char *FRESULT_str(FRESULT i)
{
    return "";
}