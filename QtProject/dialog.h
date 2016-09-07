#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>

#include "qcustomplot.h"

#include <QComboBox>
#include <QSignalMapper>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:

    // Start
    void on_ConnectDisconnect_toggled(bool checked);

    void on_RefreshPorts_clicked();

    void on_ShowText(const QString text);


    // Analog part
    void SlotComboBoxAnalog(int id);

    void SlotComboBoxDigitalMode(int id);


    // Read/Write/Show data
    void on_ButtonReadAnalog_toggled(bool checked);

    void SlotModeReadAnalog(int id);

    void on_ButtonReadDigital_toggled(bool checked);

    void SlotModeReadDigital(int id);

    void on_ButtonWriteDigital_clicked();

    void SlotModeWriteDigital(int id);

    void UpdateData(const bool listo);


    // Pin AREF
    void on_ArefDefault_clicked();

    void on_ArefExternal_clicked();

    void on_ArefInternal_clicked();


    // Digital Part
    void SlotDigitalMode(int id);

    void SlotDigitalValue(int id);

    void SlotSliderPWM(int id);


    // Plot data
    void on_listAnalogPin_currentIndexChanged(int index);

    void on_listDigitalPin_currentIndexChanged(int index);

    void on_radioButton_CaptureData_clicked();

    void on_radioButton_RealTime_clicked();

    void on_CaptureDuration_valueChanged(int arg1);

    void on_StartCapture_clicked();

    void on_StopCapture_clicked();

    void EndReceiveData(const bool stop);

    void on_SaveGraph_clicked();


    // Tone / NoTone
    void SlotSetTone(int id);

    void SlotSetNoTone(int id);

    void SlotSetToneA(int id);

    void SlotSetNoToneA(int id);

    void EndTimeTone(const bool stop);


    // Servo

    void SlotActServo(int id);

    void SlotMinPulse(int id);

    void SlotMaxPulse(int id);

    void SlotSliderServo(int id);


    // Interrupts
    void ShowInterrupt(const int number);

    void on_listModesInt0_currentIndexChanged(int index);

    void on_listModesInt1_currentIndexChanged(int index);

    void on_DisableInt0_clicked();

    void on_DisableInt1_clicked();


    // I2C
    void ShowDataI2C(const int length);

    void ShowI2CDetected(const bool detected);

    void on_ShowInLCD_clicked();

    void on_I2C_Scan_clicked();

    void on_I2C_Config_clicked();

    void on_Write_I2C_clicked();

    void on_Read_I2C_clicked();

    void on_Stop_I2C_clicked();

    void on_listI2Cdevices_currentIndexChanged(int index);

    void on_BytesToRead_valueChanged(int arg1);

private:
    Ui::Dialog *ui;
    QTimer dataTimer;

    QVector<QComboBox *> ComboBoxDigitalMode;
    QSignalMapper* signalMapDigMode;

    QVector<QLabel *> DigitalData;

    // State of Digital Outputs: LOW, HIGH
    QString StateDigital[2];

    QVector<QComboBox *> ComboBoxDigitalValue;
    QSignalMapper* signalMapDigValue;

    QVector<QSlider *> SliderPWM;
    QSignalMapper* signalMapPWM;

    // Array for place slider on ui
    int PosSlider[6];

    // Array for provide the delivery
    int PWMPin[6];
    int PWMPinReverse[12];

    QVector<QLabel *> InfoPWM;

    QVector<QLCDNumber *> AnalogData;

    QVector<QLabel *> DigitalDataAnalogPins;

    QVector<QComboBox *> DigitalMode;
    QSignalMapper* signalMapDigitalMode;

    // Array for provide the delivery
    int PinAnalogMode[6];

    QVector<QComboBox *> AnalogDigMode;
    QSignalMapper* signalMapAnalogDigMode;

    QVector<QRadioButton *> ModeReadAnalog;
    QSignalMapper* signalMapModeReadAnalog;

    QVector<QRadioButton *> ModeReadDigital;
    QSignalMapper* signalMapModeReadDigital;

    QVector<QRadioButton *> ModeWriteDigital;
    QSignalMapper* signalMapModeWriteDigital;


    QVector<QSpinBox *> Frequency;
    QVector<QSpinBox *> DurationTone;

    QVector<QSpinBox *> FrequencyA;
    QVector<QSpinBox *> DurationToneA;

    QVector<QPushButton *> SetTone;
    QSignalMapper* signalMapSetTone;

    QVector<QPushButton *> SetNoTone;
    QSignalMapper* signalMapSetNoTone;

    QVector<QPushButton *> SetToneA;
    QSignalMapper* signalMapSetToneA;

    QVector<QPushButton *> SetNoToneA;
    QSignalMapper* signalMapSetNoToneA;

    int ActiveTone = 0;

    QVector<QCheckBox *> ActServo;
    QSignalMapper* signalMapActServo;

    QVector<QSpinBox *> MinPulse;
    QSignalMapper* signalMapMinPulse;

    QVector<QSpinBox *> MaxPulse;
    QSignalMapper* signalMapMaxPulse;

    QVector<QSlider *> SliderServo;
    QSignalMapper* signalMapSliderServo;

    QVector<QLabel *> degreeServo;

    int PinServo[6];
    int PinServoReverse[10];

    // Count for Interrupts
    int ContInt0 = 0;
    int ContInt1 = 0;

    bool CaptureOK = false;
    bool ReadOK = false;
    bool ReadRTOK = false;
    bool ReadRTDigital = false;
    bool DiscreteOK = false;

};

#endif // DIALOG_H
