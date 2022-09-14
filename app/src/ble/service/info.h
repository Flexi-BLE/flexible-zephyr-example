#ifndef BLE_SERVICE_INFO_H_
#define BLE_SERVICE_INFO_H_


#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INFO_DATA_LEN 240

static struct bt_uuid_128 info_svc_uuid = BT_UUID_INIT_128(EH_INFO_SVC_UUID_VAL);
static struct bt_uuid_128 info_epoch_chr_uuid = BT_UUID_INIT_128(EH_INFO_EPOCH_CHR_UUID_VAL);

/**
 * @brief get the reference time since the last epoch time sync with the central
 * @note will return 0 if sync has not occured
 * 
 */
uint32_t ble_reference_time_ms();

#ifdef __cplusplus
}
#endif

#endif