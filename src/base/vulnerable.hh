/**
 *  This file is created by Jongho Kim.
 */

#ifndef __BASE_VULNERABLE_HH__
#define __BASE_VULNERABLE_HH__

#include <iostream>
#include <vector>
#define BITFLIP(data, bit) (data ^ (1 << (bit)))
#define BIT_PER_BYTE 8

/**
 *  This file defines two classes:
 *   - Vulnerable
 *   - VulnerableData
 *
 *  Note that any instance of class Vulnerable is
 *  a vulnerable hardware component
 *
 *  Note that the two classes have no relationship like inheritance, etc.
 *  It is because fault injection into data is different from
 *  fault injection into hardware component, although they are related
 */


class Vulnerable
{
  public:

    /**
     *  We only need to inject fault into vulnerable hardware components,
     *  not into vulnerable data.
     *  Once we inject fault into all hardware components, it will result in
     *  corruption of data
     */
    static void evaluate();

    /**
     *  Registration of fault injection
     *
     *  @time: Tick when to inject fault into hardware component
     *  @loc: Index of the bit flipped by fault injection
     *
     *  You should NOT modify or override this method.
     *  We've already prevented it by using the keyword "final"
     */
    virtual void registerFi(unsigned int time, unsigned int loc) final;
 
    /**
     *  Your own implementation of fault injection into a hardware component
     *
     *  @loc: Index of the bit flipped by fault injection
     *
     *  You should override this method, and make this method to trigger
     *  fault injection into an instance of VulnerableData.
     *  Note that this method is called at the end of registerFi(),
     *  so you just register fault injections with registerFi() and it's done
     */
    virtual void injectFault(const unsigned int loc) = 0;
 
    class FiInfo {
      public:
        friend class Vulnerable;
        FiInfo(unsigned int _t, unsigned int _l, Vulnerable *_v);
        unsigned int injTime;
        unsigned int injLoc;
        Vulnerable *target;

      protected:
        unsigned int id;
    }; // class Vulnerable::FiInfo

}; // class Vulnerable


class VulnerableData
{
  public:
    /**
     *  Your own implementation of fault injection into data
     *
     *  @loc: Index of the bit flipped by fault injection
     *
     *  You should override this method.
     */
    virtual void injectFault(const unsigned int loc) = 0;
}; // class VulnerableData

#endif // __BASE_VULNERABLE_HH__
