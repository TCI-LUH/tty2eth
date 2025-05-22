# tty2eth firmware


This project contains the management module firmware of the [Gateway-Module-v3](https://github.com/TCI-LUH/Gateway-Module-v3).

## build

```bash
mkdir build
cd build
cmake ..
make -j8
```

## flush

```bash
make -j8 flush
```

## requirements

- arm-none-eabi-gcc toolchain
- openocd
- stlink