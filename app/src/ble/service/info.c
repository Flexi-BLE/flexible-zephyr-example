#include "info.h"

#include <zephyr/logging/log.h>
#include "../../log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

static uint8_t epoch_time_data[8] = { 0 };

static bool epoch_updated = false;
static uint32_t time_at_update = 0;

static ssize_t write_epoch(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
	const void *buf,
    uint16_t len, 
    uint16_t offset,
    uint8_t flags
) {
    LOG_INF("writing accel config");

	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(epoch_time_data)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

    LOG_DBG("new epoch time written");

	memcpy(value + offset, buf, len);
	epoch_updated = 1U;
    time_at_update = k_uptime_get_32();

	return len;
}

static ssize_t read_accel_data(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading epoch time data");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &epoch_time_data[0], sizeof(epoch_time_data));
}

/**
 * @brief info service definition
 */
BT_GATT_SERVICE_DEFINE(info_svc,
	BT_GATT_PRIMARY_SERVICE(&info_svc_uuid),
    BT_GATT_CHARACTERISTIC(&info_epoch_chr_uuid.uuid,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        read_accel_data,
        write_epoch,
        epoch_time_data
    ),
);

uint32_t ble_reference_time_ms() {
    if ( epoch_updated ) {
        return k_uptime_get_32() - time_at_update;
    } else {
        return 0;
    }   
}