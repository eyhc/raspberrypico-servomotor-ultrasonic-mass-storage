# raspberrypico-servomotor-ultrasonic-mass-storage
Servomotor controlled by ultrasonic sensor and configurable via USB Mass Storage

## Electronic board

![electronic circuit](img/circuit.png)

## Build-it

```sh
git submodule update --init 
cd pico-sdk
git submodule update --init 
cd ..
mkdir build
cd build
cmake ..
make
```

## How to flash the firmware

- keep pressing on bootsel
- connect your PC to the controller and stop pressing bootsel
- the controller will be recognized as a USB storage device
- copy the file main.uf2 or test-*.uf2 on the root of this USB storage device
- the firmware is now flashed and starts/boots

## Resources

  - https://www.overleaf.com/read/kbxggsdyydsm#3743ae
  - https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html
  - https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
  - https://docs.tinyusb.org/en/latest/
  - https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__descriptors.html
  - http://elm-chan.org/fsw/ff/ and https://github.com/abbrev/fatfs
  - https://github.com/benhoyt/inih
