#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class Serial : public QObject {
    Q_OBJECT
public:
    explicit Serial(QObject* parent = nullptr);

    void toggleConnection(const QString& port, int baudrate);
    bool connectSerial(const QString& port, int baudrate);
    void disconnect();
    bool isConnected();

    QString getStatus();
    QList<QSerialPortInfo> getPorts();

    int write(QByteArray& packet);

private:
    QSerialPort* mTransport;

    bool isValidPort(const QString& port);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

signals:
    void statusChanged(bool connected);
    void errorOccured(const QString& error);
    void packetReady(QByteArray& packet);
};

#endif // SERIAL_H
