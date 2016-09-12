# Graphical User Interface for Arduino

First of all, I have used part of code from this repository https://github.com/dh1tw/firmatator and this project is based on Firmata, a protocol for communicating with microcontrollers from software on a host computer. You found it here: https://github.com/firmata/arduino.

## Description

This project is seeking to create a graphical user interface for Arduino UNO board using QT Software (https://www.qt.io/), in particular, Qt Creator.

With this program, you can controller your Arduino UNO almost entirely. The main features are described below:

* Read/Write digital/analog inputs/outputs.
* Configure the specified pin to behave as an input, output, PWM, servo etc.
* Configure the reference voltage used for analog input (pin AREF).
* Visualize analog/digital data on a graph during a specific time or real time.
* Generate a square wave of the specified frequency (and 50% duty cycle) on a pin (tone/notone).
* Control of servomotors.
* Capture on external interrupt (pin 2 and 3).
* Configure I2C protocol. Send/Receive data from I2C devices. For example, you can send text to LCD.

![Alt Text](https://github.com/SergioPA91/ControlYourArduino/blob/master/ControlYourArduino.gif)

## How to use on IDE Arduino

Copy folder "FirmataQt" and "LiquidCrystal_I2C" to ... "C:\Program Files (x86)\Arduino\libraries" and enjoy it!

## License

ControlYourArduino - Arduino program and Qt library implementation of the Firmata protocol.

Copyright (C) 2016     Sergio Pérez Alhama

SPerezAlhama@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or 
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the Lesser GNU General Public License
along with this program; If not, see <http://www.gnu.org/licenses/>.

