/**
 *  @file   main.cpp
 *  @author Weston Nielson <wnielson@github>
 *
 */

#include "config.h"
#include "switch.h"
#include "error.h"

#ifdef HAVE_PORTAUDIO_H
#include "record.h"
#endif 

#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 *  Prints out program usage to stdout.
 */
void print_usage() {
  printf("rfswitch version %s, Weston Nielson <wnielson@github>.\n\n", PACKAGE_VERSION);
  printf("Utility for controlling RF power sockets from the Raspberry Pi.\n");
  printf("Based on code originially by Geoff Johnson.\n\n");
  printf("Usage:\n\n");
  printf("  rfswitch s(witch) [options] <id> <action> : Turn switch on/off\n");
  
#ifdef HAVE_PORTAUDIO_H
  printf("  rfswitch r(ecord)                         : Record signal and extract code\n");
#endif
  
  printf("\nValid choices for 'action' are 'on' or 'off' and 'id' should be a\n");
  printf("valid switch id listed in the config file.\n\n");
  printf("Options:\n\n");
  printf(" -c<path> : Path to config file. (Defaults to $HOME/.rfswitch)\n");
  printf(" -l       : List available switches and exit.\n");
  printf(" -h       : Display this help text and exit.\n\n");
};

int quit(int code, bool show_usage) {
  if (show_usage) {
    print_usage();
  }
  
  exit(code);
};

int main(int argc, char **argv) {
  int rc = 0;
  
  if (argc < 2) {
    quit(RFE_INCORRECT_ARGS, true);
  }
  
  if (strcmp(argv[1], "s") == 0 || strcmp(argv[1], "switch") == 0)
  {
    rc = run_switch(argc-1, argv+1);
  }
  
#ifdef HAVE_PORTAUDIO_H
  else if (strcmp(argv[1], "r") == 0 || strcmp(argv[1], "record") == 0)
  {
    rc = run_record(argc-1, argv+1);
  }
#endif
  
  else {
    quit(RFE_INCORRECT_ARGS, true);
  }
  
  if (rc == RFE_SHOW_HELP) {
    quit(rc, true);
  }
  
  else {
    if (rc != RFE_NO_ERROR) {
      printf("Error: %s\n", get_error_msg((RF_ERROR)rc));
      if ((RFE_SHOW_HELP & rc) == RFE_SHOW_HELP) {
        quit(rc, true);
      }
    }
  }
  
  return rc;
};
