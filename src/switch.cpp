/**
 *  @file   switch.cpp
 *  @author Weston Nielson <wnielson@github>
 *
 *  Based on the work by Geoff Johnson.
 *
 */
#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE         (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include "switch.h"
#include "error.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <list>
#include <string>

using namespace std;

#define PAGE_SIZE   (4*1024)
#define BLOCK_SIZE  (4*1024)

int           mem_fd;
unsigned char *gpio_mem, *gpio_map;
char          *spi0_mem, *spi0_map;

// I/O access
volatile unsigned *gpio;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

// Define which GPIO pin the RF transmitter is connected to
#define PIN 7

struct CodeData {
  int   id;
  char  codes[2][255];
  int   values[2][5];
};

int load_codes(const char* path, list<CodeData>& codes)
{
  int count = 0;
  int line  = 1;
  FILE* fh  = fopen(path, "r");
  
  if (fh)
  {
    while (!feof(fh)) {
      int rc;
      
      CodeData cd;
      
      // Get the code ID
      rc = fscanf(fh, "%d\n", &cd.id);
      if (rc != 1)
      {
        printf("Invalid config file, line %d\n", line);
        break;
      }
      line++;
      
      for (int i=0; i < 2; i++) {
        rc = fscanf(fh, "%[10],%d,%d,%d,%d,%d\n",
                    cd.codes[i],      &cd.values[i][0],
                    &cd.values[i][1], &cd.values[i][2],
                    &cd.values[i][3], &cd.values[i][4]);
        
        if (rc != 6)
        {
          if (!feof(fh)) {
            printf("Invalid config file, line %d\n", line);
          }
          break;
        }
        line++;
      }
      
      codes.push_back(cd);
    }
    count = (int)codes.size();
  }
  
  return count;
};

/*
 *  Sends the code to the RF transmitter connected to GPIO PIN.
 *
 */
void send_code(CodeData* cd, string& action) {
  int   a     = (action == "on") ? 0 : 1;
  char* code  = cd->codes[a];
  
	timespec sleeptime;
	timespec remtime;
  
	for (int r = 0; r < 10; r++) {
    
		sleeptime.tv_sec  = 0;
    
		for (int i = 0 ; i < strlen(code); i++)
    {
			if (code[i] == '1')
      {
        // Send long-hi...
        sleeptime.tv_sec  = 0;
        sleeptime.tv_nsec = cd->values[a][2];
				GPIO_SET = 1<<7;
        nanosleep(&sleeptime, &remtime);
        
        // ... and short-lo
        sleeptime.tv_sec  = 0;
        sleeptime.tv_nsec = cd->values[a][3];
        GPIO_CLR = 1<<7;
        nanosleep(&sleeptime, &remtime);
			}
      
      else
      {
        // Send short-hi...
        sleeptime.tv_sec  = 0;
        sleeptime.tv_nsec = cd->values[a][0];
				GPIO_SET = 1<<7;
        nanosleep(&sleeptime, &remtime);
        
        // ... and long-lo
        sleeptime.tv_sec  = 0;
        sleeptime.tv_nsec = cd->values[a][1];
        GPIO_CLR = 1<<7;
        nanosleep(&sleeptime, &remtime);
			}
		}
    
    sleeptime.tv_sec  = 0;
		sleeptime.tv_nsec = cd->values[a][4];
		nanosleep(&sleeptime, &remtime);
	}
};

/**
 *  Setup access to the GPIO pins.
 *
 */
RF_ERROR setup_io() {
	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("Error: Can't open /dev/mem \n");
		return RFE_GPIO_NO_ACCESS;
	}
  
	/* mmap GPIO */
	// Allocate MAP block
	if ((gpio_mem = (unsigned char*)malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
		printf("Error: Allocation error \n");
		return RFE_GPIO_NO_ACCESS;
	}
  
	// Make sure pointer is on 4K boundary
	if ((unsigned long)gpio_mem % PAGE_SIZE) {
		gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);
	}
  
	// Now map it
	gpio_map = (unsigned char *)mmap(
                                   (caddr_t)gpio_mem,
                                   BLOCK_SIZE,
                                   PROT_READ|PROT_WRITE,
                                   MAP_SHARED|MAP_FIXED,
                                   mem_fd,
                                   GPIO_BASE
                                   );
  
	if ((long)gpio_map < 0) {
		printf("Error: mmap error %u\n", *gpio_map);
		return RFE_GPIO_NO_ACCESS;
	}
  
	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;
  
  return RFE_NO_ERROR;
};

int run_switch(int argc, char** argv)
{
  int     id          = -1;
  bool    list_codes  = false;
  string  config,
  action;
  bool    done = false;
  char    c;
  
  while (((c = getopt(argc, argv, "hlc:")) != -1) || done)
  {
    switch (c)
    {
      case 'h':
        return RFE_SHOW_HELP;
        break;
      case 'l':
        list_codes = true;
        break;
      case 'c':
        config = optarg;
        break;
      case 255:
        done = true;
        break;
      default:
        printf("C: %d\n",c);
    }
    
    if (done) {
      break;
    }
  }
  
  if (config.empty()) {
    config  = getenv("HOME");
    config += "/.rfswitch";
  }
  
  if (access(config.c_str(), R_OK) != 0) {
    config = "";
  }
  
  if (config.empty()) {
    printf("Could not find config file\n");
    return RFE_INCORRECT_ARGS;
  }
  
  list<CodeData> codes;
  load_codes(config.c_str(), codes);
  
  if (list_codes)
  {
    printf("Found %d codes\n", (int)codes.size());
    printf("  ---------------------------------------------------------------\n");
    for (list<CodeData>::iterator it=codes.begin(); it != codes.end(); it++) {
      printf("  id:  %d\n", (*it).id);
      printf("  on:  %s,%d,%d,%d,%d,%d\n",
             (*it).codes[0], (*it).values[0][0], (*it).values[0][1],
             (*it).values[0][2], (*it).values[0][3], (*it).values[0][4]);
      printf("  off: %s,%d,%d,%d,%d,%d\n",
             (*it).codes[1], (*it).values[1][0], (*it).values[1][1],
             (*it).values[1][2], (*it).values[1][3], (*it).values[1][4]);
      printf("  ---------------------------------------------------------------\n");
    }
    
  }
  
  else
  {
    
    // Parse arguments
    int i = 0;
    for (int index=optind; index < argc; index++) {
      if (i == 0) {
        id = atoi(argv[index]);
      } else if (i == 1) {
        action = argv[index];
      }
      i++;
    }
    
    printf("action: %s\n", action.c_str());
    
    if (action.empty() || id < 0) {
      return RFE_INCORRECT_ARGS;
    }
    
    if  (action != "on" && action != "off") {
      return RFE_INVALID_ARGS;
    }
    
    CodeData* cd = NULL;
    
    // Try to find the code
    for (list<CodeData>::iterator it = codes.begin(); it != codes.end() ; it++)
    {
      if ((*it).id == id) {
        cd = &(*it);
      }
    }
    
    // Have to have a code to continue
    if (cd == NULL) {
      return RFE_INVALID_ID;
    }
    
    // Set up gpi pointer for direct register access
    int rc = setup_io();
    if (rc != RFE_NO_ERROR) {
      return rc;
    }
    
    // Switch GPIO PIN to output mode
    INP_GPIO(PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN);
    
    // Now we can finally send the code
    send_code(cd, action);
    
  }
  
  return RFE_NO_ERROR;
};