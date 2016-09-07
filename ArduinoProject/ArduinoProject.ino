#include <FirmataQt.h>      // Firmata library modified for this project.
#include <Servo.h>          // Servo library
#include <Wire.h>           // I2C library
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define ARDUINO_DEFAULT      0    // The default analog reference of 5 volts (on 5V Arduino boards) or 3.3 volts (on 3.3V Arduino boards)
#define ARDUINO_EXTERNAL     1    // The voltage applied to the AREF pin (0 to 5V only) is used as the reference.
#define ARDUINO_INTERNAL     2    // An built-in reference, equal to 1.1 volts on the ATmega168 or ATmega328 and 2.56 volts on the ATmega8 (not available on the Arduino Mega)

#define INTERRUPT_LOW        0    // To trigger the interrupt whenever the pin is low,
#define INTERRUPT_CHANGE     1    // To trigger the interrupt whenever the pin changes value
#define INTERRUPT_RISING     2    // To trigger when the pin goes from low to high,
#define INTERRUPT_FALLING    3    // For when the pin goes from high to low.
#define DISABLE_INTERRUPT    4    // For disable interrupt

// Commands for I2C basic functions
#define I2C_WRITE B00000000
#define I2C_READ B00001000
#define I2C_READ_CONTINUOUSLY B00010000
#define I2C_STOP_READING B00011000
#define I2C_READ_WRITE_MODE_MASK B00011000

#define MAX_QUERIES 8                   // Max requests
#define REGISTER_NOT_SPECIFIED -1

#define I2C_ADDR    0x27                // Address for LCD 16x2
LiquidCrystal_I2C             lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);   // Config for LCD 16x2

unsigned int samplingInterval = 100;    // default sampling interval is 100ms
unsigned int i2cReadDelayTime = 0;      // default delay time between i2c read request and Wire.requestFrom()
unsigned int powerPinsEnabled = 0;      // use as boolean to prevent enablePowerPins from being called more than once

struct i2c_device_info {
  byte addr;
  byte reg;
  byte bytes;
};
i2c_device_info query[MAX_QUERIES];

byte i2cRxData[32];                     // Array to save data from I2C device
boolean readingContinuously = false;
byte queryIndex = 0;

int analogPin = 0;
int PinAnalog;

// variables to finish tone when time is specified
bool ReportEndTone = false;
int timeTone = 0;
unsigned long t_current = 0;
unsigned long t_updated = 0;

// boolean values for command time capture
volatile boolean Start = false;
volatile boolean Control = true;
volatile boolean DataAnalog = false;
volatile boolean DataDigital = false;
volatile boolean RealTime = false;

unsigned long currentMillis;         // store current value from millis()
unsigned long currentMillisI2C;
unsigned long previousMillis = 0;    // for comparison with currentMillis
unsigned long previousMillisI2C = 0;

unsigned long T0 = 0 ;               // Global variable for time
unsigned long T1 = 0 ;               // Global variable for time

unsigned long capture = 0;

// Variables to servo config
Servo servos[MAX_SERVOS];
byte servoPinMap[TOTAL_PINS];
byte detachedServos[MAX_SERVOS];
byte detachedServoCount = 0;
byte servoCount = 0;

unsigned int MinimumP = 544;            // Global variable to map servo values
unsigned int MaximumP = 2400;


void setPinModeCallback(byte pin, int mode)       // Change pin mode between INPUT/OUTPUT/PWM/etc.
{
  if (IS_PIN_DIGITAL(pin) && mode != PIN_MODE_SERVO) {
    if (servoPinMap[pin] < MAX_SERVOS && servos[servoPinMap[pin]].attached()) {   // To remove pin from servoPinMap
      detachServo(pin);
    }
  }
  switch (mode) {
    case PIN_MODE_ANALOG:
      if (IS_PIN_ANALOG(pin)) {
        if (IS_PIN_DIGITAL(pin)) {
          pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
          // deprecated since Arduino 1.0.1 - TODO: drop support in Firmata 2.6
          digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
        }
        Firmata.setPinMode(pin, PIN_MODE_ANALOG);
      }
      break;
    case INPUT:
      if (IS_PIN_DIGITAL(pin)) {
        pinMode(PIN_TO_DIGITAL(pin), INPUT);    // disable output driver
#if ARDUINO <= 100
        digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable internal pull-ups
#endif
        Firmata.setPinMode(pin, INPUT);
      }
      break;
    case PIN_MODE_PULLUP:
      if (IS_PIN_DIGITAL(pin)) {
        pinMode(PIN_TO_DIGITAL(pin), INPUT_PULLUP);
        Firmata.setPinMode(pin, PIN_MODE_PULLUP);
        Firmata.setPinState(pin, 1);
      }
      break;
    case OUTPUT:
      if (IS_PIN_DIGITAL(pin)) {
        digitalWrite(PIN_TO_DIGITAL(pin), LOW); // disable PWM
        pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
        Firmata.setPinMode(pin, OUTPUT);
      }
      break;
    case PIN_MODE_PWM:
      if (IS_PIN_PWM(pin)) {
        pinMode(PIN_TO_PWM(pin), OUTPUT);
        analogWrite(PIN_TO_PWM(pin), 0);
        Firmata.setPinMode(pin, PIN_MODE_PWM);
      }
      break;
    case PIN_MODE_SERVO:
      if (IS_PIN_DIGITAL(pin)) {
        Firmata.setPinMode(pin, PIN_MODE_SERVO);
        if (servoPinMap[pin] == 255 || !servos[servoPinMap[pin]].attached()) {
          // pass -1 for min and max pulse values to use default values set
          // by Servo library
          attachServo(pin, -1, -1);
        }
      }
      break;
  }
}

void setDigitalPinValueCallback(byte pin, int value)      // Set a value for digital pins
{
  if (IS_PIN_DIGITAL(pin)) {
    digitalWrite(PIN_TO_DIGITAL(pin), value);
  }
}

void analogWriteCallback(byte pin, int value)   // Write a value on a pin with pwm or servo mode
{
  if (pin < TOTAL_PINS) {
    switch (Firmata.getPinMode(pin)) {
      case PIN_MODE_PWM:
        if (IS_PIN_PWM(pin)) {
          analogWrite(PIN_TO_PWM(pin), value);
        }
        Firmata.setPinState(pin, value);
        break;
      case PIN_MODE_SERVO:
        if (IS_PIN_DIGITAL(pin)) {
          int val = map(value, 0, 180, MinimumP, MaximumP); // +info: https://learn.adafruit.com/16-channel-pwm-servo-driver/using-the-adafruit-library
          servos[servoPinMap[pin]].write(val);
        }
        Firmata.setPinState(pin, value);
        break;
    }
  }
}


void ReportAnalogCallback(byte analogPin, int value)    // Return analog value of six pins
{
  //Envía una única vez la lectura de los pines analógicos
  for (PinAnalog = 0; PinAnalog < 6; PinAnalog++) {
    Firmata.sendAnalog(PinAnalog, analogRead(PinAnalog));
  }
}

void reportDigitalCallback(byte port, int value)          // Reporting of a digital port // +info:https://www.arduino.cc/en/Reference/PortManipulation
{
  if (port < TOTAL_PORTS) {
    outputPort(port, readPort(port, readPort(port, 0xff)));
  }
}

void outputPort(byte portNumber, byte portValue)          //D0 to D7 (port = 0); D8 to D13 (port = 1); D14 to D19 (port = 2)
{
  Firmata.sendDigitalPort(portNumber, portValue);
}

void reportAnalogReferenceCallback(byte pin, int mode)    // Configure reference voltage used for analog input
{
  switch (mode) {
    case ARDUINO_DEFAULT:
      analogReference(DEFAULT);
      break;
    case ARDUINO_EXTERNAL:
      analogReference(EXTERNAL);
      break;
    case ARDUINO_INTERNAL:
      analogReference(INTERNAL);
      break;
  }
}

