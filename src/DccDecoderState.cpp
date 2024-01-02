#include "DccDecoderState.h"

void DccDecoderState::Reset ()
{
    Status = DccStatus::WAIT_PREAMBLE;
    PreambleOneBitCount = 0;
    DataCount = 0;
    Data.clear();
}