#include "kpassivepopuptest.h"
#include <QApplication>
#include <kpassivepopup.h>
#include <QPushButton>
#include <QLatin1String>
#include <qsystemtrayicon.h>

#include <kwindowsystem.h>

QPushButton *pb;
QPushButton *pb2;
QPushButton *pb3;
QPushButton *pb4;
QPushButton *pb5;
QPushButton *pb6;
QPushButton *pb7;
QSystemTrayIcon *icon;

void Test::showIt()
{
    KPassivePopup::message(QStringLiteral("Hello World"), pb);
}

void Test::showIt2()
{
    KPassivePopup::message(QStringLiteral("The caption is..."), QStringLiteral("Hello World"), pb2);
}

void Test::showIt3()
{
    KPassivePopup *pop = new KPassivePopup();
    pop->setView(QStringLiteral("Caption"), QStringLiteral("test"));
    pop->show();
}

void Test::showIt4()
{
    KPassivePopup::message(KPassivePopup::Boxed, QStringLiteral("The caption is..."), QStringLiteral("Hello World"), pb4);
}

void Test::showIt5()
{
    KPassivePopup::message(KPassivePopup::Balloon, QStringLiteral("The caption is..."), QStringLiteral("Hello World"), pb5);
}

void Test::showIt6()
{
    KPassivePopup::message(KPassivePopup::Boxed, QStringLiteral("The caption is..."), QStringLiteral("Hello World"), pb6);
}

void Test::showIt7()
{
    int iconDimension = QApplication::fontMetrics().height();
    KPassivePopup::message(QStringLiteral("The caption is..."), QStringLiteral("Hello World"), QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(iconDimension, iconDimension), pb2);
}

void Test::showItIcon(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        KPassivePopup::message(QStringLiteral("QSystemTrayIcon test"), QStringLiteral("Hello World"), icon);
    }
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication app(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    Test *t = new Test();

    pb = new QPushButton();
    pb->setText(QStringLiteral("By taskbar entry (no caption, default style)"));
    pb->connect(pb, SIGNAL(clicked()), t, SLOT(showIt()));
    pb->show();

    pb2 = new QPushButton();
    pb2->setText(QStringLiteral("By taskbar entry (with caption, default style)"));
    pb2->connect(pb2, SIGNAL(clicked()), t, SLOT(showIt2()));
    pb2->show();

    pb3 = new QPushButton();
    pb3->setText(QStringLiteral("Without WinID"));
    pb3->connect(pb3, SIGNAL(clicked()), t, SLOT(showIt3()));
    pb3->show();

    pb4 = new QPushButton();
    pb4->setText(QStringLiteral("By taskbar entry (with caption, boxed)"));
    pb4->connect(pb4, SIGNAL(clicked()), t, SLOT(showIt4()));
    pb4->show();

    pb5 = new QPushButton();
    pb5->setText(QStringLiteral("By taskbar entry (with caption, balloon)"));
    pb5->connect(pb5, SIGNAL(clicked()), t, SLOT(showIt5()));
    pb5->show();

    // this test depends on X11
    pb6 = new QPushButton();
    pb6->setText(QStringLiteral("By window (with caption, balloon)"));
    pb6->connect(pb6, SIGNAL(clicked()), t, SLOT(showIt6()));
    pb6->show();
    KWindowSystem::setState(pb6->effectiveWinId(), NET::SkipTaskbar);

    pb7 = new QPushButton();
    pb7->setText(QStringLiteral("By taskbar entry (with caption and icon, default style)"));
    pb7->connect(pb7, SIGNAL(clicked()), t, SLOT(showIt7()));
    pb7->show();

    icon = new QSystemTrayIcon();
    // TODO icon->setIcon(icon->loadIcon("xorg"));
    icon->connect(icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), t, SLOT(showItIcon(QSystemTrayIcon::ActivationReason)));
    icon->show();

    return app.exec();

}
