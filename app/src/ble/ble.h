#ifndef BLE_BLE_H_
#define BLE_BLE_H_

#include <zephyr/settings/settings.h>

#include "common.h"
#include "service/info.h"
#include "service/current_time.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_CONFIG_TIMEOUT K_SECONDS(30)

static K_SEM_DEFINE(ble_conn_config_sem_internal, 0, 1);
static K_SEM_DEFINE(ble_conn_config_sem, 0, 1);

static bool ble_connected = false;
static bool ble_connection_ready = false;

enum BLE_STATE
{
    BLE_STATE_DISCONNECTED,
    BLE_STATE_CONNECTED_NOT_READY,
    BLE_STATE_CONNECTED_READY,
};

/**
 * @brief Initialize BLE
 * @note must be called before advertisement start `ble_ad_start();`
 */
void ble_init();

/**
 * @brief Start BLE advertisement
 * @note must be called after `ble_init();`
 */
void ble_ad_start();

/**
 * @brief Stop BLE advertisement
 */
void ble_ad_stop();

/**
 * @brief Set BLE connection configuration
 */
void ble_req_config();

/**
 * @brief Get BLE connection state
 */
enum BLE_STATE get_ble_state();

#ifdef __cplusplus
}
#endif

#endif