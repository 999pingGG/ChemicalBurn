#include <stdlib.h>


#ifdef 0

#endif

float cb_random_float_between(const float min, const float max) {
  const double range = max - min;
  return min + (float)((double)rand() / (RAND_MAX / range));
}

int cb_random_int_between(const int min, const int max) {
  const int range = max - min;
  return min + rand() % range;
}
