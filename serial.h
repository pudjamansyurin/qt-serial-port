#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class Serial : public QObject {
    Q_OBJECT
public:
    explicit Serial(int timerFreq, QObject* parent = nullptr);
    ~Serial();

    bool conect(const QString& port, int baud);
    bool disconnect();
    QString getStatus() const;

    int write(const QByteArray& packet);
    void setAutoBreak(bool state);
    void toggle(const QString& port, int baud);
    bool isConnected() const;
    QList<QSerialPortInfo> getPorts() const;

private:
    QSerialPort* mPort;
    QTimer* mTimer;
    bool mAutoBreak;

    bool isValidPort(const QString& port) const;

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

signals:
    void statusChanged(bool connected);
    void errorOccured(const QString& error);
    void packetReady(const QByteArray& packet);
    void packetEmpty(void);
};

#endif // SERIAL_H
