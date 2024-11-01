#include "lpstring.h"

int lp_atoi(const char *buffer, int start, int end)
{
  int result = 0;
  int sign = 1;

  // Check for a negative sign at the start position
  if (buffer[start] == '-')
  {
    sign = -1;
    start++;
  }

  for (int i = start; i <= end; i++)
  {
    if (!isdigit(buffer[i]))
    {
      break; // Stop if a non-digit character is encountered
    }
    result = result * 10 + (buffer[i] - '0');
  }

  return result * sign;
}