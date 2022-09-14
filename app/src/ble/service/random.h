#ifndef BLE_SERVICE_RANDOM_H_
#define BLE_SERVICE_RANDOM_H_

#include <drivers/sensor.h>
#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif


static struct bt_uuid_128 random_svc_uuid = BT_UUID_INIT_128(EH_RANDOM_SVC_UUID_VAL);
static struct bt_uuid_128 random_config_chr_uuid = BT_UUID_INIT_128(EH_RANDOM_CONFIG_CHR_UUID_VAL);
static struct bt_uuid_128 random_data_chr_uuid = BT_UUID_INIT_128(EH_RANDOM_DATA_CHR_UUID_VAL);

void record_random(uint32_t ref_ms);

uint8_t random_state_get();
uint16_t random_desired_freq_get();


#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_RANDOM_SVC_H_ */