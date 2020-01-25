/**
 * Handles syncing device state with its AWS IOT shadow
 *
 * Device Shadow overview: https://mongoose-os.com/docs/mdash/shadow.md
 * JSON parser documentation: https://github.com/cesanta/frozen
 *
 * Refer to `ms_state.h` for the device state structure
 */
#include "common/cs_dbg.h"
#include "ms_state.h"

MS_STATE DeviceState = {
                        .readings = {},
                        .alerts = {},
                        .location = NULL,
                        .wait_for_shadow=0
						};

void init_state(void) {
    LOG(LL_INFO, ("INIT STATE"));

    // TODO: init/restore state (from nvflash or device shadow?)
}
