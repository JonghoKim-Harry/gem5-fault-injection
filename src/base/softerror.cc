#include "base/softerror.hh"
#include "cpu/minor/dyn_inst.hh"

namespace SoftError
{
    /** Flags: All default values are "FALSE" */
    bool injRegistered = false;
    bool injDone = false;
    bool faulty_inst_id_tracked = false;
    bool faulty_inst_id_logged = false;

    /** Injection Info */
    unsigned int injTime;
    unsigned int injLoc;
    InjComp injComp;
    unsigned int injWait;
    Minor::InstId faulty_inst_id;

    /** (Default value of wait_count) = 0 */
    void registerInj(unsigned int time, unsigned int loc, InjComp comp, unsigned int wait_count)
    {
        injRegistered = true;
        injTime = time;
        injLoc = loc;
        injComp = comp;
        injWait = wait_count;
    }

    bool timeToInject() { return injRegistered && (!injDone) && curTick() >= injTime; }
    bool injReady() { return timeToInject() && (injWait == 0); }
} // namespace SoftError

