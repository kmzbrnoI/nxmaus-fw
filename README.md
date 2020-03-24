# FW for NXmaus throttle

This repository contains firmware for ATmega88 AVR microprocessor located in
NXmaus XpressNET throttle for model railroad control.

## Build & requirements

This firmware is developed in C language, compiled via `avr-gcc` with help
of `make`. You may also find tools like `avrdude` helpful.

Hex files are available in *Releases* section.

## Programming

NXmaus PCS contains no programming connector, however the processor is in
DIL. Intended programming method is to program microprocessor in external
programmer and then put it to NXmaus.

This FW uses EEPROM, however no loading of EEPROM is required. There should
be just an empty EEPROM.

## Author's toolkit

Text editor + `make`. No more, no less.

## Authors

 * Jan Horacek ([jan.horacek@kmz-brno.cz](mailto:jan.horacek@kmz-brno.cz))

## License

This application is released under the [Apache License v2.0
](https://www.apache.org/licenses/LICENSE-2.0).
