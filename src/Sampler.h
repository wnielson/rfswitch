/**
 *  @file   Sampler.h
 *  @class  Sampler
 *  @author Weston Nielson <wnielson@github>
 *
 *  This class contains the functionality for parsing
 *  a stream of RF data and finding occurrences of repeating
 *  switch control codes.
 *
 */

#ifndef __rfswitch__Sampler__
#define __rfswitch__Sampler__

#include <portaudio.h>
#include "Code.h"

#include <map>
#include <list>
#include <string>

using namespace std;

typedef map<string, list<Code*> >           code_list_map;
typedef map<string, list<Code*> >::iterator code_list_map_it;

class Sampler {
  public:
    Sampler();
    bool  sample(float* buffer, int length);
    void  rewind();
  
    enum  MODE {
      MODE_COUNT_ZEROES,  // Count zeroes until ZERO_PREAMBLE_THRESH is reached
      MODE_WAIT_HI,       // Once ZERO_PREAMBLE_THRESH is reached, this will wait for a `1`
      MODE_READ_CODE
    };
  
  private:
    int   get_binary(float value);
    bool  process_codes(list<Code*>& codes);

    int             m_zeroes;
    Sampler::MODE   m_mode;
  
    Code*           m_code;
    code_list_map   m_codes;
};

#endif /* defined(__rfswitch__Sampler__) */
