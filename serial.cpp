#include "serial.h"
#include <QThread>

#define WAIT_SENT_MS 1000

Serial::Serial(int timerFreq, QObject* parent)
    : QObject { parent }
    , mPort(new QSerialPort)
    , mTimerFreq(timerFreq)
    , mTimer { new QTimer(this) }
{
    connect(mPort, &QSerialPort::errorOccurred,
        this, &Serial::onError);
    connect(mPort, &QSerialPort::bytesWritten,
        this, [this](qint64 bytes) {
            Q_UNUSED(bytes);
            if (mAutoBreak) {
                QThread::msleep(15);
                mPort->setBreakEnabled(true);
                QThread::msleep(1);
                mPort->setBreakEnabled(false);
            }
        });

    setAutoBreak(false);

    if (0 < mTimerFreq) {
        connect(mTimer, &QTimer::timeout,
            this, &Serial::onReadyRead);
        mTimer->start(1000 / mTimerFreq);
    } else {
        connect(mPort, &QSerialPort::readyRead,
            this, &Serial::onReadyRead);
    }
}

Serial::~Serial()
{
    delete mTimer;
    delete mPort;
}

/**
 * @brief Send automatic break signal
 *
 * @param state Break state
 */
void Serial::setAutoBreak(bool state)
{
    mAutoBreak = state;
}

/**
 * @brief Toggle connection state
 *
 * @param port      Reference to port value
 * @param baud      Baudrate value
 */
void Serial::toggle(const QString& port, int baud)
{
    isConnected() ? disconnect() : conect(port, baud);
}

/**
 * @brief Check is serial transport connection state
 *
 * @return true if connected, otherwise not.
 */
bool Serial::isConnected() const
{
    return (mPort->isOpen());
}

/**
 * @brief Get available serial ports
 *
 * @return List of serial ports
 */
QList<QSerialPortInfo> Serial::getPorts() const
{
    return (QSerialPortInfo::availablePorts());
}

/**
 * @brief Connect to serial transport
 *
 * @param port      Port name
 * @param baud      Baudrate value
 * @return OK status
 */
bool Serial::conect(const QString& port, int baud)
{
    QString msg;

    /* validate port */
    if (not isValidPort(port)) {
        msg = tr("Serial: Invalid port %1").arg(port);
        emit errorOccured(msg);
        return (false);
    }

    /* only connect when disconnected */
    if (not isConnected()) {

        /* open serial connection */
        mPort->setPortName(port);
        mPort->open(QIODevice::ReadWrite);
        if (not isConnected()) {
            return (false);
        }

        /* configure serial parameters */
        mPort->setBaudRate(baud);
        mPort->setDataBits(QSerialPort::Data8);
        mPort->setParity(QSerialPort::NoParity);
        mPort->setStopBits(QSerialPort::OneStop);
        mPort->setFlowControl(QSerialPort::NoFlowControl);
    }

    emit statusChanged(isConnected());
    return (true);
}

/**
 * @brief Disconnect serial transport
 *
 * @return OK status
 */
bool Serial::disconnect()
{
    /* close if still open */
    if (mPort->isOpen()) {
        mPort->flush();
        mPort->close();
    }

    emit statusChanged(isConnected());
    return (true);
}

/**
 * @brief Write into serial transport
 *
 * @param packet Reference to byte array data to be sent
 * @return -1 in case of error, otherwise amount of transfered bytes
 */
int Serial::write(const QByteArray& packet)
{
    int rc;

    rc = -1;

    if (isConnected()) {
        rc = mPort->write(packet);
    }

    return (rc);
}

/**
 * @brief Validate serial port
 *
 * @param port  Port name
 * @return Valid status
 */
bool Serial::isValidPort(const QString& port) const
{
    /* walk through available ports */
    for (const QSerialPortInfo& ser : getPorts()) {
        if (ser.systemLocation() == port) {
            return (true);
        }
    }
    return (false);
}

/**
 * @brief Get current serial status (text)
 *
 * @return Text of serial status
 */
QString Serial::getStatus() const
{
    QString msg;

    if (isConnected()) {
        msg = tr("Serial: Connected to %1").arg(mPort->portName());
    } else {
        msg = tr("Serial: Not connected");
    }

    return (msg);
}

/**
 * @brief Listener for data ready event
 */
void Serial::onReadyRead()
{
    if (0 < mPort->bytesAvailable()) {
        emit packetReady(mPort->readAll());
    } else {
        emit packetEmpty();
    }
}

/**
 * @brief Listener for error event
 *
 * @param error Serial error status
 */
void Serial::onError(QSerialPort::SerialPortError error)
{
    QString msg;

    if (QSerialPort::NoError != error) {
        msg = mPort->errorString();
        emit errorOccured(msg);
        disconnect();
    }
}