void reportInterruptConfigCallback(byte pin, int mode)      // Digital Pins With Interrupts; Set mode or disable interrupts
{
  switch (mode) {
    case INTERRUPT_LOW:
      if (pin == 0) {
        attachInterrupt(pin, ButtonServiceInt0, LOW);
      } else {
        attachInterrupt(pin, ButtonServiceInt1, LOW);
      }
      break;
    case INTERRUPT_CHANGE:
      if (pin == 0) {
        attachInterrupt(pin, ButtonServiceInt0, CHANGE);
      } else {
        attachInterrupt(pin, ButtonServiceInt1, CHANGE);
      }
      break;
    case INTERRUPT_RISING:
      if (pin == 0) {
        attachInterrupt(pin, ButtonServiceInt0, RISING);
      } else {
        attachInterrupt(pin, ButtonServiceInt1, RISING);
      }
      break;
    case INTERRUPT_FALLING:
      if (pin == 0) {
        attachInterrupt(pin, ButtonServiceInt0, FALLING);
      } else {
        attachInterrupt(pin, ButtonServiceInt1, FALLING);
      }
      break;
    case DISABLE_INTERRUPT:
      detachInterrupt(pin);
  }
}

void ButtonServiceInt0()        // The ISR to call when the interrupt occurs
{
  if (millis() > T0  + 250) {
    Firmata.write(INTERRUPT_CAPTURED);
    Firmata.write(0x00);
    T0 = millis();
  }
}

void ButtonServiceInt1()      // The ISR to call when the interrupt occurs
{
  if (millis() > T1  + 250) {
    Firmata.write(INTERRUPT_CAPTURED);
    Firmata.write(0x01);
    T1 = millis();
  }
}

void attachServo(byte pin, int minPulse, int maxPulse)  // Attach the Servo variable to a pin.
{
  if (servoCount < MAX_SERVOS) {
    // reuse indexes of detached servos until all have been reallocated
    if (detachedServoCount > 0) {
      servoPinMap[pin] = detachedServos[detachedServoCount - 1];
      if (detachedServoCount > 0) detachedServoCount--;
    } else {
      servoPinMap[pin] = servoCount;
      servoCount++;
    }
    if (minPulse > 0 && maxPulse > 0) {
      servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin), minPulse, maxPulse);
    } else {
      servos[servoPinMap[pin]].attach(PIN_TO_DIGITAL(pin));
    }
  }
}

void detachServo(byte pin)   // Detach the Servo variable from its pin. If all Servo variables are detached, then pins 9 and 10 can be used for PWM output with analogWrite().
{
  servos[servoPinMap[pin]].detach();
  // if we're detaching the last servo, decrement the count
  // otherwise store the index of the detached servo
  if (servoPinMap[pin] == servoCount && servoCount > 0) {
    servoCount--;
  } else if (servoCount > 0) {
    // keep track of detached servos because we want to reuse their indexes
    // before incrementing the count of attached servos
    detachedServoCount++;
    detachedServos[detachedServoCount - 1] = servoPinMap[pin];
  }
  servoPinMap[pin] = 255;
}

void attachTone(byte pin, int freq, int duration)   // Generates a square wave of the specified frequency (and 50% duty cycle) on a pin. A duration can be specified, otherwise the wave continues until a call to noTone().
{
  if (duration > 0) {
    tone(pin, freq * 4, duration);
    timeTone = duration;
    ReportEndTone = true;
    t_current = millis();
    t_updated = t_current + timeTone;
  } else {
    tone(pin, freq * 4);
  }
}

void readAndReportData(byte address, int theRegister, byte numBytes)    // Read data from I2C device
{
  if (theRegister != REGISTER_NOT_SPECIFIED) {
    Wire.beginTransmission(address);
    Wire.write((byte)theRegister);
    Wire.endTransmission();
    delayMicroseconds(i2cReadDelayTime);  // delay is necessary for some devices such as WiiNunchuck
  } else {
    theRegister = 0;                      // fill the register with a dummy value
  }

  Wire.requestFrom(address, numBytes);

  // check to be sure correct number of bytes were returned by slave
  if (numBytes == Wire.available()) {
    i2cRxData[0] = address;
    i2cRxData[1] = theRegister;
    for (int i = 0; i < numBytes; i++) {
      i2cRxData[2 + i] = Wire.read();
    }
    // send slave address, register and received bytes
    Firmata.sendSysex(I2C_REPLY, numBytes + 2, i2cRxData);
  }
}

void stringCallback(char *myString)       // Write string to I2C device slave
{
  String text = myString;
  lcd.clear();
  lcd.home ();                   // go home
  lcd.print(text);
  if (text.length() >= 16) {
    text.remove(0, 16);
    lcd.setCursor (0, 1);       // go to the 2nd line
    lcd.print(text);
  }
}

