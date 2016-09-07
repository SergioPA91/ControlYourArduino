#ifndef QTARDUINO_H
#define QTARDUINO_H

#include <string>
#include <QObject>
#include <stdint.h>

#include <QSerialPort>
#include <QSerialPortInfo>

namespace firmatator
{
    class QtArduino : public QObject
    {
        Q_OBJECT

        int name;

        // Firmata protocol vars

        std::string firmataName;
        bool ready;

        // Inputs / outputs status

        int analogInputData[20];
        int digitalInputData[20];
        int digitalOutputData[20];
        int SlaveData[4096];
        int SlaveAddr[8];

        // Serial connection vars

        bool connected;
        QString port;
        int baud_rate;
        QSerialPort serial;
        uint8_t parserBuffer[20];
        int parserCommandLenght;
        int parserReceivedCount;
        bool receiving;

        static const quint16 arduino_uno_vendor_id = 9025;
        static const quint16 arduino_uno_product_id = 67;

    public:

        // Firmata protocol constants

        static const int MAX_DATA_BYTES                  = 64;

        static const int PINMODE_INPUT                   = 0;
        static const int PINMODE_OUTPUT                  = 1;
        static const int PINMODE_ANALOG                  = 2;
        static const int PINMODE_PWM                     = 3;
        static const int PINMODE_SERVO                   = 4;
        static const int PINMODE_SHIFT                   = 5;
        static const int PINMODE_I2C                     = 6;
        static const int PINMODE_PULLUP                  = 11;

        static const int DIGITAL_LOW                     = 0;
        static const int DIGITAL_HIGH                    = 1;

        static const int I2C_MODE_7_BITS                 = 0;
        static const int I2C_MODE_10_BITS                = 1;

        static const int I2C_MODE_WRITE                  = 0;
        static const int I2C_MODE_READ_ONCE              = 1;
        static const int I2C_MODE_READ_CONSTINOUSLY      = 2;
        static const int I2C_MODE_READ_STOP              = 3;

        static const int DEFAULT                         = 0;
        static const int EXTERNAL                        = 1;
        static const int INTERNAL                        = 2;

        static const int COMMAND_END_DATA                = 0xF1; // set config for interrupts (pin 2 and 3)
        static const int COMMAND_INTERRUPT_CONFIG        = 0xF2; // set config for interrupts (pin 2 and 3)
        static const int COMMAND_INTERRUPT_CAPTURED      = 0xF3; // set config for interrupts (pin 2 and 3)
        static const int COMMAND_SET_PIN_MODE            = 0xF4; // set a pin to INPUT/OUTPUT/PWM/etc
        static const int COMMAND_SET_DIGITAL_VALUE       = 0xF5; // set a value to digital pin
        static const int COMMAND_ANALOG_REFERENCE        = 0xF6; // Set config pin aref
        static const int COMMAND_DIGITAL_MESSAGE         = 0x90; // send data for a digital port
        static const int COMMAND_ANALOG_MESSAGE          = 0xE0; // send data for an analog pin (or PWM)
        static const int COMMAND_REPORT_ANALOG           = 0xC0; // enable analog input by pin #
        static const int COMMAND_REPORT_DIGITAL          = 0xD0; // enable digital input by port
        static const int COMMAND_SERVO_CONFIG            = 0x70; // set the parameters for a servo pin (max and min pulse, max and min angle)
        static const int COMMAND_TIME_TONE               = 0xF8;

        static const int COMMAND_START_SYSEX             = 0xF0; // start a MIDI SysEx message
        static const int COMMAND_END_SYSEX               = 0xF7; // end a MIDI SysEx message

        static const int COMMAND_I2C_REQUEST             = 0x76;
        static const int COMMAND_I2C_REPLY               = 0x77;
        static const int COMMAND_I2C_CONFIG              = 0x78;
        static const int COMMAND_I2C_SAMPLING_INTERVAL   = 0x7A;

        static const int COMMAND_I2C_SCAN                = 0x56; // scan I2C connected devices
        static const int COMMAND_TIME_CAPTURE            = 0x57; // set time capture data analog/digital
        static const int COMMAND_NOTONE_CONFIG           = 0x58; // set noTone function
        static const int COMMAND_TONE_CONFIG             = 0x59; // set pin, freq, duration for tone function
        static const int COMMAND_STRING_DATA             = 0x71; // send string

        // Methods
        bool begin(QString portName);
        void end();
        bool available();

        void pinMode(int pin, int mode);
        void digitalWrite(int pin, int value);
        int  digitalData(int pin);
        void analogRead(int pin, int value);
        int  analogData(int pin);
        void analogWrite(int pin, int value);
        void digitalRead();
        void analogReference(int mode);
        void tone(int pin, int freq, int duration);
        void noTone(int pin);
        void ServoConfig(int pin,int minpul, int maxpul);
        void attachInterrupt(int interrupt, int mode);
        void detachInterrupt(int interrupt);
        void CaptureData(int mode, int duration);

        void I2CRequestWrite(int addr, QString text, int mode);
        void I2CRequestRead(int addr, int regist, int numBytes, int mode);
        int  SlaveDataI2C(int position);
        void I2CConfig(int pinState, int delay);
        void SendString(QString text);
        void I2C_Scan();
        int  I2CAddr();

    public slots:

        void processSerial();
        void parseBuffer();

    signals:

        void deviceReady();
        void SerialData(const QString text);
        void UpdateData(const bool listo);
        void IntCaptured(const int number);
        void EndReceiveData(const bool stop);
        void EndTimeTone(const bool stop);
        void ReadI2C(const int length);
        void I2CDeviceDetect(const bool detected);
    };
}

#endif // QTARDUINO

