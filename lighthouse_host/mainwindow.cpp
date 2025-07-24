#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QTimer>
#include <QtCharts/QLineSeries>

//QT_CHARTS_USE_NAMESPACE
QT_USE_NAMESPACE
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serialPort = new QSerialPort(this);
    setupSerialPortSettings();

    // 设置图表
    QChart *chart = new QChart();
    chart->setTitle("坐标轨迹");
    chart->createDefaultAxes();

    // 四象限对称范围，原点居中
    axisX = new QValueAxis;
    axisX->setRange(-2'000'000, 2'000'000);
    axisX->setTitleText("X坐标（原始）");
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QValueAxis;
    axisY->setRange(-2'000'000, 2'000'000);
    axisY->setTitleText("Y坐标（原始）");
    chart->addAxis(axisY, Qt::AlignLeft);

    // 创建设备1的轨迹系列
    QLineSeries *series1 = new QLineSeries();
    series1->setName("设备1");
    chart->addSeries(series1);
    series1->attachAxis(axisX);
    series1->attachAxis(axisY);

    // 创建设备2的轨迹系列
    QLineSeries *series2 = new QLineSeries();
    series2->setName("设备2");
    chart->addSeries(series2);
    series2->attachAxis(axisX);
    series2->attachAxis(axisY);

    // X=0 垂直线
    QLineSeries *lineX0 = new QLineSeries;
    lineX0->setName("X=0");
    lineX0->append(0, -2'000'000);
    lineX0->append(0,  2'000'000);
    lineX0->setColor(Qt::black);
    lineX0->setPen(QPen(Qt::black, 4));
    chart->addSeries(lineX0);
    lineX0->attachAxis(axisX);
    lineX0->attachAxis(axisY);

    // Y=0 水平线
    QLineSeries *lineY0 = new QLineSeries;
    lineY0->setName("Y=0");
    lineY0->append(-2'000'000, 0);
    lineY0->append( 2'000'000, 0);
    lineY0->setColor(Qt::black);
    lineY0->setPen(QPen(Qt::black, 4));
    chart->addSeries(lineY0);
    lineY0->attachAxis(axisX);
    lineY0->attachAxis(axisY);

    cursorDot = new QScatterSeries();
    cursorDot->setName("当前点");
    cursorDot->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    cursorDot->setMarkerSize(10);          // 直径 > 轨迹线宽即可
    cursorDot->setColor(Qt::red);
    cursorDot->setBorderColor(Qt::red);
    chart->addSeries(cursorDot);
    cursorDot->attachAxis(axisX);
    cursorDot->attachAxis(axisY);






    // 设置图表视图
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout->addWidget(chartView);

    //清除轨迹摁键
    connect(ui->btnClearPlot, &QPushButton::clicked, this, &MainWindow::clearPlot);

    // 滑条范围 0-1000，初始在中间
    ui->vScrollBar->setRange(0, 1000);
    ui->hScrollBar->setRange(0, 1000);
    ui->vScrollBar->setValue(500);
    ui->hScrollBar->setValue(500);

    connect(ui->vScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onScroll);
    connect(ui->hScrollBar, &QScrollBar::valueChanged, this, &MainWindow::onScroll);

    // 捕获滚轮事件，后面会写事件过滤器
    ui->chartLayout->itemAt(0)->widget()->installEventFilter(this);

    m_center = QPointF(0, 0);
    m_span = 10000; // 初始跨度，可根据需求调整


    // 连接串口数据接收信号
    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

    //追加连接
    connect(ui->btnSend, &QPushButton::clicked,
            this,        &MainWindow::on_btnSend_clicked);


    processTimer = new QTimer(this);
    processTimer->setInterval(50);
    connect(processTimer, &QTimer::timeout,
            this,        &MainWindow::processBufferedData);



    //校准功能

    connect(ui->btnCalOrigin, &QPushButton::clicked,
            this,             &MainWindow::onCalibrateOrigin);

    connect(ui->btnCal10_10,    &QPushButton::clicked,
            this,               &MainWindow::onCalibrate10_10);
    connect(ui->btnCalN10_N10,  &QPushButton::clicked,
            this,               &MainWindow::onCalibrateN10_N10);
    //ui->btnToggleCoord->setText("切换坐标系");
    connect(ui->btnToggleCoord,  &QPushButton::clicked,
            this,               &MainWindow::onToggleCoordSystem);

}

MainWindow::~MainWindow()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    delete ui;
}

void MainWindow::setupSerialPortSettings()
{
    // 填充波特率选项
    ui->baudRateComboBox->addItem("9600", QSerialPort::Baud9600);
    ui->baudRateComboBox->addItem("115200", QSerialPort::Baud115200);
    ui->baudRateComboBox->setCurrentText("115200");

    // 填充数据位选项
    ui->dataBitsComboBox->addItem("8", QSerialPort::Data8);
    ui->dataBitsComboBox->setCurrentText("8");

    // 填充校验位选项
    ui->parityComboBox->addItem("无", QSerialPort::NoParity);
    ui->parityComboBox->addItem("奇校验", QSerialPort::OddParity);
    ui->parityComboBox->addItem("偶校验", QSerialPort::EvenParity);
    ui->parityComboBox->setCurrentText("无");

    // 填充停止位选项
    ui->stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    ui->stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
    ui->stopBitsComboBox->setCurrentText("1");

    // 填充流控制选项
    ui->flowControlComboBox->addItem("无", QSerialPort::NoFlowControl);
    ui->flowControlComboBox->addItem("硬件", QSerialPort::HardwareControl);
    ui->flowControlComboBox->addItem("软件", QSerialPort::SoftwareControl);
    ui->flowControlComboBox->setCurrentText("无");

    // 刷新串口列表
    on_refreshButton_clicked();
}

void MainWindow::on_refreshButton_clicked()
{
    ui->portNameComboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->portNameComboBox->addItem(info.portName());
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        ui->connectButton->setText("连接");
        ui->statusLabel->setText("串口已断开");
    } else {
        // 设置串口参数
        serialPort->setPortName(ui->portNameComboBox->currentText());
        serialPort->setBaudRate(ui->baudRateComboBox->currentData().toInt());
        serialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->dataBitsComboBox->currentData().toInt()));
        serialPort->setParity(static_cast<QSerialPort::Parity>(ui->parityComboBox->currentData().toInt()));
        serialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBitsComboBox->currentData().toInt()));
        serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(ui->flowControlComboBox->currentData().toInt()));

        // 打开串口
        if (serialPort->open(QIODevice::ReadOnly)) {
            ui->connectButton->setText("断开");
            ui->statusLabel->setText("串口已连接");
        } else {
            QMessageBox::critical(this, "错误", "无法打开串口: " + serialPort->errorString());
            ui->statusLabel->setText("串口连接失败");
        }
    }
}

/* ====================================================================== */
/*                          接收 + 发送                                    */
/* ====================================================================== */

void MainWindow::readSerialData()
{
    buffer.append(serialPort->readAll());          // 只缓存，不处理

    if (!buffer.isEmpty() && !processTimer->isActive())
        processTimer->start();                     // 启动定时器
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
    // 解析设备ID
    quint8 deviceId = static_cast<quint8>(packet.at(1));

    // 解析X坐标 (3字节)
    quint32 x = ((static_cast<quint8>(packet.at(2)) << 16) |
                 (static_cast<quint8>(packet.at(3)) << 8) |
                 static_cast<quint8>(packet.at(4)));

    // 解析Y坐标 (3字节)
    quint32 y = ((static_cast<quint8>(packet.at(5)) << 16) |
                 (static_cast<quint8>(packet.at(6)) << 8) |
                 static_cast<quint8>(packet.at(7)));

    // 添加到对应设备的点集
    if (deviceId == 0x01) {
        device1Points.append(QPointF(x, y));
    } else if (deviceId == 0x02) {
        device2Points.append(QPointF(x, y));
    }

    // 更新图表
    updatePlot();

    QPointF raw(x, y);
    QPointF mapped = mapPoint(raw);   // 始终用变换后的坐标
    cursorDot->clear();
    cursorDot->append(mapped);

    // 在状态栏显示最新数据
    ui->statusLabel->setText(QString("收到数据 - 设备: 0x%1, X: %2, Y: %3").arg(deviceId, 2, 16, QChar('0')).arg(x).arg(y));
    refreshDev1Label();
}

void MainWindow::processBufferedData()
{
    /* ---------- 1. 每次最多处理 1000 字节，防止卡顿 ---------- */
    const int chunkSize = 1000;
    QByteArray chunk = buffer.left(chunkSize);
    buffer.remove(0, chunk.size());

    /* ---------- 2. 十六进制 + ASCII 显示（可选行数限制） ---------- */
    QString hex = chunk.toHex(' ').toUpper();
    QString ascii;
    for (char c : chunk)
        ascii.append((c >= 32 && c < 127) ? c : '.');

    ui->textRx->appendPlainText(
        QString("RX[%1] %2  |  %3")
            .arg(chunk.size(), 3, 10, QChar(' '))
            .arg(hex, -30)
            .arg(ascii));

    /* ---------- 3. 限制最大行数，防止内存爆炸 ---------- */
    const int maxLines = 1000;
    if (ui->textRx->blockCount() > maxLines)
        ui->textRx->setMaximumBlockCount(maxLines);

    /* ---------- 4. 解析数据包 ---------- */
    int head = chunk.indexOf(0xFA);
    while (head != -1 && chunk.size() > head + 8) {
        if ((quint8)chunk.at(head + 8) == 0xAF) {
            QByteArray packet = chunk.mid(head, 9);
            parseDataPacket(packet);               // 你的原函数
            chunk.remove(0, head + 9);
        } else {
            chunk.remove(0, head + 1);
        }
        head = chunk.indexOf(0xFA);
    }

    /* ---------- 5. 如果还有数据，继续定时；否则停止 ---------- */
    if (!buffer.isEmpty())
        processTimer->start();
    else
        processTimer->stop();
}




void MainWindow::updatePlot()
{
    /* 1. 简单的帧率控制，避免高频重绘 */
    static int frameSkip = 0;
    if (++frameSkip % 3)   // 每 3 次调用才刷新一次
        return;

    /* 2. 取得图表对象 */
    QChartView *chartView = qobject_cast<QChartView *>(ui->chartLayout->itemAt(0)->widget());
    if (!chartView) return;
    QChart *chart = chartView->chart();
    if (!chart) return;

    /* 3. 遍历图表里的所有系列 */
    for (QAbstractSeries *baseSeries : chart->series())
    {
        /* 3-1 只处理折线系列，其余（十字线、游标点）跳过 */
        QLineSeries *line = qobject_cast<QLineSeries *>(baseSeries);
        if (!line) continue;   // 不是折线就跳过

        /* 3-2 根据系列名字拿到原始数据容器指针 */
        const QVector<QPointF> *rawPoints = nullptr;
        if (line->name() == "设备1")
            rawPoints = &device1Points;
        else if (line->name() == "设备2")
            rawPoints = &device2Points;
        // 如果以后还有设备3、4……在这里继续 else if
        else
            continue;          // 未知系列也跳过

        /* 3-3 清空旧点，重新填充（统一经过坐标变换） */
        line->clear();
        QVector<QPointF> pts = line->points();
        for (const QPointF &rawPt : *rawPoints)
        {
            QPointF mappedPt = mapPoint(rawPt);
            pts.append(mappedPt);
        }
        line->replace(pts);
    }

    /* 4. 同步游标点（红色圆点）也要映射 */
    cursorDot->clear();
    QPointF latestRaw;

    // 谁最后收到数据就显示谁的红点（你也可以固定为设备1）
    if (!device1Points.isEmpty())
        latestRaw = device1Points.last();
    if (!device2Points.isEmpty() &&
        (device1Points.isEmpty() ||
         device2Points.last().x() > device1Points.last().x()))
        latestRaw = device2Points.last();

    if (!latestRaw.isNull())
    {
        QPointF latestMapped = mapPoint(latestRaw);
        cursorDot->append(latestMapped);
    }

    updateAxes();
}





void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1 && (!device1Points.isEmpty() || !device2Points.isEmpty())) {
        updatePlot();
    }
}
void MainWindow::clearPlot()
{
    // 清空本地数据缓存
    device1Points.clear();
    device2Points.clear();

    // 清空图表中的轨迹线
    QChartView *chartView = qobject_cast<QChartView *>(ui->chartLayout->itemAt(0)->widget());
    if (!chartView) return;

    QChart *chart = chartView->chart();
    if (!chart) return;

    for (QAbstractSeries *s : chart->series()) {
        QLineSeries *series = qobject_cast<QLineSeries *>(s);
        if (!series) continue;

        // 只清空轨迹线，保留参考线
        if (series->name() == "设备1" || series->name() == "设备2") {
            series->clear();
        }
    }

    // 可选：重置坐标轴范围
    QList<QAbstractAxis *> axesX = chart->axes(Qt::Horizontal);
    QList<QAbstractAxis *> axesY = chart->axes(Qt::Vertical);
    if (!axesX.isEmpty() && !axesY.isEmpty()) {
        QValueAxis *axisX = qobject_cast<QValueAxis *>(axesX.first());
        QValueAxis *axisY = qobject_cast<QValueAxis *>(axesY.first());
        if (axisX && axisY) {
            axisX->setRange(-10000, 10000);
            axisY->setRange(-10000, 10000);
        }
    }

    ui->statusLabel->setText("轨迹已清空");
    updateAxes();
}

//事件过滤器
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->chartLayout->itemAt(0)->widget() && event->type() == QEvent::Wheel)
    {
        QWheelEvent *we = static_cast<QWheelEvent*>(event);
        qreal factor = (we->angleDelta().y() > 0) ? 0.8 : 1.25;
        onZoom(factor);
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}
//缩放槽函数
void MainWindow::onZoom(qreal factor)
{
    m_span *= factor;
    m_span = qBound(10.0, m_span, 2e4);   // 限制最小/最大跨度
    updateAxes();
}

//滑条槽函数
void MainWindow::onScroll()
{
    // 把滑条 0-1000 线性映射到 -100000 ~ +100000
    qreal dx = (ui->hScrollBar->value() - 500) / 500.0 * 100000.0;
    qreal dy = (500 - ui->vScrollBar->value()) / 500.0 * 100000.0;
    m_center = QPointF(dx, dy);
    updateAxes();
}

//统一刷新轴范围（updateAxes）
void MainWindow::updateAxes()
{
    QChart *c = qobject_cast<QChartView*>(ui->chartLayout->itemAt(0)->widget())->chart();
    if (!c) return;

    // 以 m_center 为中心，m_span 为跨度，手动设置范围
    double half = m_span / 2.0;
    double xMin = m_center.x() - half;
    double xMax = m_center.x() + half;
    double yMin = m_center.y() - half;
    double yMax = m_center.y() + half;

    QValueAxis *ax = qobject_cast<QValueAxis*>(c->axes(Qt::Horizontal).first());
    QValueAxis *ay = qobject_cast<QValueAxis*>(c->axes(Qt::Vertical).first());
    if (ax) ax->setRange(xMin, xMax);
    if (ay) ay->setRange(yMin, yMax);
}

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
