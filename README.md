## Shortcuts for rapid compilation
F4: flash app
F5: build app
F8: clean app
F12: menuconfig

### Configure the project

```
make menuconfig
```

* Set serial port under Serial Flasher Options.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
make flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)
