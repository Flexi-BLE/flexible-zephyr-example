#include "random.h"

#include "info.h"
#include <stdlib.h> 

#include <zephyr/logging/log.h>
#include "../../log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

static volatile bool data_notify_enabled = false;
static bool config_updated = false;

static uint16_t BATCH_SIZE = 240;

static uint16_t data_cursor = 0;
static uint8_t data[MAX_NOTIFY_BUF_SIZE] = { 0 }; 

static uint32_t anchor_ms;
static uint32_t last_record_ms;

/**
 * @brief configuration byte array for managing sensor
 * @note byte 0: state of sensor (0: disabled, 1: streaming, ...)
 * @note byte 1-2: desired frequency (Hz) of streaming (ignored unless state = 1)
 */
static uint8_t config[14] = { 0U, 0U, 100U };

static ssize_t read_data(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading random data");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &data[0], sizeof(data));
}

static ssize_t read_config(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    LOG_INF("reading random config");
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &config[0], sizeof(config));
}

/**
 * @brief callback when info subscription is changed
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
static void ccc_cfg_changed_cb(const struct bt_gatt_attr *attr, uint16_t value)
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

    LOG_INF("writing random config");

	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(config)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	config_updated = 1U;

	return len;
}

BT_GATT_SERVICE_DEFINE(random_svc,
	BT_GATT_PRIMARY_SERVICE(&random_svc_uuid),
        BT_GATT_CHARACTERISTIC(&random_data_chr_uuid.uuid,
        BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        read_data,
        NULL,
        data
    ),
    BT_GATT_CCC(
        ccc_cfg_changed_cb,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),
    BT_GATT_CHARACTERISTIC(&random_config_chr_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        read_config,
        write_config,
        config
    )
);

uint8_t random_state_get() {
    return config[0];
}

uint16_t random_desired_freq_get() {
    uint16_t freq = config[2] | (config[1] << 8);
    return freq;
}

void record_random(uint32_t ref_ms) {   

    uint16_t val1 = (uint16_t) (rand() % 65535); 
    uint16_t val2 = (uint16_t) (rand() % 65535); 
    uint16_t val3 = (uint16_t) (rand() % 65535); 
    uint16_t val4 = (uint16_t) (rand() % 65535); 
    uint16_t val5 = (uint16_t) (rand() % 65535); 
    uint16_t val6 = (uint16_t) (rand() % 65535); 
    uint16_t val7 = (uint16_t) (rand() % 65535); 
    uint16_t val8 = (uint16_t) (rand() % 65535); 
    uint16_t val9 = (uint16_t) (rand() % 65535); 
    uint16_t val10 = (uint16_t) (rand() % 65535); 
    
    LOG_DBG("Random Recording Ref Time: %d", ble_reference_time_ms());

    if ( random_state_get() != 1 ) {
        return;
    }

    if ( (data_cursor + 5) >= BATCH_SIZE ) {
        
        if ( data_notify_enabled ) {
            // buffer full send notification
            int32_t start = k_uptime_get_32();
            bt_gatt_notify(default_conn, &random_svc.attrs[1], data, BATCH_SIZE);
            int32_t end = k_uptime_get_32();
            LOG_DBG("Random bt_gatt_notify took %d ms", end - start);
        }
        
        data_cursor = 0;
        for ( int i = 0; i < sizeof(data); i++ ) {
            data[i] = 0;
        }
    }

    uint32_t record_offset;

    if ( data_cursor == 0 ) {
        anchor_ms = ref_ms;
        data[data_cursor++] = (anchor_ms) & 0xFF; 
        data[data_cursor++] = (anchor_ms >> 8) & 0xFF;
        data[data_cursor++] = (anchor_ms >> 16) & 0xFF;
        data[data_cursor++] = (anchor_ms >> 24) & 0xFF;

        record_offset = 0;
    } else {
        record_offset = k_uptime_get_32() - last_record_ms;
    }

    LOG_DBG("offset: %d", record_offset);

    last_record_ms = k_uptime_get_32();
    
    data[data_cursor++] = (val1 >> 8) & 0xFF;
    data[data_cursor++] = (val1) & 0xFF;

    data[data_cursor++] = (val2 >> 8) & 0xFF;
    data[data_cursor++] = (val2) & 0xFF;

    data[data_cursor++] = (val3 >> 8) & 0xFF;
    data[data_cursor++] = (val3) & 0xFF;

    data[data_cursor++] = (val4 >> 8) & 0xFF;
    data[data_cursor++] = (val4) & 0xFF;

    data[data_cursor++] = (val5 >> 8) & 0xFF;
    data[data_cursor++] = (val5) & 0xFF;

    data[data_cursor++] = (val6 >> 8) & 0xFF;
    data[data_cursor++] = (val6) & 0xFF;

    data[data_cursor++] = (val7 >> 8) & 0xFF;
    data[data_cursor++] = (val7) & 0xFF;

    data[data_cursor++] = (val8 >> 8) & 0xFF;
    data[data_cursor++] = (val8) & 0xFF;

    data[data_cursor++] = (val9 >> 8) & 0xFF;
    data[data_cursor++] = (val9) & 0xFF;

    data[data_cursor++] = (val10 >> 8) & 0xFF;
    data[data_cursor++] = (val10) & 0xFF;

    data[data_cursor++] = (record_offset) & 0xFF;

}
