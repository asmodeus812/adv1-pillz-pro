#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/layer_state_changed.h>

#define LED_GPIO_NODE_ID DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)

uint32_t mod_led_state = 0;
uint32_t layer_led_state = 0;

// GPIO-based LED device
static const struct device *led_dev = DEVICE_DT_GET(LED_GPIO_NODE_ID);

static inline bool is_mod_raw(uint32_t keycode) {
    return (keycode >= HID_USAGE_KEY_KEYBOARD_LEFTCONTROL && keycode <= HID_USAGE_KEY_KEYBOARD_RIGHT_GUI);
}

static int led_keylock_listener_cb(const zmk_event_t *eh) {
    zmk_hid_indicators_t flags = zmk_hid_indicators_get_current_profile();
    unsigned int capsBit = 1 << (HID_USAGE_LED_CAPS_LOCK - 1);
    unsigned int numBit = 1 << (HID_USAGE_LED_NUM_LOCK - 1);
    unsigned int shiftBit  = 1 << (HID_USAGE_LED_SHIFT - 1);

    if (flags & capsBit) {
        led_on(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_caps)));
    } else {
        led_off(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_caps)));
    }
    if (flags & numBit) {
        led_on(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_num)));
    } else {
        led_off(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_num)));
    }
    return 0;
}

static int led_layer_listener_cb(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *el = as_zmk_layer_state_changed(eh);
    if (!el || el->layer == 0) {
        // led_off(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_scroll)));
        // layer_led_state = 0;
        return 0;
    }

    if (el->state) {
        led_on(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_scroll)));
        layer_led_state = 1;
    } else {
        if (mod_led_state == 0) {
            led_off(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_scroll)));
        }
        layer_led_state = 0;
    }
    return 0;
}

static int led_keycode_listener_cb(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ek = as_zmk_keycode_state_changed(eh);
    if (!ek) return 0;

    if (is_mod_raw(ek->keycode)) {
        if (ek->state) {
            led_on(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_scroll)));
            ++mod_led_state;
        } else {
            if (mod_led_state > 0) {
                --mod_led_state;
            }
            if (mod_led_state == 0 && layer_led_state == 0) {
                led_off(led_dev, DT_NODE_CHILD_IDX(DT_ALIAS(led_scroll)));
            }
        }
    }
    return 0;
}

ZMK_LISTENER(led_indicators_listener, led_keylock_listener_cb);
ZMK_SUBSCRIPTION(led_indicators_listener, zmk_hid_indicators_changed);

ZMK_LISTENER(led_layer_listener, led_layer_listener_cb);
ZMK_SUBSCRIPTION(led_layer_listener, zmk_layer_state_changed);

ZMK_LISTENER(led_keycode_listener, led_keycode_listener_cb);
ZMK_SUBSCRIPTION(led_keycode_listener, zmk_keycode_state_changed);

static int leds_init(const struct device *device) {
    if (!device_is_ready(led_dev)) {
        return -ENODEV;
    }

    return 0;
}

// run leds_init on boot
SYS_INIT(leds_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
