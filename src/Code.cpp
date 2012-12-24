/**
 *  @file   Code.cpp
 *  @author Weston Nielson <wnielson@github>
 *
 */

#include "Code.h"
#include "record.h"

using namespace std;

Code::Code()
: m_last_value(-1)
{};

int Code::addValue(int value) {
  if (value == 0 && m_last_value == -1) {
    return 0;
  }
  
  Bit* bit = NULL;
  
  if (value != m_last_value) {
    bit = new Bit;
    
    bit->state = 0;
    bit->count = 0;
    
    m_bits.push_back(bit);
  } else {
    bit = m_bits.back();
  }
  
  if (bit != NULL) {
    bit->state = value;
    bit->count++;
  }
  
  m_last_value = value;
  
  return (int)m_bits.size();
};

bool Code::validate()
{
  double  ave[2]          = {0, 0};
  int     count[2]        = {1, 1};
  int     long_count[2]   = {1, 1};
  int     short_count[2]  = {1, 1};
  int     i               = 1;
  int     size            = (int)m_bits.size();
  
  if (size < MIN_CODE_LENGTH) {
    // Invalid code - not enough bits
    return false;
  }
  
  for (std::list<Code::Bit*>::iterator it=m_bits.begin(); it != m_bits.end(); ++it) {
    if (i < size) {
      
      if ((*it)->count < MIN_BIT_LENGTH) {
        // Invalid code - bit is too short
        return false;
      }
      
      ave[(*it)->state] += ((*it)->count-ave[(*it)->state])/count[(*it)->state];
      count[(*it)->state]++;
    }
    
    i++;
  }
  
  SIGNAL_BIT bit;   // Current bit
  SIGNAL_BIT pbit;  // Previous bit
  
  i = 0;
  for (std::list<Code::Bit*>::iterator it=m_bits.begin(); it != m_bits.end(); ++it) {
    int state = (*it)->state,
        count = (*it)->count;
    
    if (i != (size-1)) {      
      if (count > ave[state]) {
        m_long_ave[state] += (count-m_long_ave[state])/(long_count[state]);
        if (state == 0) {
          bit = CODE_LO_LONG;
        } else {
          bit = CODE_HI_LONG;
        }
      } else {
        m_short_ave[state] += (count-m_short_ave[state])/(short_count[state]);
        if (state == 0) {
          bit = CODE_LO_SHORT;
        } else {
          bit = CODE_HI_SHORT;
        }
      }
      
      if (i%2 == 1) {
        if ((bit + pbit) == 5) {
          m_code += "1";
        } else {
          m_code += "0";
        }
      }
    }
    
    pbit = bit;
    i++;
  }
  
  if (pbit == CODE_HI_SHORT) {
    m_code += "0";
  } else if (pbit == CODE_HI_LONG) {
    m_code += "1";
  }
  
  return true;
};

void Code::reset()
{
  // Reset the code string
  m_code = "";
  
  // Delete all saved Code instances
  for (list<Code::Bit*>::iterator it=m_bits.begin(); it != m_bits.end(); ++it) {
    delete (*it);
    m_bits.erase(it);
  }
  
  // Reset averages
  for (int i=0; i < 2; i++) {
    m_long_ave[i]   = 0;
    m_short_ave[i]  = 0;
  }
  
};

string Code::getCodeString() {
  return m_code;
};

int Code::getLength(int i)
{
  if (i < 4) {
    if (i < 2) {
      return m_long_ave[i];
    }
    return m_short_ave[i-2];
  }
  return 0;
};
