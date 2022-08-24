#include "ble.h"

#include <zephyr/logging/log.h>
#include "log.h"
LOG_MODULE_DECLARE(BLE_LOG_NAME);

/**
 * @brief request connection parameters to central
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
void bt_configuration_set(
	const struct bt_le_conn_param *conn_param,
	const struct bt_conn_le_phy_param *phy,
	const struct bt_conn_le_data_len_param *data_len
) {
	int err;
	struct bt_conn_info info = { 0 };

	err = bt_conn_get_info(default_conn, &info);;
	if (err) {
		LOG_ERR("failed to get connection info %d", err);
		return;
	}

	err = bt_conn_le_phy_update(default_conn, phy);
	if (err) {
		LOG_ERR("PHY update failed: %d", err);
	} else {
		LOG_INF("pending PHY update ...");
		err = k_sem_take(&ble_conn_config_sem_internal, BLE_CONFIG_TIMEOUT);
		if (err)
		{
			LOG_ERR("PHY update timeout");
		}
	}

	LOG_INF("data length %d", info.le.data_len->tx_max_len);
	if (info.le.data_len->tx_max_len != data_len->tx_max_len) {
		data_length_req = true;

		err = bt_conn_le_data_len_update(default_conn, data_len);
		if (err) {
			LOG_ERR("LE data length update failed: %d", err);
		} else {
			LOG_INF("LE Data length update pending");
			err = k_sem_take(&ble_conn_config_sem_internal, BLE_CONFIG_TIMEOUT);
			if (err) {
				LOG_ERR("LE Data Length update timeout");
			}
		}
	}

	if (info.le.interval != conn_param->interval_max) {
		err = bt_conn_le_param_update(default_conn, conn_param);
		if (err) {
			LOG_ERR("Connection parameters update failed: %d", err);
		} else {
			LOG_INF("Connection parameters update pending");
			err = k_sem_take(&ble_conn_config_sem_internal, BLE_CONFIG_TIMEOUT);
			if (err) {
				LOG_ERR("Connection parameters update timeout");
			}
		}
	}

	ble_connection_ready = true;
	k_sem_give(&ble_conn_config_sem);
}

/**
 * @brief callback for ble connection
 * 
 * @param conn_param 
 * @param phy 
 * @param data_len 
 */
static void bt_connected_cb(struct bt_conn *conn, uint8_t hci_err) 
{
    struct bt_conn_info info = {0};
	int err;

	if (hci_err) {
		if (hci_err == BT_HCI_ERR_UNKNOWN_CONN_ID) {
			LOG_ERR("Canceled creating connection");
			return;
		}

		LOG_ERR("Connection failed (err 0x%02x)", hci_err);
		return;
	}

	if (default_conn) {
		LOG_INF("Connection exists, disconnect second connection");
		bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		return;
	}

	default_conn = bt_conn_ref(conn);

	err = bt_conn_get_info(default_conn, &info);
	if (err) {
		LOG_ERR("Failed to get connection info %d", err);
		return;
	}

	ble_connected = true;

	LOG_INF("Connected as %s", info.role == BT_CONN_ROLE_CENTRAL ? "central" : "peripheral");
	LOG_INF("Conn. interval is %u units", info.le.interval);
}

/**
 *  @brief callback for ble disconnection
 * 
 * @param conn 
 * @param reason 
 */
static void bt_disconnected_cb(struct bt_conn *conn, uint8_t reason) 
{
    struct bt_conn_info info = {0};
	int err;

	LOG_INF("Disconnected (reason 0x%02x)", reason);

	// test_ready = false;
	if (default_conn) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}

	err = bt_conn_get_info(conn, &info);
	if (err) {
		LOG_ERR("Failed to get connection info (%d)", err);
		return;
	}

    ble_connected = false;
	ble_connection_ready = false;

	ble_ad_start();
}

/**
 * @brief callback for requesting connection parameters update
 * 
 * @param conn 
 * @param param 
 * @return true 
 * @return false 
 */
static bool le_param_req_cb(struct bt_conn *conn, struct bt_le_conn_param *param) 
{
    LOG_INF("Connection parameters update request received.");
	LOG_INF("Minimum interval: %d, Maximum interval: %d",
	       param->interval_min, param->interval_max);
	LOG_INF("Latency: %d, Timeout: %d", param->latency, param->timeout);

	return true;
}

