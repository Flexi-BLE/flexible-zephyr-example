#include <zephyr/sys/byteorder.h>

#include "current_time.h"

#include <zephyr/logging/log.h>
#include "../../log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

static uint8_t ct[10];
static uint8_t ct_update;

static void ct_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	/* TODO: Handle value */
}

static ssize_t read_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, uint16_t len, uint16_t offset)
{
	const char *value = attr->user_data;

	LOG_INF("current time read");

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(ct));
}

static ssize_t write_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			const void *buf, uint16_t len, uint16_t offset,
			uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(ct)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	ct_update = 1U;

	LOG_INF("current time updated");
	log_time();

	return len;
}

/* Current Time Service Declaration */
BT_GATT_SERVICE_DEFINE(current_time_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_CTS),
	BT_GATT_CHARACTERISTIC(
        BT_UUID_CTS_CURRENT_TIME, 
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_ct,
        write_ct,
        ct
    ),
	BT_GATT_CCC(
        ct_ccc_cfg_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),
);

static void generate_current_time(uint8_t *buf)
{
	uint16_t year;

	/* 'Exact Time 256' contains 'Day Date Time' which contains
	 * 'Date Time' - characteristic contains fields for:
	 * year, month, day, hours, minutes and seconds.
	 */

	year = sys_cpu_to_le16(2000);
	memcpy(buf,  &year, 2); /* year */
	buf[2] = 1U; /* months starting from 1 */
	buf[3] = 1U; /* day */
	buf[4] = 00U; /* hours */
	buf[5] = 00U; /* minutes */
	buf[6] = 00U; /* seconds */

	/* 'Day of Week' part of 'Day Date Time' */
	buf[7] = 1U; /* day of week starting from 1 */

	/* 'Fractions 256 part of 'Exact Time 256' */
	buf[8] = 50U;

	/* Adjust reason */
	buf[9] = 0U; /* No update, change, etc */

	log_time();
}

void log_time(void)
{
	LOG_INF(
		"current time:\n - month: %u\n - day: %u\n - hours: %u\n - minutes: %u\n - seconds: %u\n - day of week: %u\n - fractions: %u\n - adjust reason: %u",
		ct[2], ct[3], ct[4], ct[5], ct[6], ct[7], ct[8], ct[9]
	);
}


void current_time_svc_init(void)
{
	/* Simulate current time for Current Time Service */
	generate_current_time(ct);
}

void current_time_svc_notify(void)
{	/* Current Time Service updates only when time is changed */
	if (!ct_update) {
		return;
	}

	ct_update = 0U;
	bt_gatt_notify(NULL, &current_time_svc.attrs[1], &ct, sizeof(ct));
}