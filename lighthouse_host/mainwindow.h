#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QVector>
#include <QPointF>
#include <QtCharts/QScatterSeries>




QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void on_tabWidget_currentChanged(int index);
    void on_btnSend_clicked();
    void processBufferedData();

    void onZoom(qreal factor);      // 滚轮缩放
    void onScroll();                // 滑条移动
    void updateAxes();              // 统一刷新轴范围


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

};
#endif // MAINWINDOW_H
