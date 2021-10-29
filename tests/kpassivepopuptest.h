#ifndef KPASSIVEPOPUPTEST_H
#define KPASSIVEPOPUPTEST_H

#include <QObject>
#include <QSystemTrayIcon>

class Test : public QObject
{
    Q_OBJECT

public:
    Test()
        : QObject()
    {
    }
    ~Test() override
    {
    }

public Q_SLOTS:
    void showIt();
    void showIt2();
    void showIt3();
    void showIt4();
    void showIt5();
    void showIt6();
    void showIt7();
    void showItIcon(QSystemTrayIcon::ActivationReason);
};

#endif // KPASSIVEPOPUPTEST_H
