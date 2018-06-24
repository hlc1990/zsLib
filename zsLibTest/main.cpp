
#include "testing.h"

#include <zsLib/helpers.h>

int main (ZS_MAYBE_USED() int argc, ZS_MAYBE_USED() char * const argv[]) {
  ZS_MAYBE_USED(argc);
  ZS_MAYBE_USED(argv);

  Testing::runAllTests();
  Testing::output();

  if (0 != Testing::getGlobalFailedVar()) {
    return -1;
  }
  return 0;
}
