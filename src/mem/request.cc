#include "mem/request.hh"

#include <cstring>
#include <iostream>

#include "base/cprintf.hh"
#include "base/misc.hh"
#include "base/trace.hh"
#include "debug/FI.hh"

using namespace std;

void Request::flipPaddr(int injectLoc){
    if (!hasPaddr()){
        DPRINTF(FI, "LSQ FI : to flipPaddr of request : Unvalid Paddr\n");
        return;


    }
    else{
        uint32_t originalAddr = _paddr;
        uint32_t bit_flip = originalAddr ^ (1UL << (injectLoc%32));
        _paddr = bit_flip;
        DPRINTF(FI, "LSQ FI : Flipping physical address from %x to %x\n",
        originalAddr, _paddr);
    }
}
void Request::flipVaddr(int injectLoc){
    if (!hasPaddr()){
        DPRINTF(FI, "LSQ FI : to flipPaddr of request : Unvalid Vaddr\n");
        return;

    }
    else
    {
        uint32_t originalAddr = _vaddr;
        uint32_t bit_flip = originalAddr ^ (1UL << (injectLoc%32));
        _vaddr = bit_flip;
        DPRINTF(FI, "LSQ FI : Flipping virtual address from %x to %x\n",
        originalAddr, _vaddr);
    }

}

void Request::flipSetPaddr(int newPaddr){
    if (!hasPaddr()){
        DPRINTF(FI, "LSQ FI : to flipPaddr of request : Unvalid Paddr\n");
        return;


    }
    else{
        uint32_t originalAddr = _paddr;
                _paddr = newPaddr;
        DPRINTF(FI, "LSQ FI : (FlipSetPaddr) physical address from %x to %x\n",
        originalAddr, _paddr);
    }

}
