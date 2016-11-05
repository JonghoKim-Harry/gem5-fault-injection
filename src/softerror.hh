#ifndef __SOFTERROR_HH__
#define __SOFTERROR_HH__


#define BITFLIP(data, bit) (data ^ (1 << bit))

namespace SoftError
{
    typedef enum {
        NO_INJECTION = 0,
        F1TOF2,
        F2TOD,
        DTOE,
        ETOF1,
        F2TOF1,
        NUM_INJCOMP
    } InjComp;
}

#endif
