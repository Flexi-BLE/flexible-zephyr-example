#include "accel.h"

#include <zephyr/logging/log.h>
#include "../log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

static volatile bool data_notify_enabled = false;
static bool config_updated = false;

static uint16_t accel_data_cursor = 0;
static uint8_t accel_data[MAX_NOTIFY_BUF_SIZE] = { 0 }; 

/**
 * @brief configuration byte array for managing sensor
 * @note byte 0: state of sensor (0: disabled, 1: streaming, ...)
 * @note byte 1: desired frequency (Hz) of streaming (1-255Hz, ignored unless state = 1)
 * @note byte 2: size of data batch (min: 7, max: 247)
 */
static uint8_t accel_config[3] = { 1U, 100U, 240U };

static ssize_t read_accel_data(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading accel data");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &accel_data[0], sizeof(accel_data));
}

static ssize_t read_accel_config(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading accel config");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &accel_config[0], sizeof(accel_config));
}

/**
 * @brief callback when info subscription is changed
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
static void accel_ccc_cfg_changed_cb(const struct bt_gatt_attr *attr, uint16_t value)
{
	data_notify_enabled = (value == BT_GATT_CCC_NOTIFY);
    LOG_INF("info->log ccc cfg changed %d", data_notify_enabled);
}

static ssize_t write_config(
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr,
	const void *buf,
    uint16_t len, 
    uint16_t offset,
    uint8_t flags
) {

    LOG_INF("writing accel config");

	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(accel_config)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	config_updated = 1U;

	return len;
}

BT_GATT_SERVICE_DEFINE(accel_svc,
	BT_GATT_PRIMARY_SERVICE(&accel_svc_uuid),
        BT_GATT_CHARACTERISTIC(&accel_data_chr_uuid.uuid,
        BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        read_accel_data,
        NULL,
        accel_data
    ),
    BT_GATT_CCC(
        accel_ccc_cfg_changed_cb,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),
    BT_GATT_CHARACTERISTIC(&accel_config_chr_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        read_accel_config,
        write_config,
        accel_config
    )
);

uint8_t get_status() {
    return accel_config[0];
}

uint8_t get_batch_size() {
    return accel_config[2];
}

uint8_t get_desired_freq() {
    LOG_DBG("accel desired hz: %d", accel_config[1]);
    return accel_config[1];
}


void record_accel(
    struct sensor_value accel_x,
    struct sensor_value accel_y,
    struct sensor_value accel_z
) {

    if ( get_status() != 1 ) {
        return;
    }
    
    uint8_t batch_size = get_batch_size();

    if ( accel_data_cursor + 7 >= batch_size ) {
        
        if ( data_notify_enabled ) {
            // buffer full send notification
            LOG_DBG("sending accel notification");
            int32_t start = k_uptime_get_32();
            bt_gatt_notify(default_conn, &accel_svc.attrs[1], accel_data, batch_size);
            int32_t end = k_uptime_get_32();
            LOG_DBG("ACCEL bt_gatt_notify took %d ms", end - start);
        }
        
        accel_data_cursor = 0;
        for ( int i = 0; i < sizeof(accel_data); i++ ) {
            accel_data[i] = 0;
        }
    }
    
    // LOG_DBG("recording accel data (x: %d.%d, y: %d.%d, z: %d.%d)",
    //     accel_x.val1, accel_x.val2,
    //     accel_y.val1, accel_y.val2,
    //     accel_z.val1, accel_z.val2
    // );

    accel_data[accel_data_cursor++] = (int8_t)accel_x.val1;
    accel_data[accel_data_cursor++] = (int8_t)accel_x.val2;
    accel_data[accel_data_cursor++] = (int8_t)accel_y.val1;
    accel_data[accel_data_cursor++] = (int8_t)accel_y.val2;
    accel_data[accel_data_cursor++] = (int8_t)accel_z.val1;
    accel_data[accel_data_cursor++] = (int8_t)accel_z.val2;
    // FIXME: placeholder millisecond offset
    accel_data[accel_data_cursor++] = 100U;

    // LOG_DBG("recording accel data (x: %d.%d, y: %d.%d, z: %d.%d) cur: %d",
    //     (int8_t)accel_x.val1, (int8_t)accel_x.val2,
    //     (int8_t)accel_y.val1, (int8_t)accel_y.val2,
    //     (int8_t)accel_z.val1, (int8_t)accel_z.val2,
    //     accel_data_cursor
    // );
    
};