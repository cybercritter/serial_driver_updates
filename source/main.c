#include "serial_driver.h"
#include "x17v358_regs.h"
#include <stdint.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  printf ("size of x17v358_channel_regs_t: %zu\n",
          sizeof (x17v358_channel_regs_t));
  return 0;
}
