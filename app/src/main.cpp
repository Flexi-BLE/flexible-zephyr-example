#include <zephyr.h>
#include <string.h>
#include <soc.h>

#include <logging/log.h>

#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>

#include <algorithm>
#include <vector>
#include <cstdlib>

#include "ble/ble.h"
#include "ble/service/info.h"
#include <zephyr/bluetooth/services/bas.h>
#include "ble/service/accel.h"
#include "ble/service/ppg.h"
#include "ble/service/ecg_service.h"

#include "ppg/ppg.hpp"
#include "ppg/led_mapping.h"

#include "log.h"
LOG_MODULE_REGISTER(MAIN_LOG_NAME, MAIN_LOG_LEVEL);

// get LEDs
static struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(LED1), gpios, {0});

// LED Timers
struct k_timer green_led_flasher;
int green_led_on_period_ms = 25;
int green_led_off_period_ms = 975;

void green_led_flasher_callback(struct k_timer *timer)
{
	static bool green_led_state = true;
	if (green_led_state == true)
	{
		if (green_led_on_period_ms > 0)
		{
			gpio_pin_set_dt(&green_led, true);
		}
		green_led_state = false;
		k_timer_start(&green_led_flasher, K_MSEC(green_led_on_period_ms), K_NO_WAIT);
	}
	else
	{
		gpio_pin_set_dt(&green_led, false);
		green_led_state = true;
		k_timer_start(&green_led_flasher, K_MSEC(green_led_off_period_ms), K_NO_WAIT);
	}
}

static void initialize_led(struct gpio_dt_spec led)
{
    if (led.port && !device_is_ready(led.port)) {
		LOG_ERR("LED device %s is not ready; ignoring it", led.port->name);
		led.port = NULL;
	}
	if (led.port) {
		int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			LOG_ERR("%d: failed to configure LED device %s pin %d", ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			LOG_INF("Set up LED at %s pin %d", led.port->name, led.pin);
		}
	}
}

void accel_entry_point(void*, void*, void*)
{
	LOG_INF("Accelerometer thread started");

    // Initialize IMU
	int ret;
    const struct device *imu = get_ism330_device();
	sensor_value accel_freq = {.val1=get_accel_desired_freq(), .val2=0};
	k_mutex_lock(&i2c_mutex, K_FOREVER);
    ret = sensor_attr_set(imu, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &accel_freq);
	if (ret != 0)
	{
		LOG_ERR("%d: failed to configure accelerometer", ret);
	}
	k_mutex_unlock(&i2c_mutex);

	bool last_accel_state = accel_state_get();
	uint16_t last_accel_freq = get_accel_desired_freq();
	uint32_t accel_freq_us = 1000000 / last_accel_freq;
	
	// for simulating values
	struct sensor_value accel_x = {.val1=0};
	struct sensor_value accel_y = {.val1=10};
	struct sensor_value accel_z = {.val1=20};

	while(1)
	{
		bool accel_state = accel_state_get();

		if (last_accel_state != accel_state)
		{
			LOG_INF("Accelerometer state changed to %d", accel_state);
			last_accel_state = accel_state;
		}

		if (accel_state)
		{
			uint64_t start_ticks = k_uptime_ticks();

			// simulate accelerometer data
			accel_x.val1 += 1;
			accel_y.val1 += 1;
			accel_z.val1 += 1;

			record_accel(ble_reference_time_ms(), accel_x, accel_y, accel_z);

			uint16_t desired_freq = get_accel_desired_freq();

			if (desired_freq != last_accel_freq)
			{
				last_accel_freq = desired_freq;

				accel_freq_us = 1000000 / desired_freq;

				LOG_INF("Frequency changed to %d", desired_freq);
			}

			uint64_t end_ticks = k_uptime_ticks();
			uint32_t elapsed_us = k_ticks_to_us_near32(end_ticks - start_ticks);

			if (elapsed_us < accel_freq_us)
			{
				LOG_DBG("Accel reading took %uus. Sleeping for %uus", elapsed_us, (accel_freq_us - elapsed_us));
				k_usleep(accel_freq_us - elapsed_us);
			}
			else
			{
				LOG_WRN("Can't meet desired frequency, accel reading took %uus", elapsed_us);
				k_yield();
			}
		}
		else
		{
			k_msleep(500); // sleep longer if we're not running
		}
	}
}

#define ACCEL_STACK_SIZE 1024
#define ACCEL_PRIORITY 2

K_THREAD_DEFINE(accel_tid, ACCEL_STACK_SIZE,
                accel_entry_point, NULL, NULL, NULL,
                ACCEL_PRIORITY, 0, 0);

void random_entry_point(void*, void*, void*)
{
	LOG_INF("Random thread started");

	uint32_t last_random_reading = k_uptime_get_32();
	uint8_t random_freq_ms = 1000 / random_desired_freq_get();

	while(1)
	{
		if ( ((k_uptime_get_32() - last_random_reading) >= random_freq_ms) && (random_state_get() == 1) ) {
			last_random_reading = k_uptime_get_32();

			record_random(ble_reference_time_ms());

			random_freq_ms = 1000 / random_desired_freq_get();
		}
	}
}

#define RANDOM_STACK_SIZE 1024
#define RANDOM_PRIORITY 2

K_THREAD_DEFINE(random_tid, RANDOM_STACK_SIZE,
                random_entry_point, NULL, NULL, NULL,
                RANDOM_PRIORITY, 0, 0);


void ble_entry_point(void*, void*, void*)
{
	LOG_INF("BLE thread started");
	
	// BLE Setup
	ble_init();
	ble_ad_start();
	bt_bas_set_battery_level(100U);

    while(1)
    {
		enum BLE_STATE ble_state = get_ble_state();
		switch (ble_state)
		{
			case BLE_STATE_CONNECTED_NOT_READY:
				ble_req_config();
				green_led_on_period_ms = 250;
				green_led_off_period_ms = 750;
				break;
			case BLE_STATE_CONNECTED_READY:
				green_led_on_period_ms = 750;
				green_led_off_period_ms = 250;
				break;
			case BLE_STATE_DISCONNECTED:
				green_led_on_period_ms = 50;
				green_led_off_period_ms = 950;
				break;
		}
		k_msleep(100);
    }
}

#define BLE_STACK_SIZE 2048
#define BLE_PRIORITY 1

K_THREAD_DEFINE(ble_tid, BLE_STACK_SIZE,
                ble_entry_point, NULL, NULL, NULL,
                BLE_PRIORITY, 0, 1000);


void main(void)
{   
	LOG_INF("ExtHub initializing...");

	//Initialize green led
	initialize_led(green_led);

	k_timer_init(&green_led_flasher, green_led_flasher_callback, NULL);
	k_timer_start(&green_led_flasher, K_MSEC(500), K_NO_WAIT);

	while(1)
	{
		k_msleep(1000);
	}
}
