#ifndef BLE_SERVICE_ACCEL_H_
#define BLE_SERVICE_ACCEL_H_

#include <drivers/sensor.h>
#include "../common.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct bt_uuid_128 accel_svc_uuid = BT_UUID_INIT_128(EH_ACCEL_SVC_UUID_VAL);
static struct bt_uuid_128 accel_config_chr_uuid = BT_UUID_INIT_128(EH_ACCEL_CONFIG_CHR_UUID_VAL);
static struct bt_uuid_128 accel_data_chr_uuid = BT_UUID_INIT_128(EH_ACCEL_DATA_CHR_UUID_VAL);

void record_accel(
    uint32_t ref_ms,
    struct sensor_value accel_x, 
    struct sensor_value accel_y, 
    struct sensor_value accel_z
);

/**
 * @brief Get the desired frequency of the accelerometer.
 * @return the desired frequency in Hz
 */
uint16_t get_accel_desired_freq();

/**
 * @brief Get the batches of the accelerometer service, configured over BLE
 * @return the size in bytes of the batches of the accelerometer data characteristic.
 */
uint8_t get_accel_batch_size();

/**
 * @brief Get the status of the accelerometer service, configured over BLE
 * @return The status of the accelerometer service.
 */
uint8_t accel_state_get();

#ifdef __cplusplus
}
#endif

#endif /* BLE_SERVICE_ACCEL_SVC_H_ */