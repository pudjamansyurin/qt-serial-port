#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class Serial : public QObject {
    Q_OBJECT
public:
    explicit Serial(int sampleMS, QObject* parent = nullptr);
    ~Serial();

    bool conect(const QString& port, int baudrate);
    bool disconnect();
    QString getStatus() const;

    int write(const QByteArray& packet);

    /**
     * @brief Send automatic break signal
     *
     * @param state Break state
     */
    void setAutoBreak(bool state)
    {
        mAutoBreak = state;
    }

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
        return (mPort->isOpen());
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

private:
    QSerialPort* mPort;
    bool mAutoBreak;

    int mSampleMS;
    QTimer* mTimer;

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
