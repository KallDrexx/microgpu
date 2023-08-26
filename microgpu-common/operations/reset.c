#include "reset.h"

void mgpu_exec_reset(bool *resetFlag) {
    // We can't do anything from here, as a full reset depends
    // on the actual firmware implementation. So all we can do is set the
    // reset flag to true, and let the firmware specific reset
    // functionality run.
    *resetFlag = true;
}
