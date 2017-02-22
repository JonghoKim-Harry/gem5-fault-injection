/**
 *  This file is created by Jongho Kim.
 */
#include "base/vulnerable.hh"
#include "sim/core.hh"          // curTick()
#include "base/trace.hh"        // DPRINTF, class Named
#include "debug/FIReport.hh"

namespace
{
    std::vector<Vulnerable::FiInfo> remainingFi;
    unsigned int fi_count = 0;
} // anonymous namespace


/** This method is "final" method */
void
Vulnerable::registerFi(unsigned int time, unsigned int loc, std::function<void(const unsigned int)> method)
{
    Vulnerable::FiInfo& fi_info = *new Vulnerable::FiInfo(time, loc, this, method);
    DPRINTF(FIReport, "--- Fault-injection Registered ---\n");
    DPRINTF(FIReport, "     * id:          %u\n", fi_info.id);
    DPRINTF(FIReport, "     * time:        %u\n", time);
    DPRINTF(FIReport, "     * loc:         %u\n", loc);
    DPRINTF(FIReport, "     * target type: %s\n", typeid(*this).name());
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
Vulnerable::FiInfo::FiInfo(unsigned int _time, unsigned int _loc, Vulnerable *_vul, std::function<void(const unsigned int)> _m)
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
       }
        else
            ++iter;
    }
}
