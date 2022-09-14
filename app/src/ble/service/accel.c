#include "accel.h"
#include <math.h>

#include <zephyr/logging/log.h>
#include "../../log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

static volatile bool data_notify_enabled = false;
static bool config_updated = false;

static uint16_t accel_data_cursor = 0;
static uint8_t accel_data[MAX_NOTIFY_BUF_SIZE] = { 0 }; 

static uint32_t anchor_ms;
static uint32_t last_record_ms;

/**
 * @brief configuration byte array for managing sensor
 * @note byte 0: state of sensor (0: disabled, 1: streaming, ...)
 * @note byte 1-2: desired frequency (Hz) of streaming (ignored unless state = 1)
 * @note byte 2: size of data batch (min: 7, max: 247)
 */
static uint8_t accel_config[4] = { 0U, 0U, 26U, 140U };

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

    LOG_DBG("new accel config values: %d, %d, %d", value[0], value[1], value[2]);
    // LOG_DBG("old accel config values: %d, %d, %d", buf[0], buf[1], buf[2]);

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

uint8_t accel_state_get() {
    return accel_config[0];
}

uint8_t get_accel_batch_size() {
    return accel_config[3];
}

uint16_t get_accel_desired_freq() {
    uint16_t freq = accel_config[2] | (accel_config[1] << 8);
    return freq;
}

void record_accel(
    uint32_t ref_ms,
    struct sensor_value accel_x,
    struct sensor_value accel_y,
    struct sensor_value accel_z
) {
    uint32_t now = k_uptime_get_32();

    if ( accel_state_get() != 1 ) {
        return;
    }

    uint8_t batch_size = get_accel_batch_size();

    if ( accel_data_cursor + 13 >= batch_size ) {
        
        if ( data_notify_enabled ) {
            // buffer full send notification
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

    uint32_t record_offset;

    if ( accel_data_cursor == 0 ) {
        anchor_ms = ref_ms;
        accel_data[accel_data_cursor++] = (anchor_ms) & 0xFF; 
        accel_data[accel_data_cursor++] = (anchor_ms >> 8) & 0xFF;
        accel_data[accel_data_cursor++] = (anchor_ms >> 16) & 0xFF;
        accel_data[accel_data_cursor++] = (anchor_ms >> 24) & 0xFF;

        record_offset = 0;
    } else {
        record_offset = now - last_record_ms;
    }

    last_record_ms = now;

    int32_t x32 = (accel_x.val1 * 1000000) + accel_x.val2;
    int32_t y32 = (accel_y.val1 * 1000000) + accel_y.val2;
    int32_t z32 = (accel_z.val1 * 1000000) + accel_z.val2;

    accel_data[accel_data_cursor++] = (x32 >> 24) & 0xFF;
    accel_data[accel_data_cursor++] = (x32 >> 16) & 0xFF;
    accel_data[accel_data_cursor++] = (x32 >> 8) & 0xFF;
    accel_data[accel_data_cursor++] = (x32) & 0xFF; 

    accel_data[accel_data_cursor++] = (y32 >> 24) & 0xFF;
    accel_data[accel_data_cursor++] = (y32 >> 16) & 0xFF;
    accel_data[accel_data_cursor++] = (y32 >> 8) & 0xFF;
    accel_data[accel_data_cursor++] = (y32) & 0xFF; 

    accel_data[accel_data_cursor++] = (z32 >> 24) & 0xFF;
    accel_data[accel_data_cursor++] = (z32 >> 16) & 0xFF;
    accel_data[accel_data_cursor++] = (z32 >> 8) & 0xFF;
    accel_data[accel_data_cursor++] = (z32) & 0xFF; 
    
    accel_data[accel_data_cursor++] = (record_offset) & 0xFF;
};