#include <fcntl.h>
#include <libevdev/libevdev-uinput.h>
#include <libudev.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    struct libevdev *dev = libevdev_new();

    // Not sure about this
    struct input_absinfo absinfo = {
        .value = 500,
        .minimum = 0,
        .maximum = 1000,
        .resolution = 2,
    };

    libevdev_set_name(dev, "scroller");
    libevdev_set_id_bustype(dev, BUS_USB);
    libevdev_set_id_vendor(dev, 0x1234);
    libevdev_set_id_product(dev, 0x5678);

    // These properties are copied from "SYNA32A0:00 06CB:CE14 Touchpad",
    // a touchpad on "HP ENVY x360 Convertible 13-ay0xxx" laptop

    libevdev_enable_property(dev, INPUT_PROP_POINTER);
    libevdev_enable_property(dev, INPUT_PROP_BUTTONPAD);

    libevdev_enable_event_type(dev, EV_ABS);
    libevdev_enable_event_code(dev, EV_ABS, ABS_X, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_Y, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_SLOT, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_POSITION_X, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_POSITION_Y, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_TOOL_TYPE, &absinfo);
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_TRACKING_ID, &absinfo);

    libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_FINGER, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_QUINTTAP, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOUCH, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_DOUBLETAP, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_TRIPLETAP, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_TOOL_QUADTAP, NULL);

    libevdev_enable_event_code(dev, EV_MSC, MSC_TIMESTAMP, NULL);

    struct libevdev_uinput *udev;
    if (libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED,
                                           &udev) < 0) {
        fprintf(stderr, "Could not create a uinput device");
        return 0;
    }
    sleep(3);

    int x = 100;
    int y = 100;
    int timestamp = 0;
    const int iterations = 10;

    for (int i = 0; i < iterations; i++) {
        for (int slot = 0; slot < 2; slot++) {
            libevdev_uinput_write_event(udev, EV_ABS, ABS_MT_SLOT, slot);
            if (i == 0) {
                libevdev_uinput_write_event(udev, EV_ABS, ABS_MT_TRACKING_ID,
                                            (slot + 1) * 100);
            } else if (i == iterations - 1) {
                libevdev_uinput_write_event(udev, EV_ABS, ABS_MT_TRACKING_ID,
                                            -1);
                continue;
            }

            libevdev_uinput_write_event(udev, EV_ABS, ABS_MT_POSITION_X,
                                        x + slot * 100);
            libevdev_uinput_write_event(udev, EV_ABS, ABS_MT_POSITION_Y, y);
        }
        if (i == 0) {
            libevdev_uinput_write_event(udev, EV_KEY, BTN_TOUCH, 1);
            libevdev_uinput_write_event(udev, EV_KEY, BTN_TOOL_DOUBLETAP, 1);
        } else if (i == iterations - 1) {
            libevdev_uinput_write_event(udev, EV_KEY, BTN_TOUCH, 0);
            libevdev_uinput_write_event(udev, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
        }
        libevdev_uinput_write_event(udev, EV_ABS, ABS_X, x);
        libevdev_uinput_write_event(udev, EV_ABS, ABS_Y, y);
        libevdev_uinput_write_event(udev, EV_MSC, MSC_TIMESTAMP, timestamp);
        libevdev_uinput_write_event(udev, EV_SYN, SYN_REPORT, 0);
        y += 5;
        x += 0;
        timestamp += 7000;

        usleep(10000);
    }

    return 0;
}