#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QScatterSeries>
#include <QTimer>


QT_USE_NAMESPACE

    static const int MAX_DEVICE = 50;   // 上限
static const int MIN_DEVICE = 2;    // 下限

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serialPort = new QSerialPort(this);
    setupSerialPortSettings();

    /* -------------------- 图表 -------------------- */
    chart = new QChart();
    chart->setTitle("坐标轨迹");
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout->addWidget(chartView);

    axisX = new QValueAxis;
    axisX->setRange(-2'000'000, 2'000'000);
    axisX->setTitleText("X坐标（原始）");
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QValueAxis;
    axisY->setRange(-2'000'000, 2'000'000);
    axisY->setTitleText("Y坐标（原始）");
    chart->addAxis(axisY, Qt::AlignLeft);

    /* 默认两条线：设备1 + 设备2 */
    addDeviceSeries(1);
    addDeviceSeries(2);

    /* 十字参考线 */
    addReferenceLine(Qt::Horizontal);
    addReferenceLine(Qt::Vertical);

    /* 当前点游标 */
    cursorDot = new QScatterSeries();
    cursorDot->setName("当前点");
    cursorDot->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    cursorDot->setMarkerSize(10);
    cursorDot->setColor(Qt::red);
    chart->addSeries(cursorDot);
    cursorDot->attachAxis(axisX);
    cursorDot->attachAxis(axisY);

    /* -------------------- 其它初始化 -------------------- */
    m_center = QPointF(0, 0);
    m_span   = 10000;

    ui->vScrollBar->setRange(0, 1000);
    ui->hScrollBar->setRange(0, 1000);
    ui->vScrollBar->setValue(500);
    ui->hScrollBar->setValue(500);

    connect(ui->vScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onScroll);
    connect(ui->hScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onScroll);

    chartView->installEventFilter(this);

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::on_btnSend_clicked);
    connect(ui->btnClearPlot, &QPushButton::clicked, this, &MainWindow::clearPlot);

    processTimer = new QTimer(this);
    processTimer->setInterval(50);
    connect(processTimer, &QTimer::timeout, this, &MainWindow::processBufferedData);

    /* 校准相关信号槽（保持你原来的即可） */
    connect(ui->btnCalOrigin,    &QPushButton::clicked, this, &MainWindow::onCalibrateOrigin);
    connect(ui->btnCal10_10,     &QPushButton::clicked, this, &MainWindow::onCalibrate10_10);
    connect(ui->btnCalN10_N10,   &QPushButton::clicked, this, &MainWindow::onCalibrateN10_N10);
    connect(ui->btnToggleCoord,  &QPushButton::clicked, this, &MainWindow::onToggleCoordSystem);

    /* 动态设备控制 */
    connect(ui->btnAddDevice,    &QPushButton::clicked, this, &MainWindow::onAddDevice);
    connect(ui->btnDelDevice,    &QPushButton::clicked, this, &MainWindow::onDelDevice);

    refreshDeviceButtons();  // 初始可用性
    refreshDeviceCountLabel();
}

MainWindow::~MainWindow()
{
    if (serialPort->isOpen()) serialPort->close();
    delete ui;
}

/* ====================================================================== */
/*                         动态设备管理                                     */
/* ====================================================================== */
void MainWindow::addDeviceSeries(int id)
{
    if (deviceSeriesMap.contains(id)) return;

    QLineSeries *series = new QLineSeries();
    series->setName(QString("设备%1").arg(id));
    series->setColor(nextDeviceColor(id));   // ← 只加这一句
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    deviceSeriesMap.insert(id, series);
    devicePointMap.insert(id, QVector<QPointF>());
}

void MainWindow::removeDeviceSeries(int id)
{
    if (!deviceSeriesMap.contains(id)) return;
    chart->removeSeries(deviceSeriesMap.value(id));
    delete deviceSeriesMap.take(id);
    devicePointMap.remove(id);
}

int MainWindow::nextDeviceId() const
{
    int id = 1;
    while (deviceSeriesMap.contains(id)) ++id;
    return id;
}

void MainWindow::onAddDevice()
{
    if (deviceSeriesMap.size() >= MAX_DEVICE) {
        QMessageBox::information(this, "提示", QString("最多支持 %1 个设备").arg(MAX_DEVICE));
        return;
    }
    int id = nextDeviceId();
    addDeviceSeries(id);
    refreshDeviceButtons();
    refreshDeviceCountLabel();
}

void MainWindow::onDelDevice()
{
    if (deviceSeriesMap.size() <= MIN_DEVICE) {
        QMessageBox::information(this, "提示", QString("至少保留 %1 个设备").arg(MIN_DEVICE));
        return;
    }
    /* 删除编号最大的设备（也可改成弹框选择） */
    int id = deviceSeriesMap.lastKey();
    removeDeviceSeries(id);
    refreshDeviceButtons();
    refreshDeviceCountLabel();
}

void MainWindow::refreshDeviceButtons()
{
    ui->btnAddDevice->setEnabled(deviceSeriesMap.size() < MAX_DEVICE);
    ui->btnDelDevice->setEnabled(deviceSeriesMap.size() > MIN_DEVICE);
}

/* ====================================================================== */
/*                   其余函数（串口/解析/校准/坐标变换/刷新等）            */
/*         以下函数除极少量改动外，均与你原代码保持一致，直接复用          */
/* ====================================================================== */

void MainWindow::setupSerialPortSettings()
{
    ui->baudRateComboBox->addItem("9600",   QSerialPort::Baud9600);
    ui->baudRateComboBox->addItem("115200", QSerialPort::Baud115200);
    ui->baudRateComboBox->setCurrentText("115200");

    ui->dataBitsComboBox->addItem("8",  QSerialPort::Data8);
    ui->dataBitsComboBox->setCurrentText("8");

    ui->parityComboBox->addItem("无",      QSerialPort::NoParity);
    ui->parityComboBox->addItem("奇校验",  QSerialPort::OddParity);
    ui->parityComboBox->addItem("偶校验",  QSerialPort::EvenParity);
    ui->parityComboBox->setCurrentText("无");

    ui->stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    ui->stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
    ui->stopBitsComboBox->setCurrentText("1");

    ui->flowControlComboBox->addItem("无",  QSerialPort::NoFlowControl);
    ui->flowControlComboBox->setCurrentText("无");

    on_refreshButton_clicked();
}

void MainWindow::on_refreshButton_clicked()
{
    ui->portNameComboBox->clear();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ui->portNameComboBox->addItem(info.portName());
}

void MainWindow::on_connectButton_clicked()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        ui->connectButton->setText("连接");
        ui->statusLabel->setText("串口已断开");
    } else {
        serialPort->setPortName(ui->portNameComboBox->currentText());
        serialPort->setBaudRate(ui->baudRateComboBox->currentData().toInt());
        serialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->dataBitsComboBox->currentData().toInt()));
        serialPort->setParity(static_cast<QSerialPort::Parity>(ui->parityComboBox->currentData().toInt()));
        serialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBitsComboBox->currentData().toInt()));
        serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(ui->flowControlComboBox->currentData().toInt()));

        if (serialPort->open(QIODevice::ReadOnly)) {
            ui->connectButton->setText("断开");
            ui->statusLabel->setText("串口已连接");
        } else {
            QMessageBox::critical(this, "错误", "无法打开串口: " + serialPort->errorString());
            ui->statusLabel->setText("串口连接失败");
        }
    }
}

void MainWindow::readSerialData()
{
    buffer.append(serialPort->readAll());
    if (!buffer.isEmpty() && !processTimer->isActive())
        processTimer->start();
}

void MainWindow::on_btnSend_clicked()
{
    if (!serialPort->isOpen()) {
        QMessageBox::warning(this, "提示", "请先打开串口");
        return;
    }
    QByteArray data = ui->lineTx->text().toUtf8();
    serialPort->write(data);
    ui->textRx->appendPlainText(QString("TX[%1] %2")
                                    .arg(data.size())
                                    .arg(QString::fromUtf8(data)));
    ui->lineTx->clear();
}

void MainWindow::parseDataPacket(const QByteArray &packet)
{
    quint8 deviceId = static_cast<quint8>(packet.at(1));

    quint32 x = ((static_cast<quint8>(packet.at(2)) << 16) |
                 (static_cast<quint8>(packet.at(3)) << 8)  |
                 static_cast<quint8>(packet.at(4)));

    quint32 y = ((static_cast<quint8>(packet.at(5)) << 16) |
                 (static_cast<quint8>(packet.at(6)) << 8)  |
                 static_cast<quint8>(packet.at(7)));

    if (devicePointMap.contains(deviceId))
        devicePointMap[deviceId].append(QPointF(x, y));

    updatePlot();
    QPointF raw(x, y);
    QPointF mapped = mapPoint(raw);
    cursorDot->clear();
    cursorDot->append(mapped);
    ui->statusLabel->setText(QString("收到数据 - 设备: 0x%1, X: %2, Y: %3")
                                 .arg(deviceId, 2, 16, QChar('0'))
                                 .arg(x).arg(y));
    if (deviceId == 0x01) refreshDev1Label();
}

void MainWindow::processBufferedData()
{
    const int chunkSize = 1000;
    QByteArray chunk = buffer.left(chunkSize);
    buffer.remove(0, chunk.size());

    QString hex = chunk.toHex(' ').toUpper();
    QString ascii;
    for (char c : chunk) ascii.append((c >= 32 && c < 127) ? c : '.');
    ui->textRx->appendPlainText(
        QString("RX[%1] %2  |  %3")
            .arg(chunk.size(), 3, 10, QChar(' '))
            .arg(hex, -30).arg(ascii));

    if (ui->textRx->blockCount() > 1000) ui->textRx->setMaximumBlockCount(1000);

    int head = chunk.indexOf(0xFA);
    while (head != -1 && chunk.size() > head + 8) {
        if ((quint8)chunk.at(head + 8) == 0xAF) {
            parseDataPacket(chunk.mid(head, 9));
            chunk.remove(0, head + 9);
        } else {
            chunk.remove(0, head + 1);
        }
        head = chunk.indexOf(0xFA);
    }
    if (!buffer.isEmpty()) processTimer->start();
    else                   processTimer->stop();
}

void MainWindow::updatePlot()
{
    static int frameSkip = 0;
    if (++frameSkip % 3) return;

    for (auto it = deviceSeriesMap.begin(); it != deviceSeriesMap.end(); ++it) {
        int id = it.key();
        QLineSeries *line = it.value();
        const QVector<QPointF> &raw = devicePointMap[id];
        QVector<QPointF> pts;
        for (const QPointF &p : raw) pts.append(mapPoint(p));
        line->replace(pts);
    }

    cursorDot->clear();
    QPointF latest;
    for (const QVector<QPointF> &v : devicePointMap) {
        if (!v.isEmpty() && (latest.isNull() || v.last().x() > latest.x()))
            latest = v.last();
    }
    if (!latest.isNull()) cursorDot->append(mapPoint(latest));

    updateAxes();
}

void MainWindow::clearPlot()
{
    for (auto &v : devicePointMap) v.clear();
    for (QLineSeries *s : deviceSeriesMap) s->clear();
    cursorDot->clear();
    updateAxes();
    ui->statusLabel->setText("轨迹已清空");
}

void MainWindow::addReferenceLine(Qt::Orientation o)
{
    QLineSeries *line = new QLineSeries;
    if (o == Qt::Horizontal) {
        line->setName("Y=0");
        line->append(-2'000'000, 0);
        line->append( 2'000'000, 0);
    } else {
        line->setName("X=0");
        line->append(0, -2'000'000);
        line->append(0,  2'000'000);
    }
    line->setColor(Qt::black);
    line->setPen(QPen(Qt::black, 4));
    chart->addSeries(line);
    line->attachAxis(axisX);
    line->attachAxis(axisY);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == chartView && event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent*>(event);
        qreal factor = (we->angleDelta().y() > 0) ? 0.8 : 1.25;
        m_span *= factor;
        m_span = qBound(10.0, m_span, 2e4);
        updateAxes();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onScroll()
{
    qreal dx = (ui->hScrollBar->value() - 500) / 500.0 * 100000.0;
    qreal dy = (500 - ui->vScrollBar->value()) / 500.0 * 100000.0;
    m_center = QPointF(dx, dy);
    updateAxes();
}

void MainWindow::updateAxes()
{
    double half = m_span / 2.0;
    axisX->setRange(m_center.x() - half, m_center.x() + half);
    axisY->setRange(m_center.y() - half, m_center.y() + half);
}

/* ========================= 下面校准相关函数保持原样 ========================= */
//校准坐标
void MainWindow::onCalibrateOrigin()
{
    if (device1Points.isEmpty()) {
        QMessageBox::information(this, "提示", "设备1尚未收到任何坐标，无法校准原点！");
        return;
    }
    origin0 = device1Points.last();          // 取最新点
    ui->statusLabel->setText(
        QString("原点已校准 → 设备1: (%1, %2)").arg(origin0.x()).arg(origin0.y()));

    calibRaw.append(device1Points.last());
    if (calibRaw.size() == 3) calibReady = true;

    QMessageBox::information(this, "提示", "校准完成!");


}

void MainWindow::onCalibrate10_10()
{
    if (device1Points.isEmpty()) {
        QMessageBox::information(this, "提示", "设备1尚未收到任何坐标，无法校准！");
        return;
    }

    QPointF raw = device1Points.last();
    origin0 = raw - QPointF(10, 10);   // 把当前实际坐标减去偏移量，得到“设定原点”
    ui->statusLabel->setText(
        QString("已校准(10,10) → 设备1原点: (%1, %2)").arg(origin0.x()).arg(origin0.y()));


    calibRaw.append(device1Points.last());
    if (calibRaw.size() == 3) calibReady = true;


    QMessageBox::information(this, "提示", "校准完成!");
}

void MainWindow::onCalibrateN10_N10()
{
    if (device1Points.isEmpty()) {
        QMessageBox::information(this, "提示", "设备1尚未收到任何坐标，无法校准！");
        return;
    }

    QPointF raw = device1Points.last();
    origin0 = raw - QPointF(-10, -10);   // 把当前实际坐标减去偏移量，得到“设定原点”
    ui->statusLabel->setText(
        QString("已校准(-10,-10) → 设备1原点: (%1, %2)").arg(origin0.x()).arg(origin0.y()));


    calibRaw.append(device1Points.last());
    if (calibRaw.size() == 3) calibReady = true;

    QMessageBox::information(this, "提示", "校准完成!");
}

void MainWindow::onToggleCoordSystem()
{
    if (!calibReady) {
        QMessageBox::information(this, "提示", "请先完成 3 点校准（设备1）");
        return;
    }

    useCalibrated = !useCalibrated;

    if (useCalibrated) {
        QPointF src[3] = { calibRaw[0], calibRaw[1], calibRaw[2] };
        QPointF dst[3] = { {0,0}, {5,5}, {-5,5} };
        if (!solveAffine(src, dst, affineM)) {
            QMessageBox::warning(this, "错误", "校准点共线，无法求仿射变换");
            useCalibrated = false;
            return;
        }
        ui->btnToggleCoord->setText("切换回原始坐标");

        // ✅ 更新坐标轴标题
        axisX->setTitleText("X坐标（变换后）");
        axisY->setTitleText("Y坐标（变换后）");

    } else {
        ui->btnToggleCoord->setText("切换坐标系");

        // ✅ 恢复原始标题
        axisX->setTitleText("X坐标（原始）");
        axisY->setTitleText("Y坐标（原始）");
    }

    updatePlot();   // 立即重绘全部设备
    refreshDev1Label();
    // ✅ 不调用 updateAxes()，保持当前视图
    ui->statusLabel->setText(useCalibrated ? "当前为变换坐标系" : "当前为原始坐标系");
}
bool MainWindow::solveAffine(const QPointF src[3], const QPointF dst[3], double M[6])
{
    // 6x6 线性方程组：高斯消元
    double A[6][7] = {0};
    for (int i = 0; i < 3; ++i) {
        A[i][0] = src[i].x(); A[i][1] = src[i].y(); A[i][2] = 1;     // x 方程
        A[i][6] = dst[i].x();
        A[i+3][3] = src[i].x(); A[i+3][4] = src[i].y(); A[i+3][5] = 1; // y 方程
        A[i+3][6] = dst[i].y();
    }
    for (int col = 0; col < 6; ++col) {
        int pivot = col;
        for (int r = col + 1; r < 6; ++r)
            if (qAbs(A[r][col]) > qAbs(A[pivot][col])) pivot = r;
        if (qAbs(A[pivot][col]) < 1e-10) return false;
        for (int c = 0; c < 7; ++c) qSwap(A[col][c], A[pivot][c]);
        for (int r = 0; r < 6; ++r) {
            if (r == col) continue;
            double f = A[r][col] / A[col][col];
            for (int c = col; c < 7; ++c) A[r][c] -= f * A[col][c];
        }
    }
    for (int i = 0; i < 6; ++i) M[i] = A[i][6] / A[i][i];
    return true;
}

inline QPointF MainWindow::mapPoint(const QPointF &p) const
{
    if (!useCalibrated) return p;
    return QPointF(affineM[0]*p.x() + affineM[1]*p.y() + affineM[2],
                   affineM[3]*p.x() + affineM[4]*p.y() + affineM[5]);
}
// 放在文件末尾即可
void MainWindow::refreshDev1Label()
{
    if (device1Points.isEmpty()) {
        ui->labelDev1Coord->setText("设备1：暂无数据");
        return;
    }

    QPointF raw  = device1Points.last();
    QPointF show = mapPoint(raw);   // 自动根据 useCalibrated 切换坐标系

    QString text = QString("设备1：X = %1  |  Y = %2")
                       .arg(show.x(), 0, 'f', 2)
                       .arg(show.y(), 0, 'f', 2);

    ui->labelDev1Coord->setText(text);
}
void MainWindow::refreshDeviceCountLabel()
{
    int current = deviceSeriesMap.size();
    int maxDev  = MAX_DEVICE;
    ui->labelDeviceCount->setText(
        QString("设备数：%1 / %2").arg(current).arg(maxDev));
}

QColor MainWindow::nextDeviceColor(int id) const
{
    /* Qt 自带 20 种标准色，先用完再说 */
    static const QList<QColor> base = {
        Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta,
        Qt::yellow, Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkCyan,
        Qt::darkMagenta, Qt::darkYellow, Qt::gray, Qt::darkGray, Qt::lightGray,
        Qt::black, Qt::white, Qt::transparent, QColor(255,165,0), QColor(128,0,128)
    };

    if (id <= base.size())
        return base[id - 1];

    /* 20 个以后用 HSV 均匀取色，饱和度亮度固定，只转色相 */
    const qreal golden = 0.618033988749895;   // 黄金角
    qreal hue = std::fmod(id * golden * 360.0, 360.0);
    return QColor::fromHsvF(hue, 0.95, 0.95);
}
