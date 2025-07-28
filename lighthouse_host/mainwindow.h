#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include <QPointF>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct DeviceStatus {
    bool online = false;
    qint64 lastUpdateTime = 0;
    int packetCount = 0;
    double dataRateHz = 0.0;
};



class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_connectButton_clicked();
    void on_refreshButton_clicked();
    void readSerialData();
    void on_btnSend_clicked();
    void processBufferedData();


    void onScroll();                // 滑条移动
    void updateAxes();              // 统一刷新轴范围
    void onCalibrateOrigin(); // 校准原点
    void onCalibrate10_10();      // 校准(10,10)
    void onCalibrateN10_N10();    // 校准(-10,-10)
    void onToggleCoordSystem();
    void addDeviceSeries(int id);
    void addReferenceLine(Qt::Orientation o);
    void refreshDeviceButtons();
    void onAddDevice();
    void onDelDevice();
    void removeDeviceSeries(int id);
    int nextDeviceId() const;
    void refreshDeviceCountLabel();
    void onAddManualPoint();
    void addShapeMarker(int deviceId, const QPointF &rawPoint);
    void on_btnClearMarkers_clicked();
    void updateDeviceStatus();
    void updateDeviceStatusUI(int id, const DeviceStatus &status);



private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
    QVector<QPointF> device1Points;
    QVector<QPointF> device2Points;
    QByteArray buffer;
    QTimer *processTimer;
    void setupSerialPortSettings();
    void parseDataPacket(const QByteArray &packet);
    void updatePlot();
    void clearPlot();
    QScatterSeries *cursorDot = nullptr;

    qreal m_span   = 200000.0;      // 当前坐标跨度（±span/2）
    QPointF m_center{0.0, 0.0};     // 当前几何中心


     QPointF origin0; //校准原点
     QPointF origin10_10; //校准(10,10)
     QPointF originN10_N10; //校准(-10,-10)

     QList<QPointF> calibRaw;          // 原始 3 点（只用设备1）
     double affineM[6] = {1,0,0, 0,1,0}; // x'=M0*x+M1*y+M2, y'=M3*x+M4*y+M5
     bool useCalibrated = false;       // 当前是否启用校准坐标
     bool calibReady    = false;       // 3 点是否录完

     QPointF mapPoint(const QPointF &p) const;   // 把原始坐标→校准坐标
     bool solveAffine(const QPointF src[3], const QPointF dst[3], double M[6]);

     void refreshDev1Label();
     QValueAxis *axisX = nullptr;
     QValueAxis *axisY = nullptr;

     QChart *chart;
     QChartView *chartView;
     QMap<int, QLineSeries*> deviceSeriesMap;
     QMap<int, QVector<QPointF>> devicePointMap;

     static const QVector<QColor> colorPool;
     QSet<QColor> usedColors;
     QColor pickNextColor();
     QColor nextDeviceColor(int id) const;

     QPointF mapToChart(const QPoint &pos) const;   // < 把窗口坐标 → 图表逻辑坐标
     QList<QScatterSeries*> manualMarkers;

     QMap<int, DeviceStatus> deviceStatusMap;
     QTimer *statusUpdateTimer;




};
#endif // MAINWINDOW_H
