# Arduino UNO FreeRTOS example
A minimal startingpoint to work with FreeRTOS on Arduino UNO using only cmake and avr-gcc (not using the arduino IDE)

## Build
Simply configure and build using `cmake`:
```sh
cmake --preset avr -DCMAKE_BUILD_TYPE=Release
cmake --build build/avr
```
You should now have a folder called `build/avr/` with a flashable hex-file in it.

## Flash using `avrdude`
You might need to add `sudo` and change the device depending on your setup
```sh
# new BL (your device might ship with this)
avrdude -v -p atmega328p -c arduino -P /dev/ttyACM0 -b 57600 -D -U flash:w:build/avr/unofreertos.hex:i
# old BL (optiboot (automatically downloaded on cmake configure step))
avrdude -v -p atmega328p -c arduino -P /dev/ttyACM0 -b 115200 -D -U flash:w:build/avr/unofreertos.hex:i
```

## If `avrdude` does not work
You might have to use the ICSP port, as the bootloader might've been overwritten.

### Using a raspberry pi as an ICSP programmer
Based on [this article](https://learn.adafruit.com/program-an-avr-or-arduino-using-raspberry-pi-gpio-pins/overview).

**Requirements:**
 - Raspberry Pi (preferably one with WiFi (i.e. 3 model B+))
    - Note that you can use `rpi-imager` to autoconfigure your rpi. 
      No need for a seperate display and keyboard, as you can enable ssh server in there.
      Make sure to pick the "no desktop" option. Not just the recommended one
 - 6 female-female jumper wires
 - a hex file with a freaking bootloader on it. See [optiboot](https://github.com/Optiboot/optiboot)

Check the `/etc/avrdude_gpio.conf` configuration. It might not be up to date. Make sure it has the following programmer configuration:
```
programmer # raspberry_pi_gpio
    id                     = "raspberry_pi_gpio";
    desc                   = "Bitbang Raspberry Pi GPIO via linuxgpio (sysfs or libgpiod)";
    type                   = "linuxgpio";
    prog_modes             = PM_ISP;
    connection_type        = linuxgpio;
    reset                  = 22;
    sck                    = 24;
    sdo                    = 23;
    sdi                    = 18;
;
```

Remember SPI data pins are not bi-directional buffers, so SDI pin on one chip should always be connected to SDO (or MOSI or COPI) pin on the other chip. Check the [arduino pinout](https://content.arduino.cc/assets/Pinout-UNOrev3_latest.pdf) and the rpi pinout (see below)

![](doc/GPIO.png)
