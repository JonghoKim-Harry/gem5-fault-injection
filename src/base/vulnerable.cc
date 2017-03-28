/**
 *  This file is created by Jongho Kim.
 */
#include "base/vulnerable.hh"
#include "sim/core.hh"          // curTick()
#include "base/trace.hh"        // DPRINTF, class Named
#include "debug/FI.hh"

namespace
{
    std::vector<Vulnerable::FiInfo> remainingFi;
    unsigned int fi_count = 0;
    bool injectPipeReg = false;
} // anonymous namespace


/** This method is "final" method */
void
Vulnerable::registerFi(uint64_t time, unsigned int loc, std::function<void(const unsigned int)> method)
{
    Vulnerable::FiInfo& fi_info = *new Vulnerable::FiInfo(time, loc, this, method);
    DPRINTF(FI, "--- Fault-injection Registered ---\n");
    DPRINTF(FI, "     * id:          %u\n", fi_info.id);
    DPRINTF(FI, "     * time:        %lu\n", time);
    DPRINTF(FI, "     * loc:         %u\n", loc);
    DPRINTF(FI, "     * target type: %s\n", typeid(*this).name());
    remainingFi.push_back(fi_info);
}

/*
 * Constructor of class Vulnerable::FiInfo
 *
 * @_time:   Tich when to inject fault
 * @_loc:    Index of a bit flipped by fault injection
 * @_vul:    A hardware component where fault injection happens
 *
 * All information about a fault injection is stored in this constructor
 */
Vulnerable::FiInfo::FiInfo(uint64_t _time, unsigned int _loc, Vulnerable *_vul, std::function<void(const unsigned int)> _m)
    : injTime(_time), injLoc(_loc), target(_vul), method(_m)
{
    ++fi_count;
    id = fi_count;
}

/**
 *  Iterate registered fault injections.
 *
 *  If current tick is larger than registered injection-time, inject fault.
 *  If done, registered injections will be removed from the list 'remainingFi'
 */
void
Vulnerable::evaluate()
{
    for(std::vector<FiInfo>::iterator iter = remainingFi.begin(); iter != remainingFi.end();) {
        if(iter->injTime < curTick()) {
            iter->target->injectFault(iter->injLoc, iter->method);
            iter = remainingFi.erase(iter);
            if(remainingFi.size()==0)
                injectPipeReg=true;
       }
        else
            ++iter;
    }
}


bool
Vulnerable::checkInjectPipeReg()
{
    return injectPipeReg;
}