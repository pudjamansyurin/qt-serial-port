#include "serial.h"

Serial::Serial(QObject* parent)
    : QObject { parent }
    , mTransport(new QSerialPort)
{
    connect(mTransport, &QSerialPort::readyRead,
        this, &Serial::onReadyRead);
    connect(mTransport, &QSerialPort::errorOccurred,
        this, &Serial::onError);
}

Serial::~Serial()
{
    delete mTransport;
}

/**
 * @brief Connect to serial transport
 *
 * @param port      Port name
 * @param baudrate  Baudrate value
 * @return OK status
 */
bool Serial::conect(const QString& port, int baudrate)
{
    QString msg;

    /* validate port */
    if (!isValidPort(port)) {
        msg = tr("Invalid serial port %1").arg(port);
        emit errorOccured(msg);
        return (false);
    }

    /* only connect when disconnected */
    if (!isConnected()) {

        /* open serial connection */
        mTransport->setPortName(port);
        mTransport->open(QIODevice::ReadWrite);
        if (!isConnected()) {
            return (false);
        }

        /* configure serial parameters */
        mTransport->setBaudRate(baudrate);
        mTransport->setDataBits(QSerialPort::Data8);
        mTransport->setParity(QSerialPort::NoParity);
        mTransport->setStopBits(QSerialPort::OneStop);
        mTransport->setFlowControl(QSerialPort::NoFlowControl);
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
    if (mTransport->isOpen()) {
        mTransport->flush();
        mTransport->close();
    }

    emit statusChanged(isConnected());
    return (true);
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
        msg = tr("Serial Connected to %1").arg(mTransport->portName());
    } else {
        msg = tr("Serial Not connected");
    }

    return (msg);
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
