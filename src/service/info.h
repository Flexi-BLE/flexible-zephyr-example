#ifndef BLE_SERVICE_INFO_H_
#define BLE_SERVICE_INFO_H_


#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INFO_DATA_LEN 240

static struct bt_uuid_128 info_svc_uuid = BT_UUID_INIT_128(EH_INFO_SVC_UUID_VAL);
static struct bt_uuid_128 info_chr_uuid = BT_UUID_INIT_128(EH_INFO_LOG_CHR_UUID_VAL);

/**
 * @brief test gatt characteristic notification
 * 
 * @param n a number
 */
void notify_test(uint8_t n);

#ifdef __cplusplus
}
#endif

#endif