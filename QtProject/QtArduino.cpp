
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <QtCore/QDebug>

#include "util.h"

#include <QtArduino.h>
#include <QObject.h>

#include <QtSerialPort/QSerialPortInfo>

using namespace firmatator;

// Initialize communication with Arduino Uno
bool QtArduino::begin(QString portName)
{
    connected = false;

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            if(serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id){
                if(serialPortInfo.productIdentifier() == arduino_uno_product_id){
                    if(serialPortInfo.portName() == portName){      // Check if port selected is Arduino UNO
                        connected = true;
                    }
                }
            }
        }
    }

    if(connected){
        // open and configure the serialport
        serial.setPortName(portName);
        serial.setBaudRate(QSerialPort::Baud57600);
        serial.setDataBits(QSerialPort::Data8);
        serial.setParity(QSerialPort::NoParity);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setFlowControl(QSerialPort::NoFlowControl);
        serial.open(QIODevice::ReadWrite);
        connect(&serial, SIGNAL(readyRead()), this, SLOT(processSerial()));
    }
    return connected;
}

// Close connection
void QtArduino::end()
{
    serial.close();
    connected = false;
    ready = false;
}

// Change pin mode between INPUT/OUTPUT/PWM/etc.
void QtArduino::pinMode(int pin, int mode)
{
    QString st;
    char buffer[3];
    buffer[0] = COMMAND_SET_PIN_MODE;
    buffer[1] = pin;
    buffer[2] = mode;

    QByteArray s = QByteArray(buffer, 3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Pin: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Mode: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Request value digital pins
void QtArduino::digitalRead()
{
    QString st;
    for (int i = 0; i < 3; i++)
    {
        st.clear();

        char buffer[2];

        buffer[0] = COMMAND_REPORT_DIGITAL | i;
        buffer[1] = 1;

        QByteArray s = QByteArray(buffer, 2);

        st.append("Command: ");
        st.append(upperToHex(buffer[0]));
        st.append(lowerToHex(buffer[0]));
        st.append(" Port: ");
        st.append(upperToHex(buffer[1]));
        st.append(lowerToHex(buffer[1]));

        emit SerialData(st);

        serial.write(s);
        serial.flush();
    }
}

// Set a value in digital pins
void QtArduino::digitalWrite(int pin, int value)
{
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_SET_DIGITAL_VALUE;
    buffer[1] = pin;
    buffer[2] = value;

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Pin: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Value: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Array to collect digital data
int QtArduino::digitalData(int pin)
{
    return digitalInputData[pin];
}

// Command to read analog data
void QtArduino::analogRead(int pin, int value)
{
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_REPORT_ANALOG;
    buffer[1] = pin;
    buffer[2] = value;

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Array to pick up data analog
int  QtArduino::analogData(int pin)
{
    return analogInputData[pin];
}

// Change mode of pin AREF
void QtArduino::analogReference(int mode)
{
    int pin = 13;
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_ANALOG_REFERENCE;
    buffer[1] = pin;
    buffer[2] = mode;

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Send a value to PWM Pins
void QtArduino::analogWrite(int pin, int value)
{
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_ANALOG_MESSAGE | (pin & 0x0F);;
    buffer[1] = value & 0x7F;
    buffer[2] = value >> 7;

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Value: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();

}

// Generate a tone in a pin
void QtArduino::tone(int pin, int freq, int duration)
{
    QString st;
    char buffer [8];
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_TONE_CONFIG;
    buffer[2] = pin;
    buffer[3] = freq & 0x7F;
    buffer[4] = freq >> 7;
    buffer[5] = duration & 0x7F;
    buffer[6] = duration >> 7;
    buffer[7] = COMMAND_END_SYSEX;

    QByteArray s = QByteArray(buffer,8);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Pin: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));
    st.append(" Frequency: ");
    st.append(upperToHex(buffer[3]));
    st.append(lowerToHex(buffer[3]));
    st.append(" ");
    st.append(upperToHex(buffer[4]));
    st.append(lowerToHex(buffer[4]));
    st.append(" Duration: ");
    st.append(upperToHex(buffer[5]));
    st.append(lowerToHex(buffer[5]));
    st.append(" ");
    st.append(upperToHex(buffer[6]));
    st.append(lowerToHex(buffer[6]));
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[7]));
    st.append(lowerToHex(buffer[7]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Stop tone
void QtArduino::noTone(int pin)
{
    QString st;
    char buffer [4];
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_NOTONE_CONFIG;
    buffer[2] = pin;
    buffer[3] = COMMAND_END_SYSEX;

    QByteArray s = QByteArray(buffer,4);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Pin: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[3]));
    st.append(lowerToHex(buffer[3]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Set servo config
void QtArduino::ServoConfig(int pin,int minpul, int maxpul)
{
    QString st;
    char buffer [8];
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_SERVO_CONFIG;
    buffer[2] = pin;
    buffer[3] = minpul & 0x7F;
    buffer[4] = minpul >> 7;
    buffer[5] = maxpul & 0x7F;
    buffer[6] = maxpul >> 7;
    buffer[7] = COMMAND_END_SYSEX;

    QByteArray s = QByteArray(buffer,8);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Pin: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));
    st.append(" Minimum pulse width: ");
    st.append(upperToHex(buffer[3]));
    st.append(lowerToHex(buffer[3]));
    st.append(" ");
    st.append(upperToHex(buffer[4]));
    st.append(lowerToHex(buffer[4]));
    st.append(" Maximum pulse width: ");
    st.append(upperToHex(buffer[5]));
    st.append(lowerToHex(buffer[5]));
    st.append(" ");
    st.append(upperToHex(buffer[6]));
    st.append(lowerToHex(buffer[6]));
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[7]));
    st.append(lowerToHex(buffer[7]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Attach an interrupt in a digital pin (2/3)
void QtArduino::attachInterrupt(int interrupt, int mode)
{
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_INTERRUPT_CONFIG;
    buffer[1] = interrupt;
    buffer[2] = mode;

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Interrupt: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Mode: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Detach an interrupt in a digital pin (2/3)
void QtArduino::detachInterrupt(int interrupt)
{
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_INTERRUPT_CONFIG;
    buffer[1] = interrupt;
    buffer[2] = 4;              // Send other value for disable interrupt

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Interrupt: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Mode: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Send 0-6, 7-13, 14-20 for duration
void QtArduino::CaptureData(int mode, int duration)
{
    QString st;
    char buffer [7];
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_TIME_CAPTURE;
    buffer[2] = mode;
    buffer[3] = duration & 0x3FFF;
    buffer[4] = (duration >> 7) & 0x7F;
    buffer[5] = duration >> 14;
    buffer[6] = COMMAND_END_SYSEX;

    QByteArray s = QByteArray(buffer,7);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Mode: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));
    st.append(" Duration: ");
    st.append(upperToHex(buffer[3]));
    st.append(lowerToHex(buffer[3]));
    st.append(" ");
    st.append(upperToHex(buffer[4]));
    st.append(lowerToHex(buffer[4]));
    st.append(" ");
    st.append(upperToHex(buffer[5]));
    st.append(lowerToHex(buffer[5]));
    st.append(" ");
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[6]));
    st.append(lowerToHex(buffer[6]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

bool QtArduino::available()
{
    return connected && ready;
}

// Send data to I2C Slave
void QtArduino::I2CRequestWrite(int addr, QString text, int mode)
{
    QString st;
        int pos = 3;
        int len = text.length();
        char* buffer = (char*) malloc(5 + len*2);
        buffer[0] = COMMAND_START_SYSEX;
        buffer[1] = COMMAND_I2C_REQUEST;
        buffer[2] = addr;
        buffer[3] = mode;
        for (int i = 0; i < len; i++)
        {
            pos = pos + 1;
            buffer[pos] = text.at(i).toLatin1() & 0x7F;
            pos = pos + 1;
            buffer[pos] = text.at(i).toLatin1() >> 7;
        }
        pos = pos + 1;
        buffer[pos] = COMMAND_END_SYSEX;
        pos = pos + 1;

        QByteArray s = QByteArray(buffer,pos);

        st.append("Command: ");
        st.append(upperToHex(buffer[0]));
        st.append(lowerToHex(buffer[0]));
        st.append(" Command 1: ");
        st.append(upperToHex(buffer[1]));
        st.append(lowerToHex(buffer[1]));
        st.append(" Adress: ");
        st.append(upperToHex(buffer[2]));
        st.append(lowerToHex(buffer[2]));
        st.append(" Mode: ");
        st.append(upperToHex(buffer[3]));
        st.append(lowerToHex(buffer[3]));
        st.append(" Data: ");
        for(pos = 4; pos <(4+len*2); pos++){
            st.append(upperToHex(buffer[pos]));
            st.append(lowerToHex(buffer[pos]));
            st.append(" ");
        }
        st.append(" Command 2: ");
        st.append(upperToHex(buffer[pos]));
        st.append(lowerToHex(buffer[pos]));

        emit SerialData(st);

        serial.write(s);
        serial.flush();
}

// Read data from I2C device
void QtArduino::I2CRequestRead(int addr, int regist, int numBytes, int mode)
{
    if(mode == 8 || mode == 16){        // Mode discrete/continuous
        QString st;
        char buffer [9];
        buffer[0] = COMMAND_START_SYSEX;
        buffer[1] = COMMAND_I2C_REQUEST;
        buffer[2] = addr;
        buffer[3] = mode;
        buffer[4] = regist & 0x7F;
        buffer[5] = regist >> 7;
        buffer[6] = numBytes & 0x7F;
        buffer[7] = numBytes >> 7;
        buffer[8] = COMMAND_END_SYSEX;

        QByteArray s = QByteArray(buffer,9);

        st.append("Command: ");
        st.append(upperToHex(buffer[0]));
        st.append(lowerToHex(buffer[0]));
        st.append(" Command 1: ");
        st.append(upperToHex(buffer[1]));
        st.append(lowerToHex(buffer[1]));
        st.append(" Adress: ");
        st.append(upperToHex(buffer[2]));
        st.append(lowerToHex(buffer[2]));
        st.append(" Mode: ");
        st.append(upperToHex(buffer[3]));
        st.append(lowerToHex(buffer[3]));
        st.append(" Register: ");
        st.append(upperToHex(buffer[4]));
        st.append(lowerToHex(buffer[4]));
        st.append(" ");
        st.append(upperToHex(buffer[5]));
        st.append(lowerToHex(buffer[5]));
        st.append(" numBytes: ");
        st.append(upperToHex(buffer[6]));
        st.append(lowerToHex(buffer[6]));
        st.append(" ");
        st.append(upperToHex(buffer[7]));
        st.append(lowerToHex(buffer[7]));
        st.append(" Command 2: ");
        st.append(upperToHex(buffer[8]));
        st.append(lowerToHex(buffer[8]));

        emit SerialData(st);

        serial.write(s);
        serial.flush();
    }else if(mode == 24){       // Stop read
        QString st;
        char buffer [5];
        buffer[0] = COMMAND_START_SYSEX;
        buffer[1] = COMMAND_I2C_REQUEST;
        buffer[2] = addr;
        buffer[3] = mode;
        buffer[4] = COMMAND_END_SYSEX;

        QByteArray s = QByteArray(buffer,5);

        st.append("Command: ");
        st.append(upperToHex(buffer[0]));
        st.append(lowerToHex(buffer[0]));
        st.append(" Command 1: ");
        st.append(upperToHex(buffer[1]));
        st.append(lowerToHex(buffer[1]));
        st.append(" Adress: ");
        st.append(upperToHex(buffer[2]));
        st.append(lowerToHex(buffer[2]));
        st.append(" Mode: ");
        st.append(upperToHex(buffer[3]));
        st.append(lowerToHex(buffer[3]));
        st.append(" Command 2: ");
        st.append(upperToHex(buffer[4]));
        st.append(lowerToHex(buffer[4]));

        emit SerialData(st);

        serial.write(s);
        serial.flush();
    }
}

// Store data from I2C device
int QtArduino::SlaveDataI2C(int position)
{
    return SlaveData[position];
}

// Set I2C config
void QtArduino::I2CConfig(int pinState, int delay)
{
    QString st;
    char buffer [7];
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_I2C_CONFIG;
    buffer[2] = delay & 0x7F;
    buffer[3] = delay >> 7;
    buffer[4] = pinState & 0x7F;
    buffer[5] = pinState >> 7;
    buffer[6] = COMMAND_END_SYSEX;

    QByteArray s = QByteArray(buffer,7);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Delay: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));
    st.append(" ");
    st.append(upperToHex(buffer[3]));
    st.append(lowerToHex(buffer[3]));
    st.append(" Enable power Pins: ");
    st.append(upperToHex(buffer[4]));
    st.append(lowerToHex(buffer[4]));
    st.append(" ");
    st.append(upperToHex(buffer[5]));
    st.append(lowerToHex(buffer[5]));
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[6]));
    st.append(lowerToHex(buffer[6]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Send data to I2C slave
void QtArduino::SendString(QString text)
{
    QString st;
    int pos = 1;
    int len = text.length();
    char* buffer = (char*) malloc(3 + len*2);
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_STRING_DATA;
    for (int i = 0; i < len; i++)
    {
        pos = pos + 1;
        buffer[pos] = text.at(i).toLatin1() & 0x7F;
        pos = pos + 1;
        buffer[pos] = text.at(i).toLatin1() >> 7;
    }
    pos = pos + 1;
    buffer[pos] = COMMAND_END_SYSEX;
    pos = pos + 1;

    QByteArray s = QByteArray(buffer,pos);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Data: ");
    for(pos = 2; pos <(2+len*2); pos++){
        st.append(upperToHex(buffer[pos]));
        st.append(lowerToHex(buffer[pos]));
        st.append(" ");
    }
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[pos]));
    st.append(lowerToHex(buffer[pos]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Search devices
void QtArduino::I2C_Scan()
{
    QString st;
    char buffer [3];
    buffer[0] = COMMAND_START_SYSEX;
    buffer[1] = COMMAND_I2C_SCAN;
    buffer[2] = COMMAND_END_SYSEX;

    QByteArray s = QByteArray(buffer,3);

    st.append("Command: ");
    st.append(upperToHex(buffer[0]));
    st.append(lowerToHex(buffer[0]));
    st.append(" Command 1: ");
    st.append(upperToHex(buffer[1]));
    st.append(lowerToHex(buffer[1]));
    st.append(" Command 2: ");
    st.append(upperToHex(buffer[2]));
    st.append(lowerToHex(buffer[2]));

    emit SerialData(st);

    serial.write(s);
    serial.flush();
}

// Collect address of slave
int QtArduino::I2CAddr()
{
    return SlaveAddr[0];
}

// Process incoming messages
void QtArduino::processSerial()
{
        QByteArray r = this->serial.readAll();      // Read all data

        for (int i = 0; i < r.length(); i++)
        {
            uint8_t c = r[i];
            uint8_t msn = c & 0xF0;

            if (msn == COMMAND_ANALOG_MESSAGE || msn == COMMAND_DIGITAL_MESSAGE)
            {
                parserCommandLenght = 3;
                parserReceivedCount = 0;
            }
            else if (c == COMMAND_INTERRUPT_CAPTURED)
            {
                parserCommandLenght = 2;
                parserReceivedCount = 0;
            }
            else if (c == COMMAND_TIME_TONE)
            {
                parserCommandLenght = 1;
                parserReceivedCount = 0;
            }
            else if (c == COMMAND_END_DATA)
            {
                parserCommandLenght = 1;
                parserReceivedCount = 0;
            }
            else if (c == COMMAND_START_SYSEX)
            {
                parserCommandLenght = 4096;
                parserReceivedCount = 0;
            }
            else if (c == COMMAND_END_SYSEX)
            {
                parserCommandLenght = parserReceivedCount + 1;
            }

            if (parserReceivedCount <= parserCommandLenght)
            {
                parserBuffer[parserReceivedCount] = c;
                parserReceivedCount++;
                if (parserReceivedCount == parserCommandLenght)
                {
                    receiving = false;
                    parseBuffer();      // Go to function below
                    parserReceivedCount = parserCommandLenght = 0;
                }
            }
            else if (parserReceivedCount >= parserCommandLenght - 1)
            {
                receiving = false;
                parserReceivedCount = parserCommandLenght = 0;
            }
        }
}

// Develop stored messages
void QtArduino::parseBuffer()
{
    uint8_t cmd = (parserBuffer[0] & 0xF0);
    uint8_t cmdInt = parserBuffer[0];
    int listoUpdate = false;
    if (cmd == COMMAND_ANALOG_MESSAGE && parserReceivedCount == 3)
    {
            int analog_ch = (parserBuffer[0] & 0x0F);                   // Save analog pin
            int analog_val = parserBuffer[1] | (parserBuffer[2] << 7);  // Save value pin

            analogInputData[analog_ch] = analog_val;                    // Store in array

            if(analog_ch == 5){                                         // Finish data
                listoUpdate = true;
                emit(UpdateData(listoUpdate));
            }
    }else if (cmd == COMMAND_DIGITAL_MESSAGE && parserReceivedCount == 3)
    {
            int port_num = (parserBuffer[0] & 0x0F);
            int port_val = parserBuffer[1] | (parserBuffer[2] << 7);
            int pin = port_num * 8;

            for (int mask=1; mask & 0xFF; mask <<= 1, pin++)
            {
                    uint32_t val = (port_val & mask) ? 1 : 0;
                    digitalInputData[pin] = val;                        // Save value digital pin
            }
            if(pin == 16){
                listoUpdate = false;
                emit(UpdateData(listoUpdate));
            }
    }else if(cmdInt == COMMAND_INTERRUPT_CAPTURED && parserReceivedCount == 2)
    {
        int InterruptNumber = parserBuffer[1];

        emit(IntCaptured(InterruptNumber));         // Signal with interrupt captured
    }
    else if(cmdInt == COMMAND_END_DATA && parserReceivedCount == 1)
    {
        emit(EndReceiveData(true));
    }
    else if(cmdInt == COMMAND_TIME_TONE && parserReceivedCount == 1)
    {
        emit(EndTimeTone(true));                    // Finish tone
    }
    else if (parserBuffer[0] == COMMAND_START_SYSEX && parserBuffer[parserReceivedCount - 1] == COMMAND_END_SYSEX)      // Process data from I2C slave
    {
        if (parserBuffer[1] == COMMAND_I2C_REPLY)
        {
            SlaveData[0] = parserBuffer[2] | (parserBuffer[3] << 7);
            SlaveData[1] = parserBuffer[4] | (parserBuffer[5] << 7);
            int pos = 2;
            for(int i = 6; i < parserReceivedCount - 1; i = i + 2){
                SlaveData[pos] = parserBuffer[i] | (parserBuffer[i+1] << 7);
                pos = pos + 1;
            }
            emit(ReadI2C(pos));
        } else if (parserBuffer[1] == COMMAND_I2C_SCAN)
        {
            SlaveAddr[0] = parserBuffer[2] | (parserBuffer[3] << 7);
            emit(I2CDeviceDetect(true));
        }
    }
}
