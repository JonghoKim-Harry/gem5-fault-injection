/**
 *  This file is created by Jongho Kim.
 */
#include "base/vulnerable.hh"
#include "sim/core.hh"          // curTick()
#include "debug/FIReport.hh"

namespace
{
    std::vector<Vulnerable::FiInfo> remainingFi;
} // anonymous namespace


/** This method is "final" method */
void
Vulnerable::registerFi(unsigned int time, unsigned int loc)
{
    Vulnerable::FiInfo fi_info = *new Vulnerable::FiInfo(time, loc, this);
    remainingFi.push_back(fi_info);
}
 
Vulnerable::FiInfo::FiInfo(unsigned int _t, unsigned int _l, Vulnerable *_v)
{
    injTime = _t;
    injLoc  = _l;
    target = _v;
}

/**
 *  Iterate registered fault injections. If current tick is larger than
 *  registered injection-time, inject fault. If done, registered injections
 *  will be from the list 'remainingFi'
 */
void
Vulnerable::evaluate()
{
    for(std::vector<FiInfo>::iterator iter = remainingFi.begin(); iter != remainingFi.end();) {
        if(iter->injTime < curTick()) {
            iter->target->injectFault(iter->injLoc);
            iter = remainingFi.erase(iter);
       }
        else
            ++iter;
    }
}