void sysexCallback(byte command, byte argc, byte *argv)     // get some commands to configure I2C, tone, servo... etc.
{
  byte mode;
  byte slaveAddress;
  byte slaveRegister;
  byte data;
  int  delayTime;

  switch (command) {
    case I2C_REQUEST:     //+Info: http://wiring.org.co/learning/libraries/i2cfirmata.html   http://www.techbitar.com/how-to-network-many-arduinos-with-sensors-using-i2c.html
      {
        mode = argv[1] & I2C_READ_WRITE_MODE_MASK;
        slaveAddress = argv[0];

        switch (mode) {
          case I2C_WRITE:
            Wire.beginTransmission(slaveAddress);
            for (byte i = 2; i < argc; i += 2) {
              data = argv[i] + (argv[i + 1] << 7);
              Wire.write(data);
            }
            Wire.endTransmission();
            delayMicroseconds(70);
            break;
          case I2C_READ:
            if (argc == 6) {
              // a slave register is specified
              slaveRegister = argv[2] + (argv[3] << 7);
              data = argv[4] + (argv[5] << 7);  // bytes to read
              if (slaveRegister == -1) {      
                readAndReportData(slaveAddress, (int)REGISTER_NOT_SPECIFIED, data);
              } else {
                readAndReportData(slaveAddress, (int)slaveRegister, data);
              }
            } else {
              // a slave register is NOT specified
              data = argv[2] + (argv[3] << 7);  // bytes to read
              readAndReportData(slaveAddress, (int)REGISTER_NOT_SPECIFIED, data);
            }
            break;
          case I2C_READ_CONTINUOUSLY:
            if ((queryIndex + 1) >= MAX_QUERIES) {
              // too many queries, just ignore
              break;
            }
            query[queryIndex].addr = slaveAddress;
            query[queryIndex].reg = argv[2] + (argv[3] << 7);
            query[queryIndex].bytes = argv[4] + (argv[5] << 7);
            readingContinuously = true;
            queryIndex++;
            break;
          case I2C_STOP_READING:
            readingContinuously = false;
            queryIndex = 0;
            break;
        }
        break;
      }

    case I2C_CONFIG:
      {
        delayTime = (argv[0] + (argv[1] << 7));

        if ((argv[2] + (argv[3] << 7)) > 0) {
          enablePowerPins(PORTC3, PORTC2);
        }

        if (delayTime > 0) {
          i2cReadDelayTime = delayTime;
        }
        break;
      }
    case TONE_CONFIG:
      {
        if (argc > 4) {
          // these vars are here for clarity, they'll optimized away by the compiler
          byte pin = argv[0]; // En pines 3 y 11 si se está ejecutando un PWM, la función tone interferirá
          int freq = argv[1] + (argv[2] << 7);
          int duration = argv[3] + (argv[4] << 7);

          if (IS_PIN_DIGITAL(pin)) {
            attachTone(pin, freq, duration);
          }
        }
        break;
      }
    case NOTONE_CONFIG:
      {
        byte pin = argv[0];
        noTone(pin);
        ReportEndTone = false;
        break;
      }
    case SERVO_CONFIG:
      {
        if (argc > 4) {
          byte pin = argv[0];
          int minPulse = argv[1] + (argv[2] << 7);
          int maxPulse = argv[3] + (argv[4] << 7);
          MinimumP = minPulse;
          MaximumP = maxPulse;
          if (IS_PIN_DIGITAL(pin)) {
              if (servoPinMap[pin] < MAX_SERVOS && servos[servoPinMap[pin]].attached()) {
                detachServo(pin);
              }
              attachServo(pin, minPulse, maxPulse);
              setPinModeCallback(pin, PIN_MODE_SERVO);
          }
        }
        break;
      }
    case TIME_CAPTURE:
      {
        byte mod = argv[0];
        capture = argv[1] + (argv[2] << 7) + ((unsigned int)argv[3] << 14); //+info:https://www.arduino.cc/en/Reference/Bitshift

        if (mod == 0) {  // ANALOG
          if (capture == 0) {
            RealTime = true;
          } else {
            RealTime = false;
          }
          DataAnalog = true;
          Start = true;
        }

        if (mod == 1) {  // DIGITAL
          if (capture == 0) {
            RealTime = true;
          } else {
            RealTime = false;
          }
          DataDigital = true;
          Start = true;
        }

        if (mod == 2) { // STOP
          Start = false;
          DataAnalog = false;
          DataDigital = false;
          RealTime = false;
        }
        break;
      }
    case STRING_DATA:
      {
        Firmata.sendSysex(command, argc, argv);
      }
    case I2C_SCAN:
      {
        byte error, address;
        int nDevices;

        nDevices = 0;
        for (address = 1; address < 127; address++ ) {
          // The i2c_scanner uses the return value of
          // the Write.endTransmisstion to see if
          // a device did acknowledge to the address.
          Wire.begin();                     // Initiate the Wire library and join the I2C bus as a master or slave
          Wire.beginTransmission(address);
          error = Wire.endTransmission();

          if (error == 0) {
            lcd.begin (16, 2);                    //Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display (16 characters, 2 lines)
            lcd.setBacklightPin(3, POSITIVE);     // Sets the back light pin for the LCD shield.
            lcd.setBacklight(HIGH);               // Set the backlight off/on, or set Backlight brightness (0-255)

            if (address < 16) {
              Serial.print("0");
            }

            Firmata.sendSysex(command, 1, &address);
            nDevices++;
          }
          Wire.end();
        }
      }
  }
}

