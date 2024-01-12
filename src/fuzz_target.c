#include "chemicalburn/settings.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  cb_simulation_settings_t simulation_settings;
  cb_video_settings_t video_settings;

  char* data_copy = malloc(size + 1);
  memcpy(data_copy, data, size);
  data_copy[size] = 0;

  cb_parse_settings(data_copy, &simulation_settings, &video_settings);

  free(data_copy);
  return 0;
}
