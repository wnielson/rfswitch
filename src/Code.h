/**
 *  @file   Code.h
 *  @class  Code
 *  @author Weston Nielson <wnielson@github>
 *
 *  This class represents a code used to either switch
 *  on or off an RF switch. The actual code is represented
 *  by a string, of 1s and 0s, which is constructed by
 *  analyzing the RF signal.  A '0' is represented by a
 *  ``hi-long`` pulse folled by a ``lo-short`` pulse, while
 *  a '1' is represented bi a ``hi-short`` followed by a
 *  ``lo-long``.  A truncated example signal is shown below.
 *
 *
 *        0         1         0       0         code
 *    ├────────┼───────────┼────────┼────────┤
 * hi ┌──┐     ┌──────┐    ┌──┐     ┌──┐
 *    │  │     │      │    │  │     │  │
 *    │  │     │      │    │  │     │  │
 *    │  │     │      │    │  │     │  │
 * 0 ─│──│─────│──────│────│──│─────│──│───────
 *    │  │     │      │    │  │     │  │
 * lo ┘  └─────┘      └────┘  └─────┘  └─────┘
 *      2   5     6     3    2   5    2   5     length
 *
 *
 *  In this example signal the lengths of the individual
 *  signals are:
 *
 *    hi-long   6
 *    hi-short  2
 *    lo-long   5
 *    lo-short  3
 *
 */

#ifndef __rfswitch__Code__
#define __rfswitch__Code__

#include <list>
#include <string>

using namespace std;

enum SIGNAL_BIT {
  CODE_HI_LONG  = 1,
  CODE_HI_SHORT = 5,
  CODE_LO_LONG  = 2,
  CODE_LO_SHORT = 4,
};

class Code {
  public:
    Code();
  
    int         addValue(int value);
    bool        validate();
    void        reset();
    inline int  getLength() { return (int)m_bits.size(); };
    string      getCodeString();
    int         getLength(int i);
  
    struct Bit {
      int count;
      int state;
    };
  private:
    int         m_last_value;
    list<Bit*>  m_bits;
    string      m_code;
    int         m_long_ave[2];
    int         m_short_ave[2];
};

#endif /* defined(__rfswitch__Code__) */
