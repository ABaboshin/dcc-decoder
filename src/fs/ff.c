#include <stdio.h>
#include <math.h>
#include "f_util.h"
#include "ff.h"

inline FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
    printf("f_read %u\n", btr);

    auto minRead = fmin(btr, fp->size - fp->currentPosition);

    printf("f_read %u\n", minRead);

    if (minRead == 0)
    {
        return FR_NOT_OK;
    }

    memcpy((const unsigned char*)buff, fp->data + fp->currentPosition, minRead);
    *br = minRead;
    fp->currentPosition += minRead;
    for (auto i = 0; i < fmin(btr, 10); i++)
    {
        printf("%u ", fp->data[i]);
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
    fp->currentPosition = 0;
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