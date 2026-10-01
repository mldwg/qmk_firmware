# 0xCB New Horizons

65% Keyboard

> **Unofficial port.** This firmware is a community port of the [Vial](https://get.vial.today) firmware for the New Horizons to upstream QMK. It is not created, maintained, or supported by 0xCB — please don't contact 0xCB about issues with this firmware.

* Port Maintainer: [Matteo Ludwig](https://github.com/mldwg)
* Original Keyboard Firmware: [Conor Burns](https://github.com/conor-burns) (Vial)
* Hardware Supported: https://github.com/0xcb-dev/0xCB-NewHorizons
* Hardware Availability: On [KeebSupply](https://keeb.supply/products/0xcb-new-horizons)

![](https://keeb.supply/assets/nh-full.webp)

![](https://keeb.supply/assets/nh-bottom.webp)

To go to bootloader press ESC while plugging in the board.

Make example for this keyboard using an [RP2040 controller](https://keeb.supply/products/0xcb-helios) (after setting up your build environment):

    make 0xcb/newhorizons/rp2040_ce:default

Flashing example for this keyboard:

    make 0xcb/newhorizons/rp2040_ce:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