/**
 * @brief callback did update ble connection parameters
 * 
 * @param conn 
 * @param interval 
 * @param latency 
 * @param timeout 
 */
static void le_param_updated_cb(
    struct bt_conn *conn,
    uint16_t interval,
	uint16_t latency,
    uint16_t timeout
) {
    LOG_INF("Connection parameters updated."
	       " interval: %d, latency: %d, timeout: %d",
	       interval, latency, timeout);

	k_sem_give(&ble_conn_config_sem_internal);
}


/**
 * @brief helper to print PHY status
 * 
 * @param phy 
 * @return const char* 
 */
static const char *phy2str(uint8_t phy)
{
	switch (phy) {
	case 0: return "No packets";
	case BT_GAP_LE_PHY_1M: return "LE 1M";
	case BT_GAP_LE_PHY_2M: return "LE 2M";
	case BT_GAP_LE_PHY_CODED: return "LE Coded";
	default: return "Unknown";
	}
}

/**
 * @brief callback when PHY did update
 * 
 * @param conn 
 * @param param 
 */
static void le_phy_updated_cb(
    struct bt_conn *conn,
	struct bt_conn_le_phy_info *param
) {
    LOG_INF(
		"LE PHY updated: TX PHY %s, RX PHY %s",
		phy2str(param->tx_phy),
		phy2str(param->rx_phy)
	);

	k_sem_give(&ble_conn_config_sem_internal); 
}

/**
 * @brief callback when data length did update
 * 
 * @param conn 
 * @param info 
 */
static void le_data_length_updated_cb(
    struct bt_conn *conn,
	struct bt_conn_le_data_len_info *info
) {
    if (!data_length_req) {
		return;
	}

	LOG_INF("LE data len updated: TX (len: %d time: %d)"
	       " RX (len: %d time: %d)", info->tx_max_len,
	       info->tx_max_time, info->rx_max_len, info->rx_max_time);

	data_length_req = false;
	k_sem_give(&ble_conn_config_sem_internal);
}

/** @breif define callbacks for BLE connection */
BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = bt_connected_cb,
	.disconnected = bt_disconnected_cb,
	.le_param_req = le_param_req_cb,
	.le_param_updated = le_param_updated_cb,
	.le_phy_updated = le_phy_updated_cb,
	.le_data_len_updated = le_data_length_updated_cb
};

void ble_init() 
{
	int err;
	
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	current_time_svc_init();

	LOG_INF("ExtHub Bluetooth initialized");
}

void ble_ad_start()
{
	// struct bt_le_adv_param * adv_param = BT_LE_ADV_PARAM(
	// 	BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME,
	// 	BT_GAP_ADV_FAST_INT_MIN_2,
	// 	BT_GAP_ADV_FAST_INT_MAX_2,
	// 	NULL
	// );

	int err;

	if ( IS_ENABLED(CONFIG_SETTINGS) ) {
		settings_load();
	}

	err = bt_le_adv_start(
		// adv_param,
		BT_LE_ADV_CONN_NAME,
		ad,
		ARRAY_SIZE(ad),
		// sd,
		// ARRAY_SIZE(sd)
		NULL,
		0
	);


	if (err) {
		LOG_ERR("Failed to start advertiser (%d)", err);
		return;
	}

	LOG_INF("Advertisement started");
}

void ble_ad_stop()
{
	bt_le_adv_stop();
	LOG_INF("Advertisement stopped");
	return;
}

void ble_req_config()
{
	if ( !ble_connected || ble_connection_ready ) {
		return;
	}
	
	bt_configuration_set(
		BT_LE_CONN_PARAM(
			INTERVAL_MIN, 
			INTERVAL_MAX,
			CONN_LATENCY,
			SUPERVISION_TIMEOUT
		),
		BT_CONN_LE_PHY_PARAM_2M,
		BT_LE_DATA_LEN_PARAM_MAX
	);
	k_sem_take(&ble_conn_config_sem, BLE_CONFIG_TIMEOUT);
}