#include <stdio.h>
#include <math.h>
#include "f_util.h"
#include "ff.h"

inline FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
    auto minRead = fmin(btr, fp->size - fp->currentPosition);

    printf("f_read offset %u from %u read %u can read %u\n", fp->currentPosition, fp->size, btr, minRead);


    if (minRead == 0)
    {
        return FR_NOT_OK;
    }

    memcpy(buff, &fp->data[fp->currentPosition], minRead);
    const unsigned char* x = (const unsigned char*)buff;
    *br = minRead;
    fp->currentPosition += minRead;
    for (auto i = 0; i < fmin(btr, 10); i++)
    {
        printf("%u ", x[i]);
    }
    printf("\n");
    // std::cout << "f_read" << std::endl;
    return FR_OK;
}

inline FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode)
{
    printf("f_open %s %d %u %u\n", path, mode, sizeof(fp->data), fp->size);
    return FR_OK;
}

inline FRESULT f_lseek (FIL* fp, FSIZE_t ofs)
{
    printf("f_lseek %u\n", ofs);
    if (ofs < fp->size)
    {
        fp->currentPosition = ofs;
        return FR_OK;    
    }
    fp->currentPosition = 0;
    return FR_NOT_OK;
}

inline FRESULT f_close (FIL* fp)
{
    return FR_OK;
}

inline const char *FRESULT_str(FRESULT i)
{
    return "";
}
