#include "rfid.h"
#include "utils.h"
#include "rfal_rf.h"
#include "rfal_nfca.h"
#include "rfal_nfcb.h"
#include "rfal_nfcf.h"
#include "rfal_nfcv.h"
#include "rfal_st25tb.h"
#include "rfal_nfcDep.h"
#include "rfal_isoDep.h"
#include "st_types.h"

#define ST_FIELD_OFF        0
#define ST_POLL_ACTIVE_TECH 1
#define ST_POLL_PASSIV_TECH 2
#define ST_WAIT_WAKEUP      3

#define NEXT_STATE()                 \
    {                                \
        state++;                     \
        state %= sizeof(stateArray); \
    }

static uint32_t stateArray[] = {
    ST_FIELD_OFF,
    ST_POLL_ACTIVE_TECH,
    ST_POLL_PASSIV_TECH,
    ST_WAIT_WAKEUP,
};

__ALIGNED(4)
union rfid_card_uid rfid_card_uid = {0};

static uint32_t doWakeUp = true;
static uint32_t state = ST_FIELD_OFF;

static uint32_t poll_NFCA(void);

uint32_t rfid_cycle(void)
{
    uint32_t found = 0;

    switch (stateArray[state]) {
    case ST_FIELD_OFF:
        rfalFieldOff();
        // rfalWakeUpModeStop();
        // platformDelay(300);

        /* If WakeUp is to be executed, enable Wake-Up mode */
        if (doWakeUp) {
            rfalWakeUpModeStart(NULL);
            state = ST_WAIT_WAKEUP;
            break;
        }

        NEXT_STATE();
        break;

    case ST_POLL_ACTIVE_TECH:
        // demoPollAP2P();
        // platformDelay(40);
        NEXT_STATE();
        break;

    case ST_POLL_PASSIV_TECH:
        found = poll_NFCA();
        state = ST_FIELD_OFF;
        // platformDelay(300);
        break;

    case ST_WAIT_WAKEUP:

        /* Check if Wake-Up Mode has been awaked */
        if (rfalWakeUpModeHasWoke()) {
            /* If awake, go directly to Poll */
            rfalWakeUpModeStop();
            state = ST_POLL_ACTIVE_TECH;
        }
        break;

    default:
        break;
    }
    return found;
}

uint32_t poll_NFCA(void)
{
    ReturnCode err;
    bool found = false;
    uint8_t devIt = 0;
    rfalNfcaSensRes sensRes;

    rfalNfcaPollerInitialize(); /* Initialize for NFC-A */
    rfalFieldOnAndStartGT();    /* Turns the Field On if not already and start GT timer */

    err = rfalNfcaPollerTechnologyDetection(RFAL_COMPLIANCE_MODE_NFC, &sensRes);
    if (err == ERR_NONE) {
        rfalNfcaListenDevice nfcaDevList[1];
        uint8_t devCnt;

        err = rfalNfcaPollerFullCollisionResolution(RFAL_COMPLIANCE_MODE_NFC, 1, nfcaDevList, &devCnt);

        if ((err == ERR_NONE) && (devCnt > 0)) {
            found = true;
            devIt = 0;

            /* Check if it is Topaz aka T1T */
            if (nfcaDevList[devIt].type == RFAL_NFCA_T1T) {
            } else {
                /*********************************************/
                /* NFC-A device found                        */
                /* NFCID/UID is contained in: nfcaDev.nfcId1 */
                rfid_card_uid.raw[1] = 0;
                rfid_card_uid.len = nfcaDevList[0].nfcId1Len;
                for (uint32_t i = 0; i < nfcaDevList[0].nfcId1Len; i++) {
                    rfid_card_uid.val[i] = nfcaDevList[0].nfcId1[i];
                }
            }
        }
    }
    return found;
}
