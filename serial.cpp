#include "serial.h"

Serial::Serial(QObject* parent)
    : QObject { parent }
{
    mTransport = new QSerialPort(this);

    connect(mTransport, &QSerialPort::readyRead,
        this, &Serial::onReadyRead);
    connect(mTransport, &QSerialPort::errorOccurred,
        this, &Serial::onError);
}

/**
 * @brief Toggle connection state
 *
 * @param port      Reference to port value
 * @param baudrate  Baudrate value
 */
void Serial::toggleConnection(const QString& port, int baudrate)
{
    if (!isConnected()) {
        connectSerial(port, baudrate);
    } else {
        disconnect();
    }
}

/**
 * @brief Connect to serial transport
 *
 * @param port      Reference to port value
 * @param baudrate  Baudrate value
 * @return true in case of success, otherwise error
 */
bool Serial::connectSerial(const QString& port, int baudrate)
{
    QString msg;

    /* validate port */
    if (!isValidPort(port)) {
        msg = tr("Invalid serial port %1").arg(port);
        emit errorOccured(msg);
        return false;
    }

    /* only connect when disconnected */
    if (!isConnected()) {

        /* open serial connection */
        mTransport->setPortName(port);
        mTransport->open(QIODevice::ReadWrite);
        if (!isConnected()) {
            return false;
        }

        /* configure serial parameters */
        mTransport->setBaudRate(baudrate);
        mTransport->setDataBits(QSerialPort::Data8);
        mTransport->setParity(QSerialPort::NoParity);
        mTransport->setStopBits(QSerialPort::OneStop);
        mTransport->setFlowControl(QSerialPort::NoFlowControl);
    }

    emit statusChanged(isConnected());
    return true;
}

/**
 * @brief Disconnect serial transport
 */
void Serial::disconnect()
{
    /* close if still open */
    if (mTransport->isOpen()) {
        mTransport->flush();
        mTransport->close();
    }

    emit statusChanged(isConnected());
}

/**
 * @brief Check is serial transport connection state
 *
 * @return true if connected, otherwise not.
 */
bool Serial::isConnected()
{
    return mTransport->isOpen();
}

/**
 * @brief Validate serial port
 *
 * @param port      Reference to port value
 * @return true in case of valid, otherwise not
 */
bool Serial::isValidPort(const QString& port)
{
    /* walk through available ports */
    for (const QSerialPortInfo& ser : getPorts()) {
        if (ser.systemLocation() == port) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get current serial status as string
 *
 * @return String of serial status
 */
QString Serial::getStatus()
{
    QString msg;

    if (isConnected()) {
        msg = tr("Serial Connected to %1").arg(mTransport->portName());
    } else {
        msg = tr("Serial Not connected");
    }

    return msg;
}

/**
 * @brief Get available serial ports
 *
 * @return List of serial ports
 */
QList<QSerialPortInfo> Serial::getPorts()
{
    return QSerialPortInfo::availablePorts();
}

/**
 * @brief Write into serial transport
 *
 * @param packet Reference to byte array data to be sent
 * @return -1 in case of error, otherwise amount of transfered bytes
 */
int Serial::write(QByteArray& packet)
{
    if (isConnected()) {
        return (mTransport->write(packet));
    }

    return -1;
}

/**
 * @brief Listener for data ready event
 */
void Serial::onReadyRead()
{
    QByteArray packet;

    while (0 < mTransport->bytesAvailable()) {
        packet.append(mTransport->readAll());
    }

    emit packetReady(packet);
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
        msg = mTransport->errorString();
        emit errorOccured(msg);
        disconnect();
    }
}
