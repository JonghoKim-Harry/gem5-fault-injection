/**
 *  This file is created by Jongho Kim.
 */

#ifndef __SOFTERROR_HH__
#define __SOFTERROR_HH__

#include "sim/core.hh"
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

    /** Flags */
    extern bool injRegistered;
    extern bool injDone;

    /** Injection Infos: You have to register them */
    extern unsigned int injTime;
    extern unsigned int injLoc;
    extern InjComp injComp;
    extern unsigned int injWait;

    void registerInj(unsigned int time, unsigned int loc, InjComp comp, unsigned int wait_count=0);
    bool timeToInject();
    bool injReady();
}

#endif
