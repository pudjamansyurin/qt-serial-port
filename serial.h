#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class Serial : public QObject {
    Q_OBJECT
public:
    explicit Serial(QObject* parent = nullptr);
    ~Serial();

    bool conect(const QString& port, int baudrate);
    bool disconnect();
    QString getStatus() const;

    /**
     * @brief Toggle connection state
     *
     * @param port      Reference to port value
     * @param baudrate  Baudrate value
     */
    void toggle(const QString& port, int baudrate)
    {
        isConnected() ? disconnect() : conect(port, baudrate);
    }

    /**
     * @brief Check is serial transport connection state
     *
     * @return true if connected, otherwise not.
     */
    bool isConnected() const
    {
        return (mTransport->isOpen());
    }

    /**
     * @brief Get available serial ports
     *
     * @return List of serial ports
     */
    QList<QSerialPortInfo> getPorts() const
    {
        return (QSerialPortInfo::availablePorts());
    }

    /**
     * @brief Write into serial transport
     *
     * @param packet Reference to byte array data to be sent
     * @return -1 in case of error, otherwise amount of transfered bytes
     */
    int write(const QByteArray& packet)
    {
        return (isConnected() ? mTransport->write(packet) : -1);
    }

private:
    QSerialPort* mTransport;

    bool isValidPort(const QString& port) const;

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

signals:
    void statusChanged(bool connected);
    void errorOccured(const QString& error);
    void packetReady(const QByteArray& packet);
};

#endif // SERIAL_H
