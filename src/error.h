/**
 *  @file   error.h
 *  @author Weston Nielson <wnielson@github>
 *
 *  Common error codes used throughout the code.
 *
 */

#ifndef rfswitch_error_h
#define rfswitch_error_h

#include <cstring>

enum RF_ERROR {
  RFE_NO_ERROR        = 0x0000,
  
  RFE_SHOW_HELP       = 0x1A00,
  RFE_INCORRECT_ARGS  = 0x1A01,
  RFE_INVALID_ARGS    = 0x1A02,
  
  RFE_GPIO_NO_ACCESS  = 0x2A01,
  RFE_INVALID_ID      = 0x4C01
};

inline const char* get_error_msg(RF_ERROR error) {
  const char *result;
  
  switch (error)
  {
    case RFE_NO_ERROR:        result = "Success"; break;
      
    case RFE_SHOW_HELP:       result = "Help"; break;
    case RFE_INCORRECT_ARGS:  result = "Incorrect number of arguments"; break;
    case RFE_INVALID_ARGS:    result = "Invalid argument"; break;

    case RFE_GPIO_NO_ACCESS:  result = "Unable to access GPIO"; break;

    case RFE_INVALID_ID:      result = "Invalid switch id"; break;
      
    default: result = "Invalid error code"; break;
  }
  
  return result;
};

#endif
