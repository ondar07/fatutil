#ifndef _COMMAND_INTERFACE_H
#define _COMMAND_INTERFACE_H

#include <stdio.h>
#include "ftools.h"

typedef struct {
  // buffer
  char *command;
  char *arg;
} CMND;

CMND *cmnd_interf_init();
void cmnd_interf_deinit(CMND *cmnd);
int enter_command(CMND *cmnd);

#endif
