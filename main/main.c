#include "setup.h"

#ifdef __cplusplus
extern "C" {
#endif
void app_main(void) {
  setup();
  while (1) {
    loop();
  }
}
#ifdef __cplusplus
}
#endif