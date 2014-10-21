TARGET       = Aiakos
ARDUINO_LIBS = RadioHead RDM630 SPI SoftwareSerial ATECC108

MCU          = atmega328p
F_CPU        = 16000000

# Avrdude code
ARDUINO_PORT = /dev/ttyACM0
AVRDUDE_ARD_PROGRAMMER = arduino
AVRDUDE_ARD_BAUDRATE = 115200

USER_LIB_PATH := .

include /usr/share/arduino/Arduino.mk