/* reference: BlinkM_funcs.h by Tod E. Kurt, ThingM, http://thingm.com/ */
// Enables Pins A2 and A3 to be used as GND and Power
// so that I2C devices can be plugged directly
// into Arduino header (pins A2 - A5)
static void enablePowerPins(byte pwrpin, byte gndpin)
{
  if (powerPinsEnabled == 0) {
    DDRC |= _BV(pwrpin) | _BV(gndpin);
    PORTC &= ~ _BV(gndpin);
    PORTC |=  _BV(pwrpin);
    powerPinsEnabled = 1;
    delay(100);
  }
}


void setup()
{        
  // Callback Functions
  Firmata.attach(SET_PIN_MODE, setPinModeCallback);
  Firmata.attach(SET_DIGITAL_PIN_VALUE, setDigitalPinValueCallback);
  Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
  Firmata.attach(REPORT_ANALOG, ReportAnalogCallback);
  Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
  Firmata.attach(ANALOG_REFERENCE, reportAnalogReferenceCallback);
  Firmata.attach(INTERRUPT_CONFIG, reportInterruptConfigCallback);
  Firmata.attach(STRING_DATA, stringCallback);
  Firmata.attach(START_SYSEX, sysexCallback);

  Firmata.begin(57600);               // Start the library
}

void loop()
{
  while (Firmata.available()) {       // If there is anything in serial port
    Firmata.processInput();           // Process it
  }

  if (Start) {      // Count time to stop report data or tone functions

    if (Control && !RealTime) {
      currentMillis = millis();
      capture = capture + currentMillis;
      Control = false;
    }

    if (DataAnalog) {
      currentMillis = millis();
      if (currentMillis - previousMillis > 100) {
        previousMillis = currentMillis;            // run this every 100ms
        ReportAnalogCallback(0, 0);
      }
    }

    if (DataDigital) {
      currentMillis = millis();
      if (currentMillis - previousMillis > 100) {
        previousMillis = currentMillis;            // run this every 100ms
        for (int portDigital = 0; portDigital < 3; portDigital++) {
          outputPort(portDigital, readPort(portDigital, readPort(portDigital, 0xff)));
        }
      }
    }

    currentMillis = millis();
    if (currentMillis > capture && !RealTime) {
      Start = false;
      Control = true;
      Firmata.write(0xF1);
    }
  }

  if (readingContinuously) {
    currentMillisI2C = millis();
    if (currentMillisI2C - previousMillisI2C > samplingInterval) {
      previousMillisI2C += samplingInterval;

      for (byte i = 0; i < queryIndex; i++) {
        readAndReportData(query[i].addr, query[i].reg, query[i].bytes);
      }
    }
  }
  if (ReportEndTone) {
    t_current = millis();
    if (t_current > t_updated) {
      ReportEndTone = false;
      Firmata.write(0xF8);
    }
  }
}
