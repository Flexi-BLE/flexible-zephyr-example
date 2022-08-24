#ifndef BLE_COMMON_H_
#define BLE_COMMON_H_

#include <zephyr.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/crypto.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <bluetooth/gatt_dm.h>

#include <bluetooth/services/bas.h>

#include "uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define INTERVAL_MIN 0x140 // 320 units, 400ms
#define INTERVAL_MAX 0x140
#define CONN_LATENCY 0
#define SUPERVISION_TIMEOUT 1000

#define MAX_NOTIFY_BUF_SIZE 247 // assumes DLE is enabled

static volatile bool data_length_req;
static struct bt_conn *default_conn;
static struct bt_gatt_exchange_params exchange_params;
static struct bt_le_conn_param *conn_param = BT_LE_CONN_PARAM(INTERVAL_MIN, INTERVAL_MAX, 0, 400);

/**
 * @brief ble avertisement packet
 */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_CTS_VAL)
	),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, EH_INFO_SVC_UUID_VAL)
};

/**
 * @brief ble scan response packet definition
 * @note This is requested by cenrtal when reading advertisement packet
 */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

/**
 * @brief generic ble read
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
ssize_t generic_ble_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);

#ifdef __cplusplus
}
#endif

#endif
