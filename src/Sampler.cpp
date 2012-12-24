/**
 *  @file   Sampler.cpp
 *  @author Weston Nielson <wnielson@github>
 *
 */

#include "Sampler.h"
#include "record.h"

#include <cstdio>
#include <string>

using namespace std;

Sampler::Sampler()
: m_code(NULL), m_zeroes(0), m_mode(MODE_COUNT_ZEROES)
{};

bool Sampler::sample(float* buffer, int length)
{  
  for (int j=0; j < length; j++)
  {
    // Convert the analog signal into binary
    int value = this->get_binary(buffer[j]);
    
    // Keep track on consecutive zeroes
    if (value == 1) {
      m_zeroes = 0;
    } else {
      m_zeroes++;
    }
    
    if (m_zeroes >= ZERO_PREAMBLE_THRESH)
    {
      m_mode = MODE_WAIT_HI;
      if (m_code != NULL)
      {
        if (m_code->validate())
        {
          
          fprintf(stdout, ".");
          fflush(stdout);
          
          string code_str = m_code->getCodeString();
          m_codes[code_str].push_back(m_code);
          
          for (code_list_map_it it = m_codes.begin(); it != m_codes.end(); it++)
          {
            int count = (int)(*it).second.size();
            
            if (count > CODE_COUNT) {
              // We've found the code
              printf("\nFound code\n");
              printf("  code:     %s\n", code_str.c_str());
              
              if (this->process_codes((*it).second)) {
                return true;
              }
              
              printf("Error processing the code\n");
              return false;
            }
          }
          
        } else {
          // Invalid code
          delete m_code;
          m_code = NULL;
        }
        
      }
      
      m_zeroes = 0;
    }
    
    if (m_mode == MODE_WAIT_HI) {
      if (value == 1) {
        m_mode = MODE_READ_CODE;
        m_code = new Code;
      }
    }
    
    if (m_mode == MODE_READ_CODE) {
      if (m_code == NULL) {
        m_code = new Code;
      }
      m_code->addValue(value);
    }
    
  }
  
  return false;
  
};

void Sampler::rewind()
{
  if (m_code != NULL) {
    delete m_code;
    m_code = NULL;
  }
};

int Sampler::get_binary(float value)
{
  // Convert analog signal to digital
  if (value <= SIGNAL_THRESH) {
    value = 0;
    //zeroes++;
  } else {
    value  = 1;
    //zeroes = 0;
  }
  
  return value;
};


bool Sampler::process_codes(list<Code*>& codes)
{
  int lo_long   = 0,
      lo_short  = 0,
      hi_long   = 0,
      hi_short  = 0,
      size      = (int)codes.size();
  
  for (list<Code*>::iterator it=codes.begin(); it != codes.end(); it++)
  {
    lo_long   += (*it)->getLength(0);
    hi_long   += (*it)->getLength(1);
    lo_short  += (*it)->getLength(2);
    hi_short  += (*it)->getLength(3);
  }
  
  hi_long   /=  size;
  hi_short  /=  size;
  lo_long   /=  size;
  lo_short  /=  size;
  
  printf("  hi-long:  %d\n  hi-short: %d\n", hi_long, hi_short);
  printf("  lo-long:  %d\n  lo-short: %d\n", lo_long, lo_short);
  printf("  timings:  %d,%d,%d,%d\n", (int)(hi_short/(float)SAMPLE_RATE*1e9), (int)(lo_long/(float)SAMPLE_RATE*1e9),
                                      (int)(hi_long/(float)SAMPLE_RATE*1e9), (int)(lo_short/(float)SAMPLE_RATE*1e9));

  return true;
};

