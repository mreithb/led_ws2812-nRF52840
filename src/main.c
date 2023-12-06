/*
 * Copyright (c) 2017 Linaro Limited
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#define STRIP_NODE DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)

//#define DELAY_TIME K_USEC(1)
#define DELAY_TIME K_NSEC(10)
//#define DELAY_TIME K_MSEC(100)

#define RGB(_r, _g, _b)                 \
	{                                   \
		.r = (_r), .g = (_g), .b = (_b) \
	}

struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

#define SWA_NODE DT_ALIAS(swa)
#define SWB_NODE DT_ALIAS(swb)
#define SWC_NODE DT_ALIAS(swc)
static const struct gpio_dt_spec button_a = GPIO_DT_SPEC_GET_OR(SWA_NODE, gpios, {0});
static const struct gpio_dt_spec button_b = GPIO_DT_SPEC_GET_OR(SWB_NODE, gpios, {0});
static const struct gpio_dt_spec button_c = GPIO_DT_SPEC_GET_OR(SWC_NODE, gpios, {0});

static struct gpio_callback button_a_cb_data;
static struct gpio_callback button_b_cb_data;
static struct gpio_callback button_c_cb_data;

void buttons_init(void);

void button_a_pressed(const struct device *dev, struct gpio_callback *cb,
					  uint32_t pins);

void button_b_pressed(const struct device *dev, struct gpio_callback *cb,
					  uint32_t pins);

void button_c_pressed(const struct device *dev, struct gpio_callback *cb,
					  uint32_t pins);

static const struct led_rgb colors[] = {
	RGB(0x4f, 0x00, 0x00), /* red */
};

int main(void)
{
	size_t cursor = 0, color = 0;
	int rc;

	buttons_init();

	if (device_is_ready(strip))
	{
		LOG_INF("Found LED strip device %s", strip->name);
	}
	else
	{
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

	LOG_INF("Displaying pattern on strip");
	while (1)
	{
		memset(&pixels, 0x00, sizeof(pixels));
		//memcpy(&pixels[cursor], &colors[0], (sizeof(colors) / STRIP_NUM_PIXELS) * (STRIP_NUM_PIXELS - cursor));

		for (int i = 0; i < 10; i = i + 2)
			memcpy(&pixels[i], &colors[0], sizeof(colors[0]));
	
		if( cursor % 4 == 0)
			for (int i = 1; i < 10; i = i + 2)
				memcpy(&pixels[i], &colors[0], sizeof(colors[0]));

		if (cursor > 0)
		{
			//memcpy(&pixels[0], &colors[STRIP_NUM_PIXELS - cursor - 1], (sizeof(colors) / STRIP_NUM_PIXELS) * (cursor));
		}

		cursor++;
		if(cursor == 100)
			cursor = 0;

		rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);

		if (rc)
		{
			LOG_ERR("couldn't update strip: %d", rc);
		}
		k_sleep(DELAY_TIME);
	}
	return 0;
}

void button_a_pressed(const struct device *dev, struct gpio_callback *cb,
					  uint32_t pins)
{
	LOG_INF("Button a pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void button_b_pressed(const struct device *dev, struct gpio_callback *cb,
					  uint32_t pins)
{
	LOG_INF("Button b pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void button_c_pressed(const struct device *dev, struct gpio_callback *cb,
					  uint32_t pins)
{
	LOG_INF("Button c pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void buttons_init(void)
{
	int rc;

	if (!gpio_is_ready_dt(&button_a))
	{
		LOG_INF("Error: button device %s is not ready\n",
				button_a.port->name);
		return 0;
	}
	if (!gpio_is_ready_dt(&button_b))
	{
		LOG_INF("Error: button device %s is not ready\n",
				button_b.port->name);
		return 0;
	}
	if (!gpio_is_ready_dt(&button_c))
	{
		LOG_INF("Error: button device %s is not ready\n",
				button_c.port->name);
		return 0;
	}
	rc = gpio_pin_configure_dt(&button_a, GPIO_INPUT);
	if (rc != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n", rc, button_a.port->name, button_a.pin);
		return 0;
	}
	rc = gpio_pin_configure_dt(&button_b, GPIO_INPUT);
	if (rc != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n", rc, button_b.port->name, button_b.pin);
		return 0;
	}
	rc = gpio_pin_configure_dt(&button_c, GPIO_INPUT);
	if (rc != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n", rc, button_c.port->name, button_c.pin);
		return 0;
	}
	rc = gpio_pin_interrupt_configure_dt(&button_a, GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n", rc, button_a.port->name, button_a.pin);
		return 0;
	}
	rc = gpio_pin_interrupt_configure_dt(&button_b, GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n", rc, button_b.port->name, button_b.pin);
		return 0;
	}
	rc = gpio_pin_interrupt_configure_dt(&button_c, GPIO_INT_EDGE_TO_ACTIVE);
	if (rc != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n", rc, button_c.port->name, button_c.pin);
		return 0;
	}
	gpio_init_callback(&button_a_cb_data, button_a_pressed, BIT(button_a.pin));
	gpio_add_callback(button_a.port, &button_a_cb_data);
	gpio_init_callback(&button_b_cb_data, button_b_pressed, BIT(button_b.pin));
	gpio_add_callback(button_b.port, &button_b_cb_data);
	gpio_init_callback(&button_c_cb_data, button_c_pressed, BIT(button_c.pin));
	gpio_add_callback(button_c.port, &button_c_cb_data);
	LOG_INF("Buttons ready");
}
