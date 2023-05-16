#include "common.h"

void usage (char *program_name, const char *usage_format)
{
  fprintf (stdout, usage_format, basename (program_name));
}

