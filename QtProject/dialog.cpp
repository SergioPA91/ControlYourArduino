#include "dialog.h"
#include "ui_dialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <Qstring>
#include <QObject.h>
#include <QComboBox>
#include <QVector>
#include <QSignalMapper>

#include <QtArduino.cpp>

QtArduino arduino;

Dialog::Dialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // DISPLAY LIST AVAILABLE PORTS
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->listCOMs->addItem(info.portName());
    }

    // COMBOBOXES DIGITAL MODE FOR ANALOG PINS
    // Initilize array
    PinAnalogMode[0] = 14; PinAnalogMode[1] = 15; PinAnalogMode[2] = 16; PinAnalogMode[3] = 17; PinAnalogMode[4] = 18; PinAnalogMode[5] = 19;

    signalMapDigitalMode = new QSignalMapper(this);
    bool checkDM;
    int row = 0;
    for (int i = 0 ; i < 6; i++)
    {
        DigitalMode << new QComboBox(this);
        DigitalMode.at(i)->addItem("Low");
        DigitalMode.at(i)->addItem("High");
        ui->gridLayoutPanelAnalog->addWidget(DigitalMode.at(i),row,1);
        DigitalMode.at(i)->setCurrentIndex(0);
        DigitalMode.at(i)->show();
        checkDM = connect (DigitalMode[i],SIGNAL(currentIndexChanged(int)),signalMapDigitalMode,SLOT(map()));
        signalMapDigitalMode->setMapping(DigitalMode[i],i);
        row = row + 2;
    }
    connect(signalMapDigitalMode,SIGNAL(mapped(int)),this,SLOT(SlotComboBoxDigitalMode(int)));


    // COMBOBOXES ANALOG PINS
    signalMapAnalogDigMode = new QSignalMapper(this);
    bool checkADM;
    for (int i = 0 ; i < 12; i++)
    {
        if(i%2 == 0){
            AnalogDigMode << new QComboBox(this);
            AnalogDigMode.at(i)->addItem("Analog");
            AnalogDigMode.at(i)->addItem("Digital");
        }else{
            AnalogDigMode << new QComboBox(this);
            AnalogDigMode.at(i)->addItem("Input-NoPullUp");
            AnalogDigMode.at(i)->addItem("Output");
            AnalogDigMode.at(i)->addItem("Input-PullUp");
        }
        AnalogDigMode.at(i)->setCurrentIndex(0);
        ui->gridLayoutPanelAnalog->addWidget(AnalogDigMode.at(i),i,2);
        AnalogDigMode.at(i)->show();
        checkADM = connect (AnalogDigMode[i],SIGNAL(currentIndexChanged(int)),signalMapAnalogDigMode,SLOT(map()));
        signalMapAnalogDigMode->setMapping(AnalogDigMode[i],i);
    }
    connect(signalMapAnalogDigMode,SIGNAL(mapped(int)),this,SLOT(SlotComboBoxAnalog(int)));



    // LCDS AND LABELS SHOW DATA ANALOG/DIGITAL
    row = 0;
    for (int i = 0 ; i < 6; i++)
    {
        AnalogData << new QLCDNumber(this);
        AnalogData.at(i)->setPalette(Qt::black);
        ui->gridLayoutPanelAnalog->addWidget(AnalogData.at(i),row,3);
        AnalogData.at(i)->show();
        row = row + 2;
    }
    row = 1;
    for (int i = 0 ; i < 6; i++)
    {
        DigitalDataAnalogPins << new QLabel("LOW",this);
        DigitalDataAnalogPins.at(i)->setStyleSheet("border: 2px solid black"); //Border
        DigitalDataAnalogPins.at(i)->setAlignment(Qt::AlignCenter); // Align to center
        ui->gridLayoutPanelAnalog->addWidget(DigitalDataAnalogPins.at(i),row,3);
        DigitalDataAnalogPins.at(i)->show();
        row = row + 2;
    }

    // Adjust each column of grid
    ui->gridLayoutPanelAnalog->setColumnMinimumWidth(1,40);
    ui->gridLayoutPanelAnalog->setColumnMinimumWidth(2,40);
    ui->gridLayoutPanelAnalog->setColumnMinimumWidth(3,40);
    ui->gridLayoutPanelAnalog->setColumnStretch(1,1);
    ui->gridLayoutPanelAnalog->setColumnStretch(2,1);
    ui->gridLayoutPanelAnalog->setColumnStretch(3,1);


    // Radio button for Analog Read
    signalMapModeReadAnalog = new QSignalMapper(this);
    bool checkMRead;
    ModeReadAnalog << new QRadioButton(this);
    ModeReadAnalog.at(0)->setText("Discrete");
    ModeReadAnalog.at(0)->setChecked(true);
    ui->verticalLayoutReadAnalog->addWidget(ModeReadAnalog.at(0));
    checkMRead = connect (ModeReadAnalog[0],SIGNAL(clicked()),signalMapModeReadAnalog,SLOT(map()));
    signalMapModeReadAnalog->setMapping(ModeReadAnalog[0],0);

    ModeReadAnalog << new QRadioButton(this);
    ModeReadAnalog.at(1)->setText("Continuous");
    ModeReadAnalog.at(1)->setChecked(false);
    ui->verticalLayoutReadAnalog->addWidget(ModeReadAnalog.at(1));
    checkMRead = connect (ModeReadAnalog[1],SIGNAL(clicked()),signalMapModeReadAnalog,SLOT(map()));
    signalMapModeReadAnalog->setMapping(ModeReadAnalog[1],1);

    connect(signalMapModeReadAnalog,SIGNAL(mapped(int)),this,SLOT(SlotModeReadAnalog(int)));


    // Radio button for Digital Read
    signalMapModeReadDigital = new QSignalMapper(this);
    bool checkMReadD;
    ModeReadDigital << new QRadioButton(this);
    ModeReadDigital.at(0)->setText("Discrete");
    ModeReadDigital.at(0)->setChecked(true);
    ui->verticalLayoutReadDigital->addWidget(ModeReadDigital.at(0));
    checkMReadD = connect (ModeReadDigital[0],SIGNAL(clicked()),signalMapModeReadDigital,SLOT(map()));
    signalMapModeReadDigital->setMapping(ModeReadDigital[0],0);

    ModeReadDigital << new QRadioButton(this);
    ModeReadDigital.at(1)->setText("Continuous");
    ModeReadDigital.at(1)->setChecked(false);
    ui->verticalLayoutReadDigital->addWidget(ModeReadDigital.at(1));
    checkMReadD = connect (ModeReadDigital[1],SIGNAL(clicked()),signalMapModeReadDigital,SLOT(map()));
    signalMapModeReadDigital->setMapping(ModeReadDigital[1],1);

    connect(signalMapModeReadDigital,SIGNAL(mapped(int)),this,SLOT(SlotModeReadDigital(int)));


    // Radio button for Digital Write
    signalMapModeWriteDigital = new QSignalMapper(this);
    bool checkMReadW;
    ModeWriteDigital << new QRadioButton(this);
    ModeWriteDigital.at(0)->setText("Discrete");
    ModeWriteDigital.at(0)->setChecked(true);
    ui->verticalLayoutWriteDigital->addWidget(ModeWriteDigital.at(0));
    checkMReadW = connect (ModeWriteDigital[0],SIGNAL(clicked()),signalMapModeWriteDigital,SLOT(map()));
    signalMapModeWriteDigital->setMapping(ModeWriteDigital[0],0);

    ModeWriteDigital << new QRadioButton(this);
    ModeWriteDigital.at(1)->setText("Continuous");
    ModeWriteDigital.at(1)->setChecked(false);
    ui->verticalLayoutWriteDigital->addWidget(ModeWriteDigital.at(1));
    checkMReadW = connect (ModeWriteDigital[1],SIGNAL(clicked()),signalMapModeWriteDigital,SLOT(map()));
    signalMapModeWriteDigital->setMapping(ModeWriteDigital[1],1);

    connect(signalMapModeWriteDigital,SIGNAL(mapped(int)),this,SLOT(SlotModeWriteDigital(int)));

    // LABELS SHOW DATA DIGITAL
    StateDigital[0] = "LOW";
    StateDigital[1] = "HIGH";

    row = 12;

    for (int i = 0 ; i < 12; i++)
    {
        DigitalData << new QLabel("LOW",this);
        DigitalData.at(i)->setStyleSheet("border: 2px solid black");        // Border
        DigitalData.at(i)->setAlignment(Qt::AlignCenter);                   // Align to center
        ui->gridLayoutDigital->addWidget(DigitalData.at(i),row,1);
        DigitalData.at(i)->show();
        row--;
    }


    // COMBOBOX DIGITAL MODE
    signalMapDigMode = new QSignalMapper(this);
    bool checkDigMode;
    row = 12; // Necessary to invert the order in which the grid is filling.

    for (int i = 0 ; i < 12; i++)
    {
        // +info: http://www.qtcentre.org/threads/23295-Array-of-QLabel
        ComboBoxDigitalMode << new QComboBox(this);
        ComboBoxDigitalMode.at(i)->addItem("Input-NoPullUp");
        ComboBoxDigitalMode.at(i)->addItem("Output");
        if(i == 1 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9){ // If it´s a PWM Pin(Pins: 3,5,6,9,10,11)
            ComboBoxDigitalMode.at(i)->addItem("PWM");
            ComboBoxDigitalMode.at(i)->addItem("Servo");
        }
        ComboBoxDigitalMode.at(i)->addItem("Input-PullUp");
        // +info: http://www.qtcentre.org/threads/42514-dynamically-create-QLabels
        ui->gridLayoutDigital->addWidget(ComboBoxDigitalMode.at(i),row,2);
        ComboBoxDigitalMode.at(i)->show();
        checkDigMode = connect(ComboBoxDigitalMode[i],SIGNAL(currentIndexChanged(int)),signalMapDigMode,SLOT(map()));
        signalMapDigMode->setMapping(ComboBoxDigitalMode[i],i);
        row--;
    }
    connect(signalMapDigMode,SIGNAL(mapped(int)),this,SLOT(SlotDigitalMode(int)));


    // COMBOBOX DIGITAL VALUE
    signalMapDigValue = new QSignalMapper(this);
    bool checkDigValue;
    row = 12;

    for (int i = 0 ; i < 12; i++)
    {
        ComboBoxDigitalValue << new QComboBox(this);
        ComboBoxDigitalValue.at(i)->addItem("Low");
        ComboBoxDigitalValue.at(i)->addItem("High");
        ComboBoxDigitalValue.at(i)->setCurrentIndex(-1);
        ui->gridLayoutDigital->addWidget(ComboBoxDigitalValue.at(i),row,3);
        ComboBoxDigitalValue.at(i)->show();
        checkDigValue = connect(ComboBoxDigitalValue[i],SIGNAL(currentIndexChanged(int)),signalMapDigValue,SLOT(map()));
        signalMapDigValue->setMapping(ComboBoxDigitalValue[i],i);
        row--;
    }
    connect(signalMapDigValue,SIGNAL(mapped(int)),this,SLOT(SlotDigitalValue(int)));


    // SLIDER PWM
    signalMapPWM = new QSignalMapper(this);
    bool checkPWM;

    // Initialize array to place Slider
    PosSlider[0] = 11;  PosSlider[1] = 9; PosSlider[2] = 8; PosSlider[3] = 5; PosSlider[4] = 4; PosSlider[5] = 3;

    // Initialize array for provide PWM Pin
    PWMPin[0] = 3; PWMPin[1] = 5; PWMPin[2] = 6; PWMPin[3] = 9; PWMPin[4] = 10; PWMPin[5] = 11;

    // For example, comboboxMode with id=9 is in fact pin 11 of Arduino board (and not the pin 9).
    // Therefore, I assign that Slider and labelPWM should enable.
    PWMPinReverse[0] = 0; PWMPinReverse[1] = 0; PWMPinReverse[2] = 0; PWMPinReverse[3] = 1; PWMPinReverse[4] = 2;
    PWMPinReverse[5] = 0; PWMPinReverse[6] = 0; PWMPinReverse[7] = 3; PWMPinReverse[8] = 4; PWMPinReverse[9] = 5;
    PWMPinReverse[10] = 0; PWMPinReverse[11] = 0;
    for (int i = 0 ; i < 6; i++)
    {
        SliderPWM << new QSlider(Qt::Horizontal,this);
        SliderPWM.at(i)->setMaximum(100);
        SliderPWM.at(i)->setMinimum(0);
        SliderPWM.at(i)->setValue(0);
        ui->gridLayoutDigital->addWidget(SliderPWM.at(i),PosSlider[i],4);
        SliderPWM.at(i)->show();
        checkPWM = connect(SliderPWM[i],SIGNAL(valueChanged(int)),signalMapPWM,SLOT(map()));
        signalMapPWM->setMapping(SliderPWM[i],i);
    }
    connect(signalMapPWM,SIGNAL(mapped(int)),this,SLOT(SlotSliderPWM(int)));


    // LABELS SHOW INFO PWM
    for (int i = 0 ; i < 6; i++)
    {
        InfoPWM << new QLabel("( 0 %)",this);
        InfoPWM.at(i)->setAlignment(Qt::AlignCenter); // Align to center
        ui->gridLayoutDigital->addWidget(InfoPWM.at(i),PosSlider[i],5);
        InfoPWM.at(i)->show();
    }

    // Adjust each column of grid
    ui->gridLayoutDigital->setColumnMinimumWidth(1,60);
    ui->gridLayoutDigital->setColumnMinimumWidth(2,60);
    ui->gridLayoutDigital->setColumnMinimumWidth(3,60);
    ui->gridLayoutDigital->setColumnStretch(3,1);
    ui->gridLayoutDigital->setColumnStretch(4,3);
    ui->gridLayoutDigital->setColumnStretch(5,2);



    // CUSTOM PLOT CAPTURE/REAL TIME
    // include this section to fully disable antialiasing for higher performance:
    ui->customPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    ui->customPlot->xAxis->setTickLabelFont(font);
    ui->customPlot->yAxis->setTickLabelFont(font);
    ui->customPlot->legend->setFont(font);

    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->customPlot->graph(0)->setAntialiasedFill(false);
    ui->customPlot->addGraph(); // red line
    ui->customPlot->graph(1)->setPen(QPen(Qt::red));
    ui->customPlot->graph(0)->setChannelFillGraph(ui->customPlot->graph(1));

    ui->customPlot->addGraph(); // blue dot
    ui->customPlot->graph(2)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->customPlot->addGraph(); // red dot
    ui->customPlot->graph(3)->setPen(QPen(Qt::red));
    ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->customPlot->xAxis->setAutoTickStep(false);
    ui->customPlot->xAxis->setTickStep(6); //Default: 2
    ui->customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    dataTimer.start(0);             // Interval 0 means to refresh as fast as possible


    // SPINBOX FOR FREQUENCY TONE
    row = 12;

    for (int i = 0 ; i < 12; i++)
    {
        Frequency << new QSpinBox(this);
        Frequency.at(i)->setValue(31);
        Frequency.at(i)->setMaximum(65535);
        Frequency.at(i)->setMinimum(31);
        ui->gridLayoutToneD->addWidget(Frequency.at(i),row,1);
        Frequency.at(i)->show();

        DurationTone << new QSpinBox(this);
        DurationTone.at(i)->setValue(0);
        DurationTone.at(i)->setMaximum(16383);
        DurationTone.at(i)->setMinimum(0);
        ui->gridLayoutToneD->addWidget(DurationTone.at(i),row,2);
        DurationTone.at(i)->show();

        if(i < 6){
            FrequencyA << new QSpinBox(this);
            FrequencyA.at(i)->setValue(31);
            FrequencyA.at(i)->setMaximum(65535);
            FrequencyA.at(i)->setMinimum(31);
            ui->gridLayoutToneA->addWidget(FrequencyA.at(i),i,4);
            FrequencyA.at(i)->show();

            DurationToneA << new QSpinBox(this);
            DurationToneA.at(i)->setValue(0);
            DurationToneA.at(i)->setMaximum(16383);
            DurationToneA.at(i)->setMinimum(0);
            ui->gridLayoutToneA->addWidget(DurationToneA.at(i),i,3);
            DurationToneA.at(i)->show();
        }
        row--;
    }

    // BUTTONS SET TONE/NOTONE
    signalMapSetTone = new QSignalMapper(this);
    signalMapSetNoTone = new QSignalMapper(this);

    signalMapSetToneA = new QSignalMapper(this);
    signalMapSetNoToneA = new QSignalMapper(this);

    bool checkT;
    row = 12;

    for (int i = 0 ; i < 12; i++)
    {
        SetTone << new QPushButton(this);
        SetTone.at(i)->setText("Set Tone");
        ui->gridLayoutToneD->addWidget(SetTone.at(i),row,3);
        SetTone.at(i)->show();
        checkT = connect(SetTone[i],SIGNAL(clicked()),signalMapSetTone,SLOT(map()));
        signalMapSetTone->setMapping(SetTone[i],i);

        SetNoTone << new QPushButton(this);
        SetNoTone.at(i)->setText("Set NoTone");
        ui->gridLayoutToneD->addWidget(SetNoTone.at(i),row,4);
        SetNoTone.at(i)->show();
        checkT = connect(SetNoTone[i],SIGNAL(clicked()),signalMapSetNoTone,SLOT(map()));
        signalMapSetNoTone->setMapping(SetNoTone[i],i);

        if(i < 6){
            SetToneA << new QPushButton(this);
            SetToneA.at(i)->setText("Set Tone");
            ui->gridLayoutToneA->addWidget(SetToneA.at(i),i,2);
            SetToneA.at(i)->show();
            checkT = connect(SetToneA[i],SIGNAL(clicked()),signalMapSetToneA,SLOT(map()));
            signalMapSetToneA->setMapping(SetToneA[i],i);

            SetNoToneA << new QPushButton(this);
            SetNoToneA.at(i)->setText("Set NoTone");
            ui->gridLayoutToneA->addWidget(SetNoToneA.at(i),i,1);
            SetNoToneA.at(i)->show();
            checkT = connect(SetNoToneA[i],SIGNAL(clicked()),signalMapSetNoToneA,SLOT(map()));
            signalMapSetNoToneA->setMapping(SetNoToneA[i],i);
        }

        row--;
    }
    connect(signalMapSetTone,SIGNAL(mapped(int)),this,SLOT(SlotSetTone(int)));
    connect(signalMapSetNoTone,SIGNAL(mapped(int)),this,SLOT(SlotSetNoTone(int)));
    connect(signalMapSetToneA,SIGNAL(mapped(int)),this,SLOT(SlotSetToneA(int)));
    connect(signalMapSetNoToneA,SIGNAL(mapped(int)),this,SLOT(SlotSetNoToneA(int)));

    // GRID SERVO CONFIG

    PinServo[0] = 1; PinServo[1] = 3; PinServo[2] = 4; PinServo[3] = 7; PinServo[4] = 8; PinServo[5] = 9;
    PinServoReverse[0] = 0; PinServoReverse[1] = 0; PinServoReverse[2] = 0; PinServoReverse[3] = 1; PinServoReverse[4] = 2;
    PinServoReverse[5] = 0; PinServoReverse[6] = 0; PinServoReverse[7] = 3; PinServoReverse[8] = 4; PinServoReverse[9] = 5;
    signalMapActServo = new QSignalMapper(this);
    signalMapMinPulse = new QSignalMapper(this);
    signalMapMaxPulse = new QSignalMapper(this);
    signalMapSliderServo = new QSignalMapper(this);

    bool checkS;
    row = 5;

    for (int i = 0 ; i < 6; i++)
    {
        ActServo << new QCheckBox(this);
        ActServo.at(i)->setText("Activate Servo");
        ui->gridLayoutServo->addWidget(ActServo.at(i),row,1);
        ActServo.at(i)->show();
        checkS = connect(ActServo[i],SIGNAL(clicked()),signalMapActServo,SLOT(map()));
        signalMapActServo->setMapping(ActServo[i],i);

        MinPulse << new QSpinBox(this);
        MinPulse.at(i)->setValue(544);
        MinPulse.at(i)->setMaximum(2400);
        MinPulse.at(i)->setMinimum(544);
        ui->gridLayoutServo->addWidget(MinPulse.at(i),row,2);
        MinPulse.at(i)->show();
        checkS = connect(MinPulse[i],SIGNAL(valueChanged(int)),signalMapMinPulse,SLOT(map()));
        signalMapMinPulse->setMapping(MinPulse[i],i);

        MaxPulse << new QSpinBox(this);
        MaxPulse.at(i)->setMaximum(2400);
        MaxPulse.at(i)->setMinimum(544);
        MaxPulse.at(i)->setValue(2400);
        ui->gridLayoutServo->addWidget(MaxPulse.at(i),row,3);
        MaxPulse.at(i)->show();
        checkS = connect(MaxPulse[i],SIGNAL(valueChanged(int)),signalMapMaxPulse,SLOT(map()));
        signalMapMaxPulse->setMapping(MaxPulse[i],i);

        SliderServo << new QSlider(Qt::Horizontal,this);
        SliderServo.at(i)->setMaximum(180);
        SliderServo.at(i)->setMinimum(0);
        SliderServo.at(i)->setValue(0);
        ui->gridLayoutServo->addWidget(SliderServo.at(i),row,4);
        SliderServo.at(i)->show();
        checkPWM = connect(SliderServo[i],SIGNAL(valueChanged(int)),signalMapSliderServo,SLOT(map()));
        signalMapSliderServo->setMapping(SliderServo[i],i);

        degreeServo << new QLabel("( 0 º)",this);
        degreeServo.at(i)->setAlignment(Qt::AlignCenter);           // Align to center
        ui->gridLayoutServo->addWidget(degreeServo.at(i),row,5);
        degreeServo.at(i)->show();

        row--;
    }
    connect(signalMapActServo,SIGNAL(mapped(int)),this,SLOT(SlotActServo(int)));
    connect(signalMapMinPulse,SIGNAL(mapped(int)),this,SLOT(SlotMinPulse(int)));
    connect(signalMapMaxPulse,SIGNAL(mapped(int)),this,SLOT(SlotMaxPulse(int)));
    connect(signalMapSliderServo,SIGNAL(mapped(int)),this,SLOT(SlotSliderServo(int)));

    // Adjust each column of grid
    ui->gridLayoutServo->setColumnMinimumWidth(1,60);
    ui->gridLayoutServo->setColumnMinimumWidth(2,60);
    ui->gridLayoutServo->setColumnMinimumWidth(3,60);
    ui->gridLayoutServo->setColumnMinimumWidth(4,60);
    ui->gridLayoutServo->setColumnMinimumWidth(5,60);
    ui->gridLayoutServo->setColumnStretch(1,1);
    ui->gridLayoutServo->setColumnStretch(2,1);
    ui->gridLayoutServo->setColumnStretch(3,1);
    ui->gridLayoutServo->setColumnStretch(4,3);
    ui->gridLayoutServo->setColumnStretch(5,2);

    // DISABLE BUTTONS
    ui->ButtonReadAnalog->setDisabled(true);
    ui->ButtonReadDigital->setDisabled(true);
    ui->ButtonWriteDigital->setDisabled(true);

    ui->ArefDefault->setDisabled(true);
    ui->ArefExternal->setDisabled(true);
    ui->ArefInternal->setDisabled(true);

    ui->listAnalogPin->setDisabled(true);
    ui->listDigitalPin->setDisabled(true);
    ui->CaptureDuration->setDisabled(true);
    ui->StartCapture->setDisabled(true);
    ui->StopCapture->setDisabled(true);
    ui->SaveGraph->setDisabled(true);
    ui->radioButton_CaptureData->setDisabled(true);
    ui->radioButton_RealTime->setDisabled(true);

    //Disable buttons tab tone/noTone
    for(int i = 0; i < 12; i++){
        SetTone.at(i)->setDisabled(true);
        SetNoTone.at(i)->setDisabled(true);
        Frequency.at(i)->setDisabled(true);
        DurationTone.at(i)->setDisabled(true);
        if(i < 6){
            SetToneA.at(i)->setDisabled(true);
            SetNoToneA.at(i)->setDisabled(true);
            FrequencyA.at(i)->setDisabled(true);
            DurationToneA.at(i)->setDisabled(true);
        }
    }

    // DISABLE GRIDS AND TABS
    ui->tab_4->setDisabled(true);
    ui->tab_8->setDisabled(true);

    ui->labelDigitalOptions->hide();
    ui->label_DigitalValue->hide();
    ui->label_PWM->hide();
    for(int i=0;i<12;i++){
        if(i<2){
            ModeReadAnalog.at(i)->setDisabled(true);
            ModeReadDigital.at(i)->setDisabled(true);
            ModeWriteDigital.at(i)->setDisabled(true);
        }
        ComboBoxDigitalMode.at(i)->setDisabled(true);
        ComboBoxDigitalValue.at(i)->setHidden(true);
        if(i<6){
            SliderPWM.at(i)->setHidden(true);
            InfoPWM.at(i)->setHidden(true);
            DigitalMode.at(i)->setHidden(true);
            DigitalDataAnalogPins.at(i)->setHidden(true);

            //Disable servo
            ActServo.at(i)->setDisabled(true);
            MinPulse.at(i)->setDisabled(true);
            MaxPulse.at(i)->setDisabled(true);
            SliderServo.at(i)->setDisabled(true);
        }
            AnalogDigMode.at(i)->setDisabled(true);
    }


    // SIGNALS
    connect(&arduino,SIGNAL(SerialData(const QString)),this,SLOT(on_ShowText(const QString)));
    connect(&arduino,SIGNAL(UpdateData(const bool)),this,SLOT(UpdateData(const bool)));
    connect(&arduino,SIGNAL(EndReceiveData(const bool)),this,SLOT(EndReceiveData(const bool)));
    connect(&arduino,SIGNAL(EndTimeTone(const bool)),this,SLOT(EndTimeTone(const bool)));
    connect(&arduino,SIGNAL(IntCaptured(const int)),this,SLOT(ShowInterrupt(const int)));
    connect(&arduino,SIGNAL(ReadI2C(const int)),this,SLOT(ShowDataI2C(const int)));
    connect(&arduino,SIGNAL(I2CDeviceDetect(const bool)),this,SLOT(ShowI2CDetected(const bool)));

    // TOOLTIPS
    ui->ArefDefault->setToolTip("The default analog reference of 5 volts (on 5V Arduino boards) or 3.3 volts (on 3.3V Arduino boards).");
    ui->ArefExternal->setToolTip("The voltage applied to the AREF pin (0 to 5V only) is used as the reference.");
    ui->ArefInternal->setToolTip("An built-in reference, equal to 1.1 volts.");

    ModeReadAnalog.at(0)->setToolTip("A reading is taken (one-time only).");
    ModeReadAnalog.at(1)->setToolTip("Reading is taken continuously.");

    ModeReadDigital.at(0)->setToolTip("A reading is taken (one-time only).");
    ModeReadDigital.at(1)->setToolTip("Reading is taken continuously.");

    ModeWriteDigital.at(0)->setToolTip("A reading is taken (one-time only).");
    ModeWriteDigital.at(1)->setToolTip("Reading is taken continuously.");
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_ConnectDisconnect_toggled(bool checked)         // Connect/Disconnect Arduino Board
{
    if(checked == true){  // Connect
        if(arduino.begin(ui->listCOMs->currentText())){

            // WELCOME MESSAGE
            ui->Info->append(QTime::currentTime().toString("hh:mm:ss ->"));
            ui->Info->append(QString("¡Arduino connected to %1!").arg(ui->listCOMs->currentText()));

            // ENABLE/DISABLE BUTTONS
            ui->ButtonReadAnalog->setDisabled(false);
            ui->ButtonReadDigital->setDisabled(false);
            if(ModeWriteDigital.at(1)->isChecked()){
                ui->ButtonWriteDigital->setEnabled(true);
            }
            ui->RefreshPorts->setDisabled(true);
            ui->listCOMs->setDisabled(true);

            ui->ArefDefault->setEnabled(true);
            ui->ArefExternal->setEnabled(true);
            ui->ArefInternal->setEnabled(true);

            ui->listAnalogPin->setEnabled(true);
            ui->listDigitalPin->setEnabled(true);

            if(ui->listAnalogPin->currentIndex() != -1 || ui->listDigitalPin->currentIndex() != -1){
                ui->radioButton_CaptureData->setEnabled(true);
                ui->radioButton_RealTime->setEnabled(true);
                if(ui->CaptureDuration->value() == 0){
                    ui->StartCapture->setDisabled(true);
                    ui->SaveGraph->setDisabled(true);
                }
                if(ui->radioButton_RealTime->isChecked()){
                    ui->StartCapture->setEnabled(true);
                    ui->SaveGraph->setEnabled(true);
                }
            }
            if(ui->radioButton_CaptureData->isChecked()){
                ui->CaptureDuration->setEnabled(true);
            }

            // To disable button Read analog when all analog pins are digital
            bool AnyAnalog = false;
            for(int i = 0; i < 12; i = i + 2){
                if(AnalogDigMode.at(i)->currentIndex() == 0){
                    AnyAnalog = true;
                }
            }
            if(AnyAnalog == false){
                ui->ButtonReadAnalog->setDisabled(true);
                ModeReadAnalog.at(0)->setDisabled(true);
                ModeReadAnalog.at(1)->setDisabled(true);
            }else{
                ui->ButtonReadAnalog->setDisabled(false);
                ModeReadAnalog.at(0)->setDisabled(false);
                ModeReadAnalog.at(1)->setDisabled(false);
            }

            // ENABLE/DISABLE GRIDS AND TABS
            ui->tab_4->setDisabled(false);
            ui->tab_8->setDisabled(false);
            for(int i=0;i<12;i++){
                if(i<2){
                    ModeReadDigital.at(i)->setEnabled(true);
                    ModeWriteDigital.at(i)->setEnabled(true);
                }
                ComboBoxDigitalMode.at(i)->setDisabled(false);
                if(i%2 == 0){
                    AnalogDigMode.at(i)->setEnabled(true);
                    if(AnalogDigMode.at(i)->currentIndex() == 1){   // If Digital
                        AnalogDigMode.at(i+1)->setEnabled(true);
                    }
                    if(AnalogDigMode.at(i+1)->currentIndex() == 1){ // If Output
                        DigitalMode.at(i/2)->setEnabled(true);
                        ui->labelDigitalOptions->show();
                    }
                }
                if(ComboBoxDigitalMode.at(i)->currentIndex() == 1)  // If Output mode is selected
                    ComboBoxDigitalValue.at(i)->setDisabled(false);
                if(ComboBoxDigitalMode.at(i)->currentText() == "PWM"){
                    SliderPWM.at(PWMPinReverse[i])->setDisabled(false);
                    InfoPWM.at(PWMPinReverse[i])->setDisabled(false);
                }
            }

            // Enable/disable buttons tab tone/noTone
            if(ActiveTone == 0){
                for(int i = 0; i < 12; i++){
                    SetTone.at(i)->setDisabled(false);
                    SetNoTone.at(i)->setDisabled(false);
                    Frequency.at(i)->setDisabled(false);
                    DurationTone.at(i)->setDisabled(false);
                    if(i < 6){
                        SetToneA.at(i)->setDisabled(false);
                        SetNoToneA.at(i)->setDisabled(false);
                        FrequencyA.at(i)->setDisabled(false);
                        DurationToneA.at(i)->setDisabled(false);
                    }
                }
            }else if(ActiveTone < 14){
                SetTone.at(ActiveTone - 2)->setEnabled(true);
                SetNoTone.at(ActiveTone - 2)->setEnabled(true);
                Frequency.at(ActiveTone - 2)->setEnabled(true);
                DurationTone.at(ActiveTone - 2)->setEnabled(true);
            }else if(ActiveTone >= 14){
                SetToneA.at(ActiveTone - 14)->setEnabled(true);
                SetNoToneA.at(ActiveTone - 14)->setEnabled(true);
                FrequencyA.at(ActiveTone - 14)->setEnabled(true);
                DurationToneA.at(ActiveTone - 14)->setEnabled(true);
            }

            if(ComboBoxDigitalMode.at(1)->currentText() == "PWM")
            {
                SetTone.at(1)->setDisabled(true);
                SetNoTone.at(1)->setDisabled(true);
                Frequency.at(1)->setDisabled(true);
                DurationTone.at(1)->setDisabled(true);
            }

            if(ComboBoxDigitalMode.at(9)->currentText() == "PWM")
            {
                SetTone.at(9)->setDisabled(true);
                SetNoTone.at(9)->setDisabled(true);
                Frequency.at(9)->setDisabled(true);
                DurationTone.at(9)->setDisabled(true);
            }

            // Enable/disable servo tab
            for(int i = 0; i < 12; i++){
                if(i < 6){
                    ActServo.at(i)->setEnabled(true);
                }
                if(ComboBoxDigitalMode.at(i)->currentText() == "Servo"){
                    ActServo.at(PinServoReverse[i])->setChecked(true);
                    SliderServo.at(PinServoReverse[i])->setEnabled(true);
                }else{
                    MinPulse.at(PinServoReverse[i])->setEnabled(true);
                    MaxPulse.at(PinServoReverse[i])->setEnabled(true);
                }
            }
            if(ComboBoxDigitalMode.at(7)->currentText() == "PWM" || ComboBoxDigitalMode.at(8)->currentText() == "PWM"){
                ui->tab_3->setDisabled(true);
            }

            ui->tab_8->setEnabled(true);
            ui->Stop_I2C->setDisabled(true);
            if(!ui->ChangeDelayTime->isChecked()){
                ui->I2CDelayTime->setDisabled(true);
            }

            if(ui->BytesToRead->value() == 0){
                ui->Read_I2C->setDisabled(true);
            }
            bool detectedLCD = false;
            for(int i = 0; i < ui->listI2Cdevices->count();i++){
                if(ui->listI2Cdevices->itemText(i) == "27"){
                    detectedLCD = true;
                }
            }
            if(detectedLCD){
                ui->ShowInLCD->setEnabled(true);
            }else{
                ui->ShowInLCD->setDisabled(true);
            }
            if(ui->listI2Cdevices->currentIndex() == -1){
                ui->ChangeDelayTime->setDisabled(true);
                ui->I2CDelayTime->setDisabled(true);
                ui->Write_I2C->setDisabled(true);
                ui->BytesToRead->setDisabled(true);
                ui->radioButton_Cont->setDisabled(true);
                ui->radioButton_Once->setDisabled(true);
                ui->Read_I2C->setDisabled(true);
                ui->Stop_I2C->setDisabled(true);
                ui->I2C_Scan->setDisabled(false);
                ui->ShowInLCD->setDisabled(true);
                ui->listI2Cdevices->setDisabled(false);
            }

        }else{
            ui->ConnectDisconnect->setChecked(false);
            QMessageBox::information(this,"Unable to connect Arduino UNO",
                                     "Please, select other port or unplug and plug in your board and try again.");
        }
    }else{  // Disconnect
        arduino.end();

        // FAREWELL MESSAGE
        if(arduino.begin(ui->listCOMs->currentText())){
                ui->Info->append(QTime::currentTime().toString("hh:mm:ss ->"));
                ui->Info->append("¡Arduino disconnected!");
        }

        // ENABLE/DISABLE BUTTONS
        ui->ButtonReadAnalog->setDisabled(true);
        ui->ButtonReadDigital->setDisabled(true);
        ui->ButtonWriteDigital->setDisabled(true);
        ui->RefreshPorts->setDisabled(false);
        ui->listCOMs->setDisabled(false);

        ui->ArefDefault->setDisabled(true);
        ui->ArefExternal->setDisabled(true);
        ui->ArefInternal->setDisabled(true);

        ui->listAnalogPin->setDisabled(true);
        ui->listDigitalPin->setDisabled(true);
        ui->CaptureDuration->setDisabled(true);
        ui->StartCapture->setDisabled(true);
        ui->SaveGraph->setDisabled(true);
        ui->StopCapture->setDisabled(true);
        ui->radioButton_CaptureData->setDisabled(true);
        ui->radioButton_RealTime->setDisabled(true);

        //Disable buttons tab tone/noTone
        for(int i = 0; i < 12; i++){
            SetTone.at(i)->setDisabled(true);
            SetNoTone.at(i)->setDisabled(true);
            Frequency.at(i)->setDisabled(true);
            DurationTone.at(i)->setDisabled(true);
            if(i < 6){
                SetToneA.at(i)->setDisabled(true);
                SetNoToneA.at(i)->setDisabled(true);
                FrequencyA.at(i)->setDisabled(true);
                DurationToneA.at(i)->setDisabled(true);
            }
        }

        // ENABLE/DISABLE GRIDS AND TABS
        ui->tab_4->setDisabled(true);
        ui->tab_8->setDisabled(true);
        for(int i=0;i<12;i++){
            if(i<2){
                ModeReadAnalog.at(i)->setDisabled(true);
                ModeReadDigital.at(i)->setDisabled(true);
                ModeWriteDigital.at(i)->setDisabled(true);
            }
            ComboBoxDigitalMode.at(i)->setDisabled(true);
            ComboBoxDigitalValue.at(i)->setDisabled(true);
            AnalogDigMode.at(i)->setDisabled(true);

            if(i<6){
                SliderPWM.at(i)->setDisabled(true);
                InfoPWM.at(i)->setDisabled(true);
                DigitalMode.at(i)->setDisabled(true);

                //Disable servo
                ActServo.at(i)->setDisabled(true);
                MinPulse.at(i)->setDisabled(true);
                MaxPulse.at(i)->setDisabled(true);
                SliderServo.at(i)->setDisabled(true);
            }
        }
    }
}

// Update list of available ports
void Dialog::on_RefreshPorts_clicked()
{
    ui->listCOMs->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->listCOMs->addItem(info.portName());
    }
}

// Update sent/received menssages
void Dialog::on_ShowText(const QString text)
{
     ui->Info->append(QTime::currentTime().toString("hh:mm:ss ->"));
     ui->Info->append(text);
}

// SLOT CHANGE MODE ANALOG/DIGITAL (INPUT/OUTPUT)
void Dialog::SlotComboBoxAnalog(int id)
{
    bool OkOutput = false;
    bool AnyAnalog = false;

    // To disable button Read analog when all analog pins are digital
    for(int i = 0; i < 12; i = i + 2){
        if(AnalogDigMode.at(i)->currentIndex() == 0){
            AnyAnalog = true;
        }
    }
    if(AnyAnalog == false){
        ui->ButtonReadAnalog->setDisabled(true);
        ModeReadAnalog.at(0)->setDisabled(true);
        ModeReadAnalog.at(1)->setDisabled(true);
    }else{
        ui->ButtonReadAnalog->setDisabled(false);
        ModeReadAnalog.at(0)->setDisabled(false);
        ModeReadAnalog.at(1)->setDisabled(false);
    }

    if (QComboBox *c = qobject_cast<QComboBox *>(signalMapAnalogDigMode->mapping(id)))
    {
        if(id%2 == 0){                              // If pair, it´s combobox analog/digital mode
            if(c->currentIndex() == 0){ // Analog
                ui->listDigitalPin->setCurrentIndex(-1);

                arduino.pinMode(PinAnalogMode[id/2],2);
                AnalogDigMode.at(id+1)->setDisabled(true);
                DigitalMode.at(id/2)->setHidden(true);
                if(OkOutput == false){
                    ui->labelDigitalOptions->hide();
                }
                DigitalDataAnalogPins.at(id/2)->setHidden(true);
                AnalogData.at(id/2)->show();

                // Analog/Digital Labels
                if(id/2 == 0){
                    ui->label_A0->setText("A0");
                    ui->label_A0->setGeometry(255,140,31,21);
                    ui->label_A0->setStyleSheet("QLabel { color : white; }");
                }
                if(id/2 == 1){
                    ui->label_A1->setText("A1");
                    ui->label_A1->setGeometry(255,200,31,21);
                    ui->label_A1->setStyleSheet("QLabel { color : white; }");
                }
                if(id/2 == 2){
                    ui->label_A2->setText("A2");
                    ui->label_A2->setGeometry(255,260,31,21);
                    ui->label_A2->setStyleSheet("QLabel { color : white; }");
                }
                if(id/2 == 3){
                    ui->label_A3->setText("A3");
                    ui->label_A3->setGeometry(255,320,31,21);
                    ui->label_A3->setStyleSheet("QLabel { color : white; }");
                }
                if(id/2 == 4){
                    ui->label_A4->setText("A4");
                    ui->label_A4->setGeometry(255,380,31,21);
                    ui->label_A4->setStyleSheet("QLabel { color : white; }");
                }
                if(id/2 == 5){
                    ui->label_A5->setText("A5");
                    ui->label_A5->setGeometry(255,440,31,21);
                    ui->label_A5->setStyleSheet("QLabel { color : white; }");
                }
            }else{  // Digital
                ui->listAnalogPin->setCurrentIndex(-1);

                arduino.pinMode(PinAnalogMode[id/2],0);
                AnalogDigMode.at(id+1)->setEnabled(true);
                AnalogDigMode.at(id+1)->setCurrentIndex(0);
                DigitalDataAnalogPins.at(id/2)->show();
                AnalogData.at(id/2)->setHidden(true);

                // Analog/Digital Labels
                if(id/2 == 0){
                    ui->label_A0->setText("D14");
                    ui->label_A0->setGeometry(255,170,31,21);
                    ui->label_A0->setStyleSheet("QLabel { color : black; }");
                }
                if(id/2 == 1){
                    ui->label_A1->setText("D15");
                    ui->label_A1->setGeometry(255,230,31,21);
                    ui->label_A1->setStyleSheet("QLabel { color : black; }");
                }
                if(id/2 == 2){
                    ui->label_A2->setText("D16");
                    ui->label_A2->setGeometry(255,290,31,21);
                    ui->label_A2->setStyleSheet("QLabel { color : black; }");
                }
                if(id/2 == 3){
                    ui->label_A3->setText("D17");
                    ui->label_A3->setGeometry(255,350,31,21);
                    ui->label_A3->setStyleSheet("QLabel { color : black; }");
                }
                if(id/2 == 4){
                    ui->label_A4->setText("D18");
                    ui->label_A4->setGeometry(255,410,31,21);
                    ui->label_A4->setStyleSheet("QLabel { color : black; }");
                }
                if(id/2 == 5){
                    ui->label_A5->setText("D19");
                    ui->label_A5->setGeometry(255,470,31,21);
                    ui->label_A5->setStyleSheet("QLabel { color : black; }");
                }
            }
        }
        for(int i = 1; i < 12; i = i + 2){
            if(AnalogDigMode.at(i)->currentIndex() == 1){   // Any combobox with output
                OkOutput = true;
            }
        }

        if(id%2 != 0){          // If unpair, it´s combobox input/output
            if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                if(c->currentIndex() == 0 || c->currentIndex() == 1){ // Mode input/output
                    arduino.pinMode(PinAnalogMode[id/2],c->currentIndex());
                }else if(c->currentIndex() == 2){ // Mode pullup
                    arduino.pinMode(PinAnalogMode[id/2],11);
                }

            }
            if(c->currentIndex() == 1){ // If output
                DigitalMode.at(id/2)->show();
                DigitalMode.at(id/2)->setEnabled(true);
                ui->labelDigitalOptions->show();
                if(!DiscreteOK){
                    DigitalMode.at(id/2)->setCurrentIndex(0);
                }
                DigitalDataAnalogPins.at(id/2)->setHidden(true);
            }else if(c->currentIndex() == 0){  // If input NoPullup
                DigitalMode.at(id/2)->setHidden(true);
                DigitalMode.at(id/2)->setDisabled(true);
                if(OkOutput == false){
                    ui->labelDigitalOptions->hide();
                }
                DigitalDataAnalogPins.at(id/2)->show();
            }else if(c->currentIndex() == 2){ // If input pullup
                DigitalMode.at(id/2)->setHidden(true);
                DigitalMode.at(id/2)->setDisabled(true);
                if(OkOutput == false){
                    ui->labelDigitalOptions->hide();
                }
                DigitalDataAnalogPins.at(id/2)->show();
            }
        }
    }
}

// SLOT COMBOBOXES INPUT/OUTPUT
void Dialog::SlotComboBoxDigitalMode(int id)
{
    if (QComboBox *c = qobject_cast<QComboBox *>(signalMapDigitalMode->mapping(id)))
    {
        if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
            arduino.digitalWrite(PinAnalogMode[id],c->currentIndex());
        }
    }
}

// Button to read data analog
void Dialog::on_ButtonReadAnalog_toggled(bool checked)
{
    if(checked == true){
        if(ModeReadAnalog.at(0)->isChecked()){  // Mode Discrete
            ReadOK = true;
            arduino.analogRead(0xA0,0);
            ui->ButtonReadAnalog->setChecked(false);
        }else{  // Mode Continuous
            ui->ArefDefault->setDisabled(true);
            ui->ArefExternal->setDisabled(true);
            ui->ArefInternal->setDisabled(true);

            ui->ButtonReadAnalog->setText("Stop Analog Read");
            ModeReadAnalog.at(0)->setDisabled(true);
            ModeReadAnalog.at(1)->setDisabled(true);

            ReadRTOK = true;

            ui->ButtonReadDigital->setDisabled(true);
            ui->ButtonWriteDigital->setDisabled(true);
            ui->tab_2->setDisabled(true);
            arduino.CaptureData(0,0);
        }
    }else { // Released
        if(ReadRTOK == true){
            ui->ArefDefault->setDisabled(false);
            ui->ArefExternal->setDisabled(false);
            ui->ArefInternal->setDisabled(false);

            ui->ButtonReadAnalog->setText("Start Analog Read");
            ModeReadAnalog.at(0)->setEnabled(true);
            ModeReadAnalog.at(1)->setEnabled(true);

            arduino.CaptureData(2,0);
            ReadRTOK = false;

            ui->ButtonReadDigital->setEnabled(true);
            ui->ButtonWriteDigital->setEnabled(true);
            ui->tab_2->setEnabled(true);
        }
    }
}

// Slot mode read data analog
void Dialog::SlotModeReadAnalog(int id)
{
    if (QRadioButton *c = qobject_cast<QRadioButton *>(signalMapModeReadAnalog->mapping(id)))
    {
        if(id == 0){
            ui->ButtonReadAnalog->setText("Analog Read - Once");
        } else{
            ui->ButtonReadAnalog->setText("Start Analog Read");
        }
    }
}

// Button to read digital data
void Dialog::on_ButtonReadDigital_toggled(bool checked)
{
    if(checked == true){
        if(ModeReadDigital.at(0)->isChecked()){  // Mode Discrete
            ReadOK = true;
            arduino.digitalRead();
            ui->ButtonReadDigital->setChecked(false);
        }else{  // Mode Continuous
            ui->ArefDefault->setDisabled(true);
            ui->ArefExternal->setDisabled(true);
            ui->ArefInternal->setDisabled(true);

            ui->ButtonReadDigital->setText("Stop Digital Read");
            ModeReadDigital.at(0)->setDisabled(true);
            ModeReadDigital.at(1)->setDisabled(true);

            ReadRTDigital = true;

            ui->ButtonReadAnalog->setDisabled(true);
            ui->ButtonWriteDigital->setDisabled(true);
            ui->tab_2->setDisabled(true);

            arduino.CaptureData(1,0);
        }
    }else { // Released
        if(ReadRTDigital == true){
            ui->ArefDefault->setDisabled(false);
            ui->ArefExternal->setDisabled(false);
            ui->ArefInternal->setDisabled(false);

            ui->ButtonReadDigital->setText("Start Digital Read");
            ModeReadDigital.at(0)->setEnabled(true);
            ModeReadDigital.at(1)->setEnabled(true);

            arduino.CaptureData(2,0);

            ui->ButtonReadAnalog->setEnabled(true);
            ui->ButtonWriteDigital->setEnabled(true);
            ReadRTDigital = false;
            ui->tab_2->setEnabled(true);
        }
    }
}

// Slot mode read data digital
void Dialog::SlotModeReadDigital(int id)
{
    if (QRadioButton *c = qobject_cast<QRadioButton *>(signalMapModeReadDigital->mapping(id)))
    {
        if(id == 0){
            ui->ButtonReadDigital->setText("Digital Read - Once");
        } else{
            ui->ButtonReadDigital->setText("Start Digital Read");
        }
    }
}

// Button to write digital data
void Dialog::on_ButtonWriteDigital_clicked()
{
    DiscreteOK = true;
    for(int i = 0; i < 12; i++){
        SlotDigitalMode(i);
        SlotDigitalValue(i);
        if(ComboBoxDigitalMode.at(i)->currentText() == "PWM"){
            SlotSliderPWM(i);
        }
        if(i%2 == 0 && AnalogDigMode.at(i)->currentIndex() == 1){
            SlotComboBoxAnalog(i+1);
            SlotComboBoxDigitalMode(i/2);
        }
    }
    DiscreteOK = false;
}

// Slot mode write digital data
void Dialog::SlotModeWriteDigital(int id)
{
    if (QRadioButton *c = qobject_cast<QRadioButton *>(signalMapModeWriteDigital->mapping(id)))
    {
        if(id == 0){
            ui->ButtonWriteDigital->setDisabled(true);
        } else{
            ui->ButtonWriteDigital->setEnabled(true);
        }
    }
}

// Function to show data of analog/digital pins
void Dialog::UpdateData(const bool listo)
{
    int p = 12;
    if(ReadOK){
            ReadOK = false;
            if(listo == true){ // Analog Data
                for(int i = 0; i < 6; i++){
                        AnalogData.at(i)->display(arduino.analogData(i));
                }
            }else{             // Digital Data
                for (int i = 0; i < 12; i++){
                    DigitalData.at(i)->setText(StateDigital[arduino.digitalData(i+2)]);
                }
                for(int i = 0; i < 6; i++){
                    DigitalDataAnalogPins.at(i)->setText(StateDigital[arduino.digitalData(p+2)]);
                    p++;
                }

            }
    }

    if(ReadRTOK){ // Analog Data
            for(int i = 0; i < 6; i++){
                    AnalogData.at(i)->display(arduino.analogData(i));
            }
    }

    if(ReadRTDigital){
        p = 12;
        for (int i = 0; i < 12; i++){
            DigitalData.at(i)->setText(StateDigital[arduino.digitalData(i+2)]);
        }
        for(int i = 0; i < 6; i++){
            DigitalDataAnalogPins.at(i)->setText(StateDigital[arduino.digitalData(p+2)]);
            p++;
        }
    }

    // CUSTOM PLOT CAPTURE/REAL TIME
    if(CaptureOK){
        double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
        static double lastPointKey = 0;
        float value0;
        if (key-lastPointKey > 0.01) // at most add point every 10 ms
        {
          if(ui->listAnalogPin->currentIndex() == -1){
            value0 = 5 * arduino.digitalData((ui->listDigitalPin->currentIndex()) + 2);      // To put from 0 to 5 V
          } else{
            value0 = 5.00 * arduino.analogData(ui->listAnalogPin->currentIndex()) / 1024.00; // To put from 0 to 5 V
          }
            // add data to lines:
          ui->customPlot->graph(0)->addData(key, value0);
          // set data of dots:
          ui->customPlot->graph(2)->clearData();
          ui->customPlot->graph(2)->addData(key, value0);
          ui->customPlot->graph(3)->clearData();
          if(ui->radioButton_RealTime->isChecked()){
              // remove data of lines that's outside visible range:
              ui->customPlot->graph(0)->removeDataBefore(key-8);
              ui->customPlot->graph(1)->removeDataBefore(key-8);
          }
          // rescale value (vertical) axis to fit the current data:
          ui->customPlot->graph(0)->rescaleValueAxis();
          ui->customPlot->graph(1)->rescaleValueAxis(true);
          lastPointKey = key;
        }

        // make key axis range scroll with the data (at a constant range size of 8):
        ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
        if(ui->listDigitalPin->currentIndex() != -1){
            ui->customPlot->yAxis->setRange(-0.5,6);
        }else{
            ui->customPlot->yAxis->setRange(-0.5,6);  // Axis Y fixed
        }
        if(ui->radioButton_CaptureData->isChecked()){
            // Not to move xAxis and yAxis
            ui->customPlot->xAxis->rescale(true);
        }
        ui->customPlot->replot();
    }
}

// Mode Default
void Dialog::on_ArefDefault_clicked()
{
    ui->ArefDefault->setChecked(true);
    ui->ArefExternal->setChecked(false);
    ui->ArefInternal->setChecked(false);
    arduino.analogReference(0);
}

// Mode External
void Dialog::on_ArefExternal_clicked()
{
    ui->ArefExternal->setChecked(true);
    ui->ArefDefault->setChecked(false);
    ui->ArefInternal->setChecked(false);
    arduino.analogReference(1);
}

// Mode Internal
void Dialog::on_ArefInternal_clicked()
{
    ui->ArefInternal->setChecked(true);
    ui->ArefDefault->setChecked(false);
    ui->ArefExternal->setChecked(false);
    arduino.analogReference(2);
}

// SLOT MODE CHANGE COMBOBOX DIGITAL MODE
// link: https://forum.qt.io/topic/38296/qvector-qcombobox-signal-and-slot-problem/8
// link: http://doc.qt.io/qt-4.8/qsignalmapper.html
void Dialog::SlotDigitalMode(int id)
{
    int pin,mode;
    bool okValue = false;
    bool okPWM = false;
    bool AnyServo = false;
    if (QComboBox *c = qobject_cast<QComboBox *>(signalMapDigMode->mapping(id)))
    {
        for(int i=0;i<12;i++){
            if(ComboBoxDigitalMode.at(i)->currentText() == "PWM" && okPWM == false){
                okPWM = true;
            }

            if(ComboBoxDigitalMode.at(i)->currentText() == "Servo"){
                ComboBoxDigitalMode.at(7)->setItemData(2,0,Qt::UserRole-1);
                ComboBoxDigitalMode.at(8)->setItemData(2,0,Qt::UserRole-1);
                AnyServo = true;
            }

            // Disable Servo in tab Servo config for another mode
            if((i == 1 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9) && ComboBoxDigitalMode.at(i)->currentText() != "Servo"){
                //Disable servo
                ActServo.at(PinServoReverse[i])->setChecked(false);
                ActServo.at(PinServoReverse[i])->clicked(true);
            }

            if(ComboBoxDigitalMode.at(i)->currentText() == "Output" && okValue == false){
                okValue = true;
            }
        }

        if(AnyServo == false){
            ComboBoxDigitalMode.at(7)->setItemData(2,33,Qt::UserRole-1);
            ComboBoxDigitalMode.at(8)->setItemData(2,33,Qt::UserRole-1);
        }

        if(ComboBoxDigitalMode.at(7)->currentText() != "PWM" && ComboBoxDigitalMode.at(8)->currentText() != "PWM"){
            ui->tab_3->setDisabled(false);
            ComboBoxDigitalMode.at(1)->setItemData(3,33,Qt::UserRole-1);
            ComboBoxDigitalMode.at(3)->setItemData(3,33,Qt::UserRole-1);
            ComboBoxDigitalMode.at(4)->setItemData(3,33,Qt::UserRole-1);
            ComboBoxDigitalMode.at(7)->setItemData(3,33,Qt::UserRole-1);
            ComboBoxDigitalMode.at(8)->setItemData(3,33,Qt::UserRole-1);
            ComboBoxDigitalMode.at(9)->setItemData(3,33,Qt::UserRole-1);
        }

        if(ComboBoxDigitalMode.at(1)->currentText() == "PWM"){
            ui->tab_tone->setDisabled(true);
        }else if(ComboBoxDigitalMode.at(9)->currentText() == "PWM"){
            ui->tab_tone->setDisabled(true);
        }else{
            ui->tab_tone->setEnabled(true);
        }

        pin = id + 2;
        mode = c->currentIndex();
        if(c->currentText() == "PWM"){ // If PWM mode
            if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                arduino.pinMode(pin,3);
            }
            // Disable servo
            if(id == 7 || id == 8){
                ui->tab_3->setDisabled(true);
                ComboBoxDigitalMode.at(1)->setItemData(3,0,Qt::UserRole-1);
                ComboBoxDigitalMode.at(3)->setItemData(3,0,Qt::UserRole-1);
                ComboBoxDigitalMode.at(4)->setItemData(3,0,Qt::UserRole-1);
                ComboBoxDigitalMode.at(7)->setItemData(3,0,Qt::UserRole-1);
                ComboBoxDigitalMode.at(8)->setItemData(3,0,Qt::UserRole-1);
                ComboBoxDigitalMode.at(9)->setItemData(3,0,Qt::UserRole-1);
            }
            SliderPWM.at(PWMPinReverse[id])->show();
            SliderPWM.at(PWMPinReverse[id])->setDisabled(false);
            InfoPWM.at(PWMPinReverse[id])->show();
            InfoPWM.at(PWMPinReverse[id])->setDisabled(false);
            if(!okValue){
                ui->label_DigitalValue->hide();
            }
            ui->label_PWM->show();

            // Hide digital data
            DigitalData.at(id)->hide();

            // If currentIndex = Item disabled... -> set current index -1
            if(ui->listDigitalPin->currentIndex() == id){
                ui->listDigitalPin->setCurrentIndex(-1);
                if(ui->listAnalogPin->currentIndex() == -1){
                    ui->StartCapture->setDisabled(true);
                    ui->CaptureDuration->setDisabled(true);
                    ui->SaveGraph->setDisabled(true);
                    ui->radioButton_CaptureData->setDisabled(true);
                    ui->radioButton_RealTime->setDisabled(true);
                }
            }

            // Hide combobox and reset index -1
            ComboBoxDigitalValue.at(id)->setCurrentIndex(-1);
            ComboBoxDigitalValue.at(id)->hide();
            ComboBoxDigitalValue.at(id)->setDisabled(true);

            // Disable modes for Interrupt 1
            if(pin == 3){
                ui->listModesInt1->setDisabled(true);
                ui->DisableInt1->setDisabled(true);
            }
        }else if(c->currentText() == "Servo"){  // Mode Servo (mode = 3)

            // Enable servo
            ActServo.at(PinServoReverse[id])->setChecked(true);
            ActServo.at(PinServoReverse[id])->clicked(true);

            if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                arduino.pinMode(pin,4);
            }
            if(!okValue){
                ui->label_DigitalValue->hide();
            }

            // Hide combobox and reset index -1
            ComboBoxDigitalValue.at(id)->setCurrentIndex(-1);
            ComboBoxDigitalValue.at(id)->hide();
            ComboBoxDigitalValue.at(id)->setDisabled(true);

            // Hide combobox and reset value 0
            SliderPWM.at(PWMPinReverse[id])->setValue(0);
            SliderPWM.at(PWMPinReverse[id])->hide();
            InfoPWM.at(PWMPinReverse[id])->hide();

            // Hide digital data
            DigitalData.at(id)->hide();

            // If currentIndex = Item disabled... -> set current index -1
            if(ui->listDigitalPin->currentIndex() == id){
                ui->listDigitalPin->setCurrentIndex(-1);
                if(ui->listAnalogPin->currentIndex() == -1){
                    ui->StartCapture->setDisabled(true);
                    ui->CaptureDuration->setDisabled(true);
                    ui->SaveGraph->setDisabled(true);
                    ui->radioButton_CaptureData->setDisabled(true);
                    ui->radioButton_RealTime->setDisabled(true);
                }
            }

            if(!okPWM){
                ui->label_PWM->hide();
            }

            // Disable modes for Interrupt 1
            if(pin == 3){
                ui->listModesInt1->setDisabled(true);
                ui->DisableInt1->setDisabled(true);
            }
        }else if(c->currentText() == "Input-PullUp"){ // If not PWM-->PullUp mode
            if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                arduino.pinMode(pin,11);
            }
            // Hide combobox and reset value 0
            SliderPWM.at(PWMPinReverse[id])->setValue(0);
            SliderPWM.at(PWMPinReverse[id])->hide();
            InfoPWM.at(PWMPinReverse[id])->hide();
            if(!okValue){
                ui->label_DigitalValue->hide();
            }
            if(!okPWM){
                ui->label_PWM->hide();
            }

            // Hide combobox and reset index -1
            ComboBoxDigitalValue.at(id)->setCurrentIndex(-1);
            ComboBoxDigitalValue.at(id)->hide();
            ComboBoxDigitalValue.at(id)->setDisabled(true);

            // If currentIndex = Item disabled... -> set current idex -1
            if(ui->listDigitalPin->currentIndex() == id){
                ui->listDigitalPin->setCurrentIndex(-1);
                if(ui->listAnalogPin->currentIndex() == -1){
                    ui->StartCapture->setDisabled(true);
                    ui->CaptureDuration->setDisabled(true);
                    ui->SaveGraph->setDisabled(true);
                    ui->radioButton_CaptureData->setDisabled(true);
                    ui->radioButton_RealTime->setDisabled(true);
                }
            }

            // Enable interrupts
            if(pin == 2){
                ui->listModesInt0->setDisabled(false);
                ui->DisableInt0->setDisabled(false);
            } else if(pin == 3){
                ui->listModesInt1->setDisabled(false);
                ui->DisableInt1->setDisabled(false);
            }


        }else if(c->currentText() == "Output" || c->currentText() == "Input-NoPullUp"){ // If Input/Output
            if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                arduino.pinMode(pin,c->currentIndex());
            }
            // Hide combobox and reset value 0
            SliderPWM.at(PWMPinReverse[id])->setValue(0);
            SliderPWM.at(PWMPinReverse[id])->hide();
            InfoPWM.at(PWMPinReverse[id])->hide();

            if(mode == 1){  // MODE OUTPUT
                if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                    ComboBoxDigitalValue.at(id)->setCurrentIndex(0);
                }
                ComboBoxDigitalValue.at(id)->show();
                ComboBoxDigitalValue.at(id)->setDisabled(false);
                ui->label_DigitalValue->show();
                DigitalData.at(id)->hide();

                // If currentIndex = Item disabled... -> set current index -1
                if(ui->listDigitalPin->currentIndex() == id){
                    ui->listDigitalPin->setCurrentIndex(-1);
                    if(ui->listAnalogPin->currentIndex() == -1){
                        ui->StartCapture->setDisabled(true);
                        ui->CaptureDuration->setDisabled(true);
                        ui->SaveGraph->setDisabled(true);
                        ui->radioButton_CaptureData->setDisabled(true);
                        ui->radioButton_RealTime->setDisabled(true);
                    }
                }

                if(!okPWM){
                    ui->label_PWM->hide();
                }
                // Disable modes for Interrupt 0/1
                if(pin == 2){
                    ui->listModesInt0->setDisabled(true);
                    ui->DisableInt0->setDisabled(true);
                } else if(pin == 3){
                    ui->listModesInt1->setDisabled(true);
                    ui->DisableInt1->setDisabled(true);
                }
            }else{           // MODE INPUT-NOPULLUP
                // Hide combobox and reset index -1
                ComboBoxDigitalValue.at(id)->setCurrentIndex(-1);
                ComboBoxDigitalValue.at(id)->hide();
                ComboBoxDigitalValue.at(id)->setDisabled(true);

                // Show digital data
                DigitalData.at(id)->show();

                if(!okValue){
                    ui->label_DigitalValue->hide();
                }
                if(!okPWM){
                    ui->label_PWM->hide();
                }
                if(pin == 2){
                    ui->listModesInt0->setDisabled(false);
                    ui->DisableInt0->setDisabled(false);
                } else if(pin == 3){
                    ui->listModesInt1->setDisabled(false);
                    ui->DisableInt1->setDisabled(false);
                }
            }
        }
    }
}

// SLOT VALUE CHANGE COMBOBOX DIGITAL PINS
void Dialog::SlotDigitalValue(int id)
{
    int pin,value;
    if (QComboBox *c = qobject_cast<QComboBox *>(signalMapDigValue->mapping(id)))
    {
        pin = id + 2;
        value = c->currentIndex();
        if(value != -1){
            if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
                arduino.digitalWrite(pin,value);
            }
        }
    }
}

// SLOT SLIDER VALUE CHANGE
void Dialog::SlotSliderPWM(int id)
{
    int a,ciclo;
    if (QSlider *c = qobject_cast<QSlider *>(signalMapPWM->mapping(id)))
    {
        a = 255 * (c->value());
        ciclo = a/100;
        if(ModeWriteDigital.at(0)->isChecked() || DiscreteOK){
            arduino.analogWrite(PWMPin[id],ciclo);
        }
        InfoPWM.at(id)->setText(QString("<span style=\" font-size:10pt; font-weight:400;\">( %1 %)</span>").arg(c->value()));
    }
}

// Set config when current index from list change
void Dialog::on_listAnalogPin_currentIndexChanged(int index)
{
    if(index != -1){
        AnalogDigMode.at(index * 2)->setCurrentIndex(0);

        ui->radioButton_CaptureData->setEnabled(true);
        ui->radioButton_RealTime->setEnabled(true);
        if(ui->radioButton_RealTime->isChecked()){
            ui->StartCapture->setEnabled(true);
            ui->SaveGraph->setEnabled(true);
        }
        ui->listDigitalPin->setCurrentIndex(-1);
    }else if (ui->listDigitalPin->currentIndex() == -1){
        ui->radioButton_CaptureData->setDisabled(true);
        ui->radioButton_RealTime->setDisabled(true);
        ui->StartCapture->setDisabled(true);
    }
}

// Set config when current index from list change
void Dialog::on_listDigitalPin_currentIndexChanged(int index)
{
    if(index != -1){
        if(index < 12){
            ComboBoxDigitalMode.at(index)->setCurrentIndex(0);
        }else{
            AnalogDigMode.at((index-12)*2)->setCurrentIndex(1);     // Set digital
            AnalogDigMode.at((index-12)*2+1)->setCurrentIndex(0);   // Set input
        }

        ui->radioButton_CaptureData->setEnabled(true);
        ui->radioButton_RealTime->setEnabled(true);
        if(ui->radioButton_RealTime->isChecked()){
            ui->StartCapture->setEnabled(true);
            ui->SaveGraph->setEnabled(true);
        }
        ui->listAnalogPin->setCurrentIndex(-1);
    }else if (ui->listAnalogPin->currentIndex() == -1){
        ui->radioButton_CaptureData->setDisabled(true);
        ui->radioButton_RealTime->setDisabled(true);
        ui->StartCapture->setDisabled(true);
    }
}

// Mode Capture data
void Dialog::on_radioButton_CaptureData_clicked()
{
    ui->CaptureDuration->setEnabled(true);
    if(ui->CaptureDuration->value() == 0){
        ui->StartCapture->setDisabled(true);
        ui->SaveGraph->setDisabled(true);
    }
}

// Mode Real time
void Dialog::on_radioButton_RealTime_clicked()
{
    ui->CaptureDuration->setDisabled(true);
    ui->StartCapture->setEnabled(true);
    ui->SaveGraph->setEnabled(true);
}

// Time to capture data
void Dialog::on_CaptureDuration_valueChanged(int arg1)
{
    if(arg1 > 0){
        ui->StartCapture->setEnabled(true);
        ui->SaveGraph->setEnabled(true);
    } else if(arg1 == 0){
        ui->StartCapture->setDisabled(true);
        ui->SaveGraph->setDisabled(true);
    }
}

// Init receive data
void Dialog::on_StartCapture_clicked()
{
    ui->ArefDefault->setDisabled(true);
    ui->ArefExternal->setDisabled(true);
    ui->ArefInternal->setDisabled(true);

    CaptureOK = true;
    ui->customPlot->graph(0)->clearData();
    ui->customPlot->graph(2)->clearData();
    ui->StartCapture->setDisabled(true);
    ui->StopCapture->setEnabled(true);
    ui->ButtonReadDigital->setDisabled(true);
    ui->ButtonReadAnalog->setDisabled(true);
    ui->ButtonWriteDigital->setDisabled(true);
    ui->listAnalogPin->setDisabled(true);
    ui->listDigitalPin->setDisabled(true);
    if(ui->listAnalogPin->currentIndex() == -1){
        if(ui->radioButton_CaptureData->isChecked()){
            arduino.CaptureData(1,ui->CaptureDuration->value());
            ui->radioButton_RealTime->setDisabled(true);
            ui->radioButton_CaptureData->setDisabled(true);
        }else{
            arduino.CaptureData(1,0);
            ui->radioButton_RealTime->setDisabled(true);
            ui->radioButton_CaptureData->setDisabled(true);
        }
    }else{
        if(ui->radioButton_CaptureData->isChecked()){
            arduino.CaptureData(0,ui->CaptureDuration->value());
            ui->radioButton_RealTime->setDisabled(true);
            ui->radioButton_CaptureData->setDisabled(true);
        }else{
            arduino.CaptureData(0,0);
            ui->radioButton_RealTime->setDisabled(true);
            ui->radioButton_CaptureData->setDisabled(true);
        }
    }
}

// Stop data
void Dialog::on_StopCapture_clicked()
{
    ui->ArefDefault->setDisabled(false);
    ui->ArefExternal->setDisabled(false);
    ui->ArefInternal->setDisabled(false);

    CaptureOK = false;
    arduino.CaptureData(2,0);
    ui->StopCapture->setDisabled(true);
    ui->StartCapture->setEnabled(true);
    ui->ButtonReadDigital->setEnabled(true);
    ui->ButtonReadAnalog->setEnabled(true);
    ui->ButtonWriteDigital->setEnabled(true);
    ui->listAnalogPin->setEnabled(true);
    ui->listDigitalPin->setEnabled(true);
    ui->radioButton_RealTime->setEnabled(true);
    ui->radioButton_CaptureData->setEnabled(true);
}

// Receive signal from Arduino
void Dialog::EndReceiveData(const bool stop)
{
    ui->ArefDefault->setDisabled(false);
    ui->ArefExternal->setDisabled(false);
    ui->ArefInternal->setDisabled(false);

    ui->StartCapture->setEnabled(true);
    ui->StopCapture->setDisabled(true);
    ui->ButtonReadDigital->setEnabled(true);
    ui->ButtonReadAnalog->setEnabled(true);
    ui->ButtonWriteDigital->setEnabled(true);
    ui->listAnalogPin->setEnabled(true);
    ui->listDigitalPin->setEnabled(true);
    CaptureOK = false;
}

// Save Graph
//+Info: http://www.qcustomplot.com/index.php/support/forum/201
void Dialog::on_SaveGraph_clicked()
{
    //QString fileName = "graph.jpg" ;
    QString fileName;
    fileName.append("graph_");
    QTime time = QTime::currentTime();
    fileName.append(QString::number(time.hour()));
    fileName.append("-");
    fileName.append(QString::number(time.minute()));
    fileName.append("-");
    fileName.append(QString::number(time.second()));
    fileName.append(".jpg");

    QFile file(QDir::currentPath()+"/"+fileName);

    if (!file.open(QIODevice::WriteOnly|QFile::WriteOnly))
    {
         QMessageBox::warning(0,"Could not create Project File",
                              QObject::tr( "\n Could not create Project File on disk"));
    }
    ui->customPlot->saveJpg(QDir::currentPath()+"/"+fileName,0, 0, 1.0, -1);
}

// Generate a tone in a digital pin (2-13)
void Dialog::SlotSetTone(int id)
{
    if (QPushButton *c = qobject_cast<QPushButton *>(signalMapSetTone->mapping(id)))
    {
        ActiveTone = id + 2;

        ComboBoxDigitalMode.at(id)->setCurrentIndex(1);

        arduino.tone(id + 2,(Frequency.at(id)->value())* 4, DurationTone.at(id)->value());

        for(int i = 0; i < 12; i++){
            if(id != i){
                SetTone.at(i)->setDisabled(true);
                SetNoTone.at(i)->setDisabled(true);
                Frequency.at(i)->setDisabled(true);
                DurationTone.at(i)->setDisabled(true);
            }
        }
        for(int i = 0; i < 6; i++){
            SetToneA.at(i)->setDisabled(true);
            SetNoToneA.at(i)->setDisabled(true);
            FrequencyA.at(i)->setDisabled(true);
            DurationToneA.at(i)->setDisabled(true);
        }

        ComboBoxDigitalMode.at(1)->setItemData(2,0,Qt::UserRole-1);
        ComboBoxDigitalMode.at(9)->setItemData(2,0,Qt::UserRole-1);
    }
}

// Stop tone in a pin (2-13)
void Dialog::SlotSetNoTone(int id)
{
    if (QPushButton *c = qobject_cast<QPushButton *>(signalMapSetNoTone->mapping(id)))
    {
        ActiveTone = 0;
        arduino.noTone(id + 2);

        ComboBoxDigitalValue.at(id)->setCurrentIndex(0);
        ComboBoxDigitalValue.at(id)->setCurrentIndex(1);

        for(int i = 0; i < 12; i++){
            if(id != i){
                SetTone.at(i)->setDisabled(false);
                SetNoTone.at(i)->setDisabled(false);
                Frequency.at(i)->setDisabled(false);
                DurationTone.at(i)->setDisabled(false);
            }
        }
        for(int i = 0; i < 6; i++){
            SetToneA.at(i)->setDisabled(false);
            SetNoToneA.at(i)->setDisabled(false);
            FrequencyA.at(i)->setDisabled(false);
            DurationToneA.at(i)->setDisabled(false);
        }

        ComboBoxDigitalMode.at(1)->setItemData(2,33,Qt::UserRole-1);
        ComboBoxDigitalMode.at(9)->setItemData(2,33,Qt::UserRole-1);

        if(ComboBoxDigitalMode.at(1)->currentText() == "PWM")
        {
            SetTone.at(1)->setDisabled(true);
            SetNoTone.at(1)->setDisabled(true);
            Frequency.at(1)->setDisabled(true);
            DurationTone.at(1)->setDisabled(true);
        }

        if(ComboBoxDigitalMode.at(9)->currentText() == "PWM")
        {
            SetTone.at(9)->setDisabled(true);
            SetNoTone.at(9)->setDisabled(true);
            Frequency.at(9)->setDisabled(true);
            DurationTone.at(9)->setDisabled(true);
        }   
    }
}

// Generate a tone in a digital pin (14-19)
void Dialog::SlotSetToneA(int id)
{
    if (QPushButton *c = qobject_cast<QPushButton *>(signalMapSetToneA->mapping(id)))
    {
        ActiveTone = id + 14;

        //Mode Digital
        AnalogDigMode.at(id*2)->setCurrentIndex(1);
        //Mode Output
        AnalogDigMode.at((id*2)+1)->setCurrentIndex(1);
        DigitalMode.at(id)->setCurrentIndex(1);
        arduino.tone(id + 14,(FrequencyA.at(id)->value())* 4, DurationToneA.at(id)->value());
        for(int i = 0; i < 12; i++){
            SetTone.at(i)->setDisabled(true);
            SetNoTone.at(i)->setDisabled(true);
            Frequency.at(i)->setDisabled(true);
            DurationTone.at(i)->setDisabled(true);
        }
        for(int i = 0; i < 6; i++){
            if(id != i){
                SetToneA.at(i)->setDisabled(true);
                SetNoToneA.at(i)->setDisabled(true);
                FrequencyA.at(i)->setDisabled(true);
                DurationToneA.at(i)->setDisabled(true);
            }
        }
        ComboBoxDigitalMode.at(1)->setItemData(2,0,Qt::UserRole-1);
        ComboBoxDigitalMode.at(9)->setItemData(2,0,Qt::UserRole-1);
    }
}

//Stop tone in a pin (14-19)
void Dialog::SlotSetNoToneA(int id)
{
    if (QPushButton *c = qobject_cast<QPushButton *>(signalMapSetNoToneA->mapping(id)))
    {
        ActiveTone = 0;
        arduino.noTone(id + 14);
        ComboBoxDigitalMode.at(1)->setItemData(2,33,Qt::UserRole-1);
        ComboBoxDigitalMode.at(9)->setItemData(2,33,Qt::UserRole-1);

        for(int i = 0; i < 12; i++){
            SetTone.at(i)->setDisabled(false);
            SetNoTone.at(i)->setDisabled(false);
            Frequency.at(i)->setDisabled(false);
            DurationTone.at(i)->setDisabled(false);
        }
        for(int i = 0; i < 6; i++){
            if(id != i){
                SetToneA.at(i)->setDisabled(false);
                SetNoToneA.at(i)->setDisabled(false);
                FrequencyA.at(i)->setDisabled(false);
                DurationToneA.at(i)->setDisabled(false);
            }
        }
        if(ComboBoxDigitalMode.at(1)->currentText() == "PWM")
        {
            SetTone.at(1)->setDisabled(true);
            SetNoTone.at(1)->setDisabled(true);
            Frequency.at(1)->setDisabled(true);
            DurationTone.at(1)->setDisabled(true);
        }

        if(ComboBoxDigitalMode.at(9)->currentText() == "PWM")
        {
            SetTone.at(9)->setDisabled(true);
            SetNoTone.at(9)->setDisabled(true);
            Frequency.at(9)->setDisabled(true);
            DurationTone.at(9)->setDisabled(true);
        }
        DigitalMode.at(id)->setCurrentIndex(0);
        DigitalMode.at(id)->setCurrentIndex(1);
    }
}

// Signal from Arduino to stop tone
void Dialog::EndTimeTone(const bool stop){
    for(int i = 0; i < 12; i++){
        if(i < 6){
            if(SetToneA.at(i)->isEnabled()){
                SlotSetNoToneA(i);
            }
        }
        if(SetTone.at(i)->isEnabled()){
            SlotSetNoTone(i);
        }
    }
}

// Activate a servo in a pin
void Dialog::SlotActServo(int id)
{
    if (QCheckBox *c = qobject_cast<QCheckBox *>(signalMapActServo->mapping(id)))
    {
        if(c->isChecked()){
            SlotMinPulse(id);

            ComboBoxDigitalMode.at(PinServo[id])->setCurrentIndex(3);
            MinPulse.at(id)->setDisabled(true);
            MaxPulse.at(id)->setDisabled(true);
            SliderServo.at(id)->setEnabled(true);
        }else{
            // Set input
            if(ComboBoxDigitalMode.at(PinServo[id])->currentText() == "Servo"){
                ComboBoxDigitalMode.at(PinServo[id])->setCurrentIndex(0);
            }
            MinPulse.at(id)->setEnabled(true);
            MaxPulse.at(id)->setEnabled(true);
            SliderServo.at(id)->setDisabled(true);
            SliderServo.at(id)->setValue(0);
        }
    }
}

// Set minimum pulse to wave
void Dialog::SlotMinPulse(int id)
{
    if (QSpinBox *c = qobject_cast<QSpinBox *>(signalMapMinPulse->mapping(id)))
    {
        arduino.ServoConfig(PWMPin[id],MinPulse.at(id)->value(),MaxPulse.at(id)->value());
    }
}

// Set maximum pulse to wave
void Dialog::SlotMaxPulse(int id)
{
    if (QSpinBox *c = qobject_cast<QSpinBox *>(signalMapMaxPulse->mapping(id)))
    {
        arduino.ServoConfig(PWMPin[id],MinPulse.at(id)->value(),MaxPulse.at(id)->value());
    }
}

// Change value to move servo
void Dialog::SlotSliderServo(int id)
{
    if (QSlider *c = qobject_cast<QSlider *>(signalMapSliderServo->mapping(id)))
    {
        arduino.analogWrite(PWMPin[id],SliderServo.at(id)->value());
        degreeServo.at(id)->setText(QString("<span style=\" font-size:10pt; font-weight:400;\">( %1 º)</span>").arg(SliderServo.at(id)->value()));
    }
}

// Show interrupts received from digital pin 2-3
void Dialog::ShowInterrupt(const int number)
{
    if(number == 0)
    {
        ContInt1 = 0;
        ContInt0++;
    }else
    {
        ContInt0 = 0;
        ContInt1++;
    }
    if(number == 0)
        ui->InfoInterrupt->setText(QString("Interrupt 0 detected, %1 consecutives").arg(ContInt0));
    else
        ui->InfoInterrupt->setText(QString("Interrupt 1 detected, %1 consecutives").arg(ContInt1));
}

// Interrupt mode for Interrupt 0
void Dialog::on_listModesInt0_currentIndexChanged(int index)
{
    if(index != -1)
    {
        arduino.attachInterrupt(0,index);
    }
}

// Interrupt mode for Interrupt 1
void Dialog::on_listModesInt1_currentIndexChanged(int index)
{
    if(index != -1)
    {
        arduino.attachInterrupt(1,index);
    }
}

// Disable interrupt 0
void Dialog::on_DisableInt0_clicked()
{
    ui->listModesInt0->setCurrentIndex(-1);
    arduino.detachInterrupt(0);
}

// Disable interrupt 0
void Dialog::on_DisableInt1_clicked()
{
    ui->listModesInt1->setCurrentIndex(-1);
    arduino.detachInterrupt(1);
}

// Show data received from I2C slave
void Dialog::ShowDataI2C(const int length)
{
    QString info = QString("Read from address Slave %1,").arg(arduino.SlaveDataI2C(0));
    info.append(QString(" in register %1: ").arg(arduino.SlaveDataI2C(1)));
    for(int i = 2; i < length; i++){
        info.append(QString::number(arduino.SlaveDataI2C(i)));
    }
    on_ShowText(info);
    ui->Info_ReadI2C->setText(info);
}

// Signal produced when I2C is detected
void Dialog::ShowI2CDetected(const bool detected)
{
    QString hexadecimal;
    hexadecimal.setNum(arduino.I2CAddr(),16);
    ui->listI2Cdevices->addItem(hexadecimal);
}

// Send data to LCD
void Dialog::on_ShowInLCD_clicked()
{
    arduino.SendString(ui->TextToLCD->text());
}

// Search I2C devices connected
void Dialog::on_I2C_Scan_clicked()
{
    ui->listI2Cdevices->clear();
    arduino.I2C_Scan();
}

// Set I2C config
void Dialog::on_I2C_Config_clicked()
{
    if(ui->ChangeDelayTime->isChecked()){
        if(ui->EnPowerPins->isChecked()){

            AnalogDigMode.at(4)->setCurrentIndex(1);
            AnalogDigMode.at(5)->setCurrentIndex(1);
            DigitalMode.at(2)->setCurrentIndex(0);

            AnalogDigMode.at(6)->setCurrentIndex(1);
            AnalogDigMode.at(7)->setCurrentIndex(1);
            DigitalMode.at(3)->setCurrentIndex(1);
            arduino.I2CConfig(5,ui->I2CDelayTime->value());
        }else{
            arduino.I2CConfig(0,ui->I2CDelayTime->value());
        }
    }else if(ui->EnPowerPins->isChecked()){

        AnalogDigMode.at(4)->setCurrentIndex(1);
        AnalogDigMode.at(5)->setCurrentIndex(1);
        DigitalMode.at(2)->setCurrentIndex(0);

        AnalogDigMode.at(6)->setCurrentIndex(1);
        AnalogDigMode.at(7)->setCurrentIndex(1);
        DigitalMode.at(3)->setCurrentIndex(1);
        arduino.I2CConfig(5,0);
    }else{      // unchecked both
        // Set input
        AnalogDigMode.at(4)->setCurrentIndex(0);
        AnalogDigMode.at(6)->setCurrentIndex(0);
    }
}

// Send information to I2C slave
void Dialog::on_Write_I2C_clicked()
{
    if(ui->dataSendI2C->text().size() > 0){
        arduino.I2CRequestWrite(ui->listI2Cdevices->currentText().toInt(),ui->dataSendI2C->text(),0);
    }
}

// Read data from I2C device
void Dialog::on_Read_I2C_clicked()
{
    if(ui->radioButton_Once->isChecked()){
        arduino.I2CRequestRead(ui->listI2Cdevices->currentText().toInt(),0,ui->BytesToRead->value(),8);
    } else{
        ui->Stop_I2C->setEnabled(true);
        ui->Read_I2C->setDisabled(true);
        arduino.I2CRequestRead(ui->listI2Cdevices->currentText().toInt(),0,ui->BytesToRead->value(),16); //16 en Hex es 10, que es lo que busco
    }
}

// Finish read data
void Dialog::on_Stop_I2C_clicked()
{
    ui->Read_I2C->setEnabled(true);
    ui->Stop_I2C->setDisabled(true);
    arduino.I2CRequestRead(ui->listI2Cdevices->currentText().toInt(),0,ui->BytesToRead->value(),24);   // 24 en Hex es 18, que es lo que busco
}

// Select a I2C device
void Dialog::on_listI2Cdevices_currentIndexChanged(int index)
{
    if(index == -1){
        ui->ChangeDelayTime->setDisabled(true);
        ui->I2CDelayTime->setDisabled(true);
        //ui->I2C_Config->setDisabled(true);
        ui->Write_I2C->setDisabled(true);
        ui->BytesToRead->setDisabled(true);
        ui->radioButton_Cont->setDisabled(true);
        ui->radioButton_Once->setDisabled(true);
        ui->Read_I2C->setDisabled(true);
        ui->Stop_I2C->setDisabled(true);
        ui->I2C_Scan->setDisabled(false);
        ui->ShowInLCD->setDisabled(true);
        ui->listI2Cdevices->setDisabled(false);
        ui->ChangeDelayTime->setChecked(false);
    }else{
        ui->ChangeDelayTime->setDisabled(false);
        ui->I2CDelayTime->setDisabled(true);
        ui->EnPowerPins->setDisabled(false);
        //ui->I2C_Config->setDisabled(true);
        ui->Write_I2C->setDisabled(false);
        ui->BytesToRead->setDisabled(false);
        ui->radioButton_Cont->setDisabled(false);
        ui->radioButton_Once->setDisabled(false);
        ui->Read_I2C->setDisabled(true);
        ui->Stop_I2C->setDisabled(true);
        ui->I2C_Scan->setDisabled(false);
        ui->listI2Cdevices->setDisabled(false);
        if(!ui->ChangeDelayTime->isChecked()){
            ui->I2CDelayTime->setDisabled(true);
        }
        /*if(!ui->ChangeDelayTime->isChecked() && !ui->EnPowerPins->isChecked()){
            ui->I2C_Config->setDisabled(true);
        }*/
        if(ui->BytesToRead->value() == 0){
            ui->Read_I2C->setDisabled(true);
        }

        if(ui->listI2Cdevices->currentText() == "27"){
            ui->ShowInLCD->setEnabled(true);
            ui->Write_I2C->setDisabled(true);
            ui->Read_I2C->setDisabled(true);
        }else{
            ui->ShowInLCD->setDisabled(true);
            ui->Write_I2C->setEnabled(true);
            ui->Read_I2C->setEnabled(true);
        }
    }
}

// Num bytes to read
void Dialog::on_BytesToRead_valueChanged(int arg1)
{
    if(arg1 > 0 && ui->listI2Cdevices->currentText() != "27"){
        ui->Read_I2C->setEnabled(true);
    }else if(arg1 <= 0 && ui->listI2Cdevices->currentText() != "27"){
        ui->Read_I2C->setDisabled(true);
    }
}
