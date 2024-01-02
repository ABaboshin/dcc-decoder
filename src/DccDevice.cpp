#include "DccDevice.h"
#include "DccDecoder.h"

void DccDevice::SetDecoder (const DccDecoder& decoder)
{
    this->decoder = std::make_shared<DccDecoder>(decoder);
}