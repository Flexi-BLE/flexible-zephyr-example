#include "info.h"

#include <zephyr/logging/log.h>
#include "../../log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

static volatile bool info_notify_enabled = false;
static uint8_t info_data[INFO_DATA_LEN] = { 0 };

/**
 * @brief read info data
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
static ssize_t read_info(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading accel notify");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(info_data[0]));
}

/**
 * @brief callback when info subscription is changed
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
static void info_ccc_cfg_changed_cb(const struct bt_gatt_attr *attr, uint16_t value)
{
	info_notify_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("info->log ccc cfg changed %d", info_notify_enabled);
}

/**
 * @brief info service definition
 */
BT_GATT_SERVICE_DEFINE(info_svc,
	BT_GATT_PRIMARY_SERVICE(&info_svc_uuid),
    BT_GATT_CHARACTERISTIC(&info_chr_uuid.uuid,
        BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        read_info,
        NULL,
        info_data
    ),
    BT_GATT_CCC(
        info_ccc_cfg_changed_cb,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),
);

void notify_test(uint8_t n)
{
    if (!info_notify_enabled) {
        return;
    }

    info_data[0] = n;

    bt_gatt_notify(default_conn, &info_svc.attrs[1], info_data, sizeof(info_data));   
}