/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *tabSerial;
    QVBoxLayout *verticalLayout_Serial;
    QGridLayout *gridLayout;
    QLabel *portNameLabel;
    QComboBox *portNameComboBox;
    QPushButton *refreshButton;
    QLabel *baudRateLabel;
    QComboBox *baudRateComboBox;
    QLabel *dataBitsLabel;
    QComboBox *dataBitsComboBox;
    QLabel *parityLabel;
    QComboBox *parityComboBox;
    QLabel *stopBitsLabel;
    QComboBox *stopBitsComboBox;
    QLabel *flowControlLabel;
    QComboBox *flowControlComboBox;
    QPushButton *connectButton;
    QLabel *statusLabel;
    QLabel *labelRx;
    QPlainTextEdit *textRx;
    QLabel *labelTx;
    QHBoxLayout *horizontalLayout_Tx;
    QLineEdit *lineTx;
    QPushButton *btnSend;
    QWidget *tabChart;
    QVBoxLayout *chartLayout;
    QPushButton *btnClearPlot;
    QScrollBar *vScrollBar;
    QScrollBar *hScrollBar;
    QPushButton *btnCalOrigin;
    QPushButton *btnCal10_10;
    QPushButton *btnCalN10_N10;
    QPushButton *btnToggleCoord;
    QLabel *labelDev1Coord;
    QPushButton *btnAddDevice;
    QPushButton *btnDelDevice;
    QLabel *labelDeviceCount;
    QComboBox *comboDeviceId;
    QLineEdit *lineManualX;
    QLineEdit *lineManualY;
    QPushButton *btnAddPoint;
    QComboBox *comboClearTarget;
    QPushButton *btnClearMarkers;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1225, 750);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(20, 20, 1041, 621));
        tabWidget->setMaximumSize(QSize(16777215, 16777215));
        tabSerial = new QWidget();
        tabSerial->setObjectName("tabSerial");
        verticalLayout_Serial = new QVBoxLayout(tabSerial);
        verticalLayout_Serial->setObjectName("verticalLayout_Serial");
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        portNameLabel = new QLabel(tabSerial);
        portNameLabel->setObjectName("portNameLabel");

        gridLayout->addWidget(portNameLabel, 0, 0, 1, 1);

        portNameComboBox = new QComboBox(tabSerial);
        portNameComboBox->setObjectName("portNameComboBox");

        gridLayout->addWidget(portNameComboBox, 0, 1, 1, 1);

        refreshButton = new QPushButton(tabSerial);
        refreshButton->setObjectName("refreshButton");

        gridLayout->addWidget(refreshButton, 0, 2, 1, 1);

        baudRateLabel = new QLabel(tabSerial);
        baudRateLabel->setObjectName("baudRateLabel");

        gridLayout->addWidget(baudRateLabel, 1, 0, 1, 1);

        baudRateComboBox = new QComboBox(tabSerial);
        baudRateComboBox->setObjectName("baudRateComboBox");

        gridLayout->addWidget(baudRateComboBox, 1, 1, 1, 1);

        dataBitsLabel = new QLabel(tabSerial);
        dataBitsLabel->setObjectName("dataBitsLabel");

        gridLayout->addWidget(dataBitsLabel, 2, 0, 1, 1);

        dataBitsComboBox = new QComboBox(tabSerial);
        dataBitsComboBox->setObjectName("dataBitsComboBox");

        gridLayout->addWidget(dataBitsComboBox, 2, 1, 1, 1);

        parityLabel = new QLabel(tabSerial);
        parityLabel->setObjectName("parityLabel");

        gridLayout->addWidget(parityLabel, 3, 0, 1, 1);

        parityComboBox = new QComboBox(tabSerial);
        parityComboBox->setObjectName("parityComboBox");

        gridLayout->addWidget(parityComboBox, 3, 1, 1, 1);

        stopBitsLabel = new QLabel(tabSerial);
        stopBitsLabel->setObjectName("stopBitsLabel");

        gridLayout->addWidget(stopBitsLabel, 4, 0, 1, 1);

        stopBitsComboBox = new QComboBox(tabSerial);
        stopBitsComboBox->setObjectName("stopBitsComboBox");

        gridLayout->addWidget(stopBitsComboBox, 4, 1, 1, 1);

        flowControlLabel = new QLabel(tabSerial);
        flowControlLabel->setObjectName("flowControlLabel");

        gridLayout->addWidget(flowControlLabel, 5, 0, 1, 1);

        flowControlComboBox = new QComboBox(tabSerial);
        flowControlComboBox->setObjectName("flowControlComboBox");

        gridLayout->addWidget(flowControlComboBox, 5, 1, 1, 1);

        connectButton = new QPushButton(tabSerial);
        connectButton->setObjectName("connectButton");

        gridLayout->addWidget(connectButton, 6, 0, 1, 3);

        statusLabel = new QLabel(tabSerial);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setTabletTracking(false);

        gridLayout->addWidget(statusLabel, 7, 0, 1, 3);


        verticalLayout_Serial->addLayout(gridLayout);

        labelRx = new QLabel(tabSerial);
        labelRx->setObjectName("labelRx");

        verticalLayout_Serial->addWidget(labelRx);

        textRx = new QPlainTextEdit(tabSerial);
        textRx->setObjectName("textRx");
        textRx->setReadOnly(true);
        textRx->setMaximumBlockCount(10000);

        verticalLayout_Serial->addWidget(textRx);

        labelTx = new QLabel(tabSerial);
        labelTx->setObjectName("labelTx");

        verticalLayout_Serial->addWidget(labelTx);

        horizontalLayout_Tx = new QHBoxLayout();
        horizontalLayout_Tx->setObjectName("horizontalLayout_Tx");
        lineTx = new QLineEdit(tabSerial);
        lineTx->setObjectName("lineTx");

        horizontalLayout_Tx->addWidget(lineTx);

        btnSend = new QPushButton(tabSerial);
        btnSend->setObjectName("btnSend");

        horizontalLayout_Tx->addWidget(btnSend);


        verticalLayout_Serial->addLayout(horizontalLayout_Tx);

        tabWidget->addTab(tabSerial, QString());
        tabChart = new QWidget();
        tabChart->setObjectName("tabChart");
        chartLayout = new QVBoxLayout(tabChart);
        chartLayout->setObjectName("chartLayout");
        tabWidget->addTab(tabChart, QString());
        btnClearPlot = new QPushButton(centralWidget);
        btnClearPlot->setObjectName("btnClearPlot");
        btnClearPlot->setGeometry(QRect(30, 680, 93, 28));
        vScrollBar = new QScrollBar(centralWidget);
        vScrollBar->setObjectName("vScrollBar");
        vScrollBar->setGeometry(QRect(1070, 10, 17, 631));
        vScrollBar->setOrientation(Qt::Orientation::Vertical);
        hScrollBar = new QScrollBar(centralWidget);
        hScrollBar->setObjectName("hScrollBar");
        hScrollBar->setGeometry(QRect(20, 650, 1041, 17));
        hScrollBar->setOrientation(Qt::Orientation::Horizontal);
        btnCalOrigin = new QPushButton(centralWidget);
        btnCalOrigin->setObjectName("btnCalOrigin");
        btnCalOrigin->setGeometry(QRect(1110, 150, 101, 28));
        btnCal10_10 = new QPushButton(centralWidget);
        btnCal10_10->setObjectName("btnCal10_10");
        btnCal10_10->setGeometry(QRect(1110, 180, 101, 28));
        btnCalN10_N10 = new QPushButton(centralWidget);
        btnCalN10_N10->setObjectName("btnCalN10_N10");
        btnCalN10_N10->setGeometry(QRect(1110, 210, 101, 28));
        btnToggleCoord = new QPushButton(centralWidget);
        btnToggleCoord->setObjectName("btnToggleCoord");
        btnToggleCoord->setGeometry(QRect(1110, 240, 101, 28));
        labelDev1Coord = new QLabel(centralWidget);
        labelDev1Coord->setObjectName("labelDev1Coord");
        labelDev1Coord->setGeometry(QRect(30, 710, 201, 19));
        labelDev1Coord->setFrameShape(QFrame::Shape::Box);
        btnAddDevice = new QPushButton(centralWidget);
        btnAddDevice->setObjectName("btnAddDevice");
        btnAddDevice->setGeometry(QRect(1110, 50, 93, 28));
        btnDelDevice = new QPushButton(centralWidget);
        btnDelDevice->setObjectName("btnDelDevice");
        btnDelDevice->setGeometry(QRect(1110, 80, 93, 28));
        labelDeviceCount = new QLabel(centralWidget);
        labelDeviceCount->setObjectName("labelDeviceCount");
        labelDeviceCount->setGeometry(QRect(1100, 20, 111, 19));
        comboDeviceId = new QComboBox(centralWidget);
        comboDeviceId->setObjectName("comboDeviceId");
        comboDeviceId->setGeometry(QRect(1120, 310, 91, 25));
        lineManualX = new QLineEdit(centralWidget);
        lineManualX->setObjectName("lineManualX");
        lineManualX->setGeometry(QRect(1120, 340, 91, 25));
        lineManualY = new QLineEdit(centralWidget);
        lineManualY->setObjectName("lineManualY");
        lineManualY->setGeometry(QRect(1120, 370, 91, 25));
        btnAddPoint = new QPushButton(centralWidget);
        btnAddPoint->setObjectName("btnAddPoint");
        btnAddPoint->setGeometry(QRect(1110, 400, 101, 28));
        comboClearTarget = new QComboBox(centralWidget);
        comboClearTarget->setObjectName("comboClearTarget");
        comboClearTarget->setGeometry(QRect(1130, 430, 83, 25));
        btnClearMarkers = new QPushButton(centralWidget);
        btnClearMarkers->setObjectName("btnClearMarkers");
        btnClearMarkers->setGeometry(QRect(1120, 460, 91, 28));
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\344\270\262\345\217\243\346\225\260\346\215\256\345\217\257\350\247\206\345\214\226\344\270\212\344\275\215\346\234\272", nullptr));
        portNameLabel->setText(QCoreApplication::translate("MainWindow", "\344\270\262\345\217\243\345\220\215\347\247\260:", nullptr));
        refreshButton->setText(QCoreApplication::translate("MainWindow", "\345\210\267\346\226\260", nullptr));
        baudRateLabel->setText(QCoreApplication::translate("MainWindow", "\346\263\242\347\211\271\347\216\207:", nullptr));
        dataBitsLabel->setText(QCoreApplication::translate("MainWindow", "\346\225\260\346\215\256\344\275\215:", nullptr));
        parityLabel->setText(QCoreApplication::translate("MainWindow", "\346\240\241\351\252\214\344\275\215:", nullptr));
        stopBitsLabel->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242\344\275\215:", nullptr));
        flowControlLabel->setText(QCoreApplication::translate("MainWindow", "\346\265\201\346\216\247\345\210\266:", nullptr));
        connectButton->setText(QCoreApplication::translate("MainWindow", "\350\277\236\346\216\245", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "\344\270\262\345\217\243\346\234\252\350\277\236\346\216\245", nullptr));
        labelRx->setText(QCoreApplication::translate("MainWindow", "\346\216\245\346\224\266\345\214\272:", nullptr));
        labelTx->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201\345\214\272:", nullptr));
        btnSend->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabSerial), QCoreApplication::translate("MainWindow", "\344\270\262\345\217\243\351\205\215\347\275\256", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabChart), QCoreApplication::translate("MainWindow", "\345\235\220\346\240\207\350\275\250\350\277\271", nullptr));
        btnClearPlot->setText(QCoreApplication::translate("MainWindow", "\346\270\205\351\231\244\350\275\250\350\277\271", nullptr));
        btnCalOrigin->setText(QCoreApplication::translate("MainWindow", "\346\240\241\345\207\206\345\216\237\347\202\271", nullptr));
        btnCal10_10->setText(QCoreApplication::translate("MainWindow", "\346\240\241\345\207\206\347\202\2711", nullptr));
        btnCalN10_N10->setText(QCoreApplication::translate("MainWindow", "\346\240\241\345\207\206\347\202\2712", nullptr));
        btnToggleCoord->setText(QCoreApplication::translate("MainWindow", "\345\210\207\346\215\242\345\235\220\346\240\207\347\263\273", nullptr));
        labelDev1Coord->setText(QString());
        btnAddDevice->setText(QCoreApplication::translate("MainWindow", "\346\267\273\345\212\240\350\256\276\345\244\207", nullptr));
        btnDelDevice->setText(QCoreApplication::translate("MainWindow", "\345\207\217\345\260\221\350\256\276\345\244\207", nullptr));
        labelDeviceCount->setText(QCoreApplication::translate("MainWindow", "\350\256\276\345\244\207\346\225\260\357\274\2322 / 50", nullptr));
        btnAddPoint->setText(QCoreApplication::translate("MainWindow", "btnAddPoint", nullptr));
        btnClearMarkers->setText(QCoreApplication::translate("MainWindow", "btnClearMarkers", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
