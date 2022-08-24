#include "common.h"

#include <zephyr/logging/log.h>
#include "../log.h"
LOG_MODULE_REGISTER(BLE_LOG_NAME, BLE_LOG_LEVEL);

ssize_t generic_ble_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading value for %u", attr->handle);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(uint8_t));
}