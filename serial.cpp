#include "serial.h"

Serial::Serial(QObject* parent)
    : QObject { parent }
{
    mTransport = new QSerialPort(this);

    connect(mTransport, &QSerialPort::readyRead, this, &Serial::onReadyRead);
    connect(mTransport, &QSerialPort::errorOccurred, this, &Serial::onError);
}

void Serial::toggleConnection(const QString& port, int baudrate)
{
    if (isConnected()) {
        disconnect();
    } else {
        connectSerial(port, baudrate);
    }
}

bool Serial::connectSerial(const QString& port, int baudrate)
{
    if (!isValidPort(port)) {
        emit errorOccured(tr("Invalid serial port %1").arg(port));
        return false;
    }

    if (!isConnected()) {
#ifdef Q_OS_UNIX
        QFileInfo fi(port);
        if (fi.exists()) {
            if (!fi.isWritable()) {
                emit errorOccured(tr("Serial port is not writable: %1").arg(port));
                return false;
            }
        }
#endif

        mTransport->setPortName(port);
        mTransport->open(QIODevice::ReadWrite);
        if (!isConnected()) {
            return false;
        }

        mTransport->setBaudRate(baudrate);
        mTransport->setDataBits(QSerialPort::Data8);
        mTransport->setParity(QSerialPort::NoParity);
        mTransport->setStopBits(QSerialPort::OneStop);
        mTransport->setFlowControl(QSerialPort::NoFlowControl);
    }

    emit statusChanged(isConnected());
    return true;
}

void Serial::disconnect()
{
    if (mTransport->isOpen()) {
        mTransport->flush();
        mTransport->close();
    }
    emit statusChanged(isConnected());
}

bool Serial::isConnected()
{
    return mTransport->isOpen();
}

bool Serial::isValidPort(const QString& port)
{
    for (const QSerialPortInfo& ser : ports()) {
        if (ser.systemLocation() == port) {
            return true;
        }
    }
    return false;
}

QString Serial::currentStatus()
{
    if (isConnected()) {
        return (tr("Serial Connected to %1").arg(mTransport->portName()));
    }
    return tr("Serial Not connected");
}

QList<QSerialPortInfo> Serial::ports()
{
    return QSerialPortInfo::availablePorts();
}

int Serial::write(QByteArray& packet)
{
    if (!isConnected()) {
        return -1;
    }

    return (mTransport->write(packet));
}

void Serial::onReadyRead()
{
    QByteArray packet;

    while (mTransport->bytesAvailable() > 0) {
        packet.append(mTransport->readAll());
    }
    emit packetReady(packet);
}

void Serial::onError(QSerialPort::SerialPortError error)
{
    if (QSerialPort::NoError == error) {
        return;
    }

    emit errorOccured(mTransport->errorString());
    disconnect();
}
