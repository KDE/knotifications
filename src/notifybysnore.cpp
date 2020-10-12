/*
    SPDX-FileCopyrightText: 2019 Piyush Aggarwal <piyushaggarwal002@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notifybysnore.h"
#include "knotification.h"
#include "knotifyconfig.h"
#include "debug_p.h"

#include <QIcon>
#include <QLocalSocket>
#include <QGuiApplication>

#include <snoretoastactions.h>

/*
 * On Windows a shortcut to your app is needed to be installed in the Start Menu
 * (and subsequently, registered with the OS) in order to show notifications.
 * Since KNotifications is a library, an app using it can't (feasibly) be properly
 * registered with the OS. It is possible we could come up with some complicated solution
 * which would require every KNotification-using app to do some special and probably
 * difficult to understand change to support Windows. Or we can have SnoreToast.exe
 * take care of all that nonsense for us.
 * Note that, up to this point, there have been no special
 * KNotifications changes to the generic application codebase to make this work,
 * just some tweaks to the Craft blueprint and packaging script
 * to pull in SnoreToast and trigger shortcut building respectively.
 * Be sure to have a shortcut installed in Windows Start Menu by SnoreToast.
 *
 * So the location doesn't matter, but it's only possible to register the internal COM server in an executable.
 * We could make it a static lib and link it in all KDE applications,
 * but to make the action center integration work, we would need to also compile a class
 * into the executable using a compile time uuid.
 *
 * The used header is meant to help with parsing the response.
 * The cmake target for LibSnoreToast is a INTERFACE lib, it only provides the include path.
 *
 *
 * Trigger the shortcut installation during the installation of your app; syntax for shortcut installation is -
 * ./SnoreToast.exe -install <absolute\address\of\shortcut> <absolute\address\to\app.exe> <appID>
 *
 * appID: use as-is from your app's QCoreApplication::applicationName() when installing the shortcut.
 * NOTE: Install the shortcut in Windows Start Menu folder.
 * For example, check out Craft Blueprint for Quassel-IRC or KDE Connect.
*/

namespace {
    const QString SnoreToastExecName() { return QStringLiteral("SnoreToast.exe"); }
}

NotifyBySnore::NotifyBySnore(QObject* parent) :
    KNotificationPlugin(parent)
{
    m_server.listen(QString::number(qHash(qApp->applicationDirPath())));
    connect(&m_server, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket* responseSocket = m_server.nextPendingConnection();
        connect(responseSocket, &QLocalSocket::readyRead, [this, responseSocket](){
            const QByteArray rawNotificationResponse = responseSocket->readAll();
            responseSocket->deleteLater();

            const QString notificationResponse =
                        QString::fromWCharArray(reinterpret_cast<const wchar_t *>(rawNotificationResponse.constData()),
                                            rawNotificationResponse.size() / sizeof(wchar_t));
            QMap<QString, QStringRef> notificationResponseMap;
            for (auto &str : notificationResponse.splitRef(QLatin1Char(';'))) {
                const int equalIndex = str.indexOf(QLatin1Char('='));
                notificationResponseMap.insert(str.mid(0, equalIndex).toString(), str.mid(equalIndex + 1));
            }
            const QString responseAction = notificationResponseMap[QStringLiteral("action")].toString();
            const int responseNotificationId = notificationResponseMap[QStringLiteral("notificationId")].toInt();

            qCDebug(LOG_KNOTIFICATIONS) << "The notification ID is : " << responseNotificationId;

            KNotification *notification;
            const auto iter = m_notifications.constFind(responseNotificationId);
            if (iter != m_notifications.constEnd()) {
                notification = iter.value();
            }
            else {
                qCWarning(LOG_KNOTIFICATIONS) << "Received a response for an unknown notification.";
                return;
            }

            std::wstring w_action(responseAction.size(), 0);
            responseAction.toWCharArray(const_cast<wchar_t *>(w_action.data()));

            switch (SnoreToastActions::getAction(w_action)) {
            case SnoreToastActions::Actions::Clicked:
                qCDebug(LOG_KNOTIFICATIONS) << "User clicked on the toast.";
                break;

            case SnoreToastActions::Actions::Hidden:
                qCDebug(LOG_KNOTIFICATIONS) << "The toast got hidden.";
                break;

            case SnoreToastActions::Actions::Dismissed:
                qCDebug(LOG_KNOTIFICATIONS) << "User dismissed the toast.";
                break;

            case SnoreToastActions::Actions::Timedout:
                qCDebug(LOG_KNOTIFICATIONS) << "The toast timed out.";
                break;

            case SnoreToastActions::Actions::ButtonClicked: {
                qCDebug(LOG_KNOTIFICATIONS) << "User clicked an action button in the toast.";
                const QString responseButton = notificationResponseMap[QStringLiteral("button")].toString();
                QStringList s = m_notifications.value(responseNotificationId)->actions();
                int actionNum = s.indexOf(responseButton) + 1;       // QStringList starts with index 0 but not actions
                emit actionInvoked(responseNotificationId, actionNum);
                break;
            }

            case SnoreToastActions::Actions::TextEntered:
                qCWarning(LOG_KNOTIFICATIONS) << "User entered some text in the toast. This is is not handled yet.";
                break;

            default:
                qCWarning(LOG_KNOTIFICATIONS) << "Unexpected behaviour with the toast. Please file a bug report / feature request.";
                break;
            }

            // Action Center callbacks are not yet supported so just close the notification once done
            if (notification != nullptr) {
                NotifyBySnore::close(notification);
            }
        });
    });
}

NotifyBySnore::~NotifyBySnore()
{
    m_server.close();
}

void NotifyBySnore::notify(KNotification *notification, KNotifyConfig *config)
{
    Q_UNUSED(config);
    // HACK work around that notification->id() is only populated after returning from here
    // note that config will be invalid at that point, so we can't pass that along
    QMetaObject::invokeMethod(this, [this, notification](){ NotifyBySnore::notifyDeferred(notification); }, Qt::QueuedConnection);
}

void NotifyBySnore::notifyDeferred(KNotification* notification)
{
    const QString notificationTitle = ((!notification->title().isEmpty()) ? stripRichText(notification->title())
                                                                          : qApp->applicationDisplayName());
    QStringList snoretoastArgsList {
        QStringLiteral("-id"), QString::number(notification->id()),
        QStringLiteral("-t"), notificationTitle,
        QStringLiteral("-m"), stripRichText(notification->text()),
        QStringLiteral("-appID"), qApp->applicationName(),
        QStringLiteral("-pid"), QString::number(qApp->applicationPid()),
        QStringLiteral("-pipename"), m_server.fullServerName()
    };

    // handle the icon for toast notification
    const QString iconPath = m_iconDir.path() + QLatin1Char('/') + QString::number(notification->id());
    const bool hasIcon = (notification->pixmap().isNull()) ? qApp->windowIcon().pixmap(1024, 1024).save(iconPath, "PNG")
                                                           : notification->pixmap().save(iconPath, "PNG");
    if (hasIcon) {
        snoretoastArgsList << QStringLiteral("-p") << iconPath;
    }

    // add actions if any
    if (!notification->actions().isEmpty()) {
        snoretoastArgsList << QStringLiteral("-b") << notification->actions().join(QLatin1Char(';'));
    }

    qCDebug(LOG_KNOTIFICATIONS) << snoretoastArgsList;

    QProcess* snoretoastProcess = new QProcess();
    connect(snoretoastProcess, &QProcess::started, [this, notification]() {
        m_notifications.insert(notification->id(), notification); // TODO: handle failure of this call
        qCDebug(LOG_KNOTIFICATIONS) << "Inserted notification into m_notifications";
    });

    connect(snoretoastProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, snoretoastProcess, iconPath](int exitCode, QProcess::ExitStatus exitStatus) {
                snoretoastProcess->deleteLater();
                QFile::remove(iconPath);
    });

    snoretoastProcess->start(SnoreToastExecName(), snoretoastArgsList);
}

void NotifyBySnore::close(KNotification *notification)
{
    qCDebug(LOG_KNOTIFICATIONS) << "Requested to close notification with ID:" << notification->id();
    if (m_notifications.constFind(notification->id()) == m_notifications.constEnd()) {
        qCWarning(LOG_KNOTIFICATIONS) << "Couldn't find the notification in m_notifications. Nothing to close.";
        return;
    }

    const QStringList snoretoastArgsList{ QStringLiteral("-close"),
                                          QString::number(notification->id()),
                                          QStringLiteral("-appID"),
                                          qApp->applicationName()
                                        };

    qCDebug(LOG_KNOTIFICATIONS) << "SnoreToast closing notification with ID:" << notification->id();

    QProcess* snoretoastProcess = new QProcess();
    connect(snoretoastProcess, &QProcess::started, [this, notification]() {
        qCDebug(LOG_KNOTIFICATIONS) << "Removed" << m_notifications.remove(notification->id()) << "notifications from m_notifications";
    });

    connect(snoretoastProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), snoretoastProcess, &QProcess::deleteLater);
    snoretoastProcess->start(SnoreToastExecName(), snoretoastArgsList);
}

void NotifyBySnore::update(KNotification *notification, KNotifyConfig *config)
{
    Q_UNUSED(notification);
    Q_UNUSED(config);
    qCWarning(LOG_KNOTIFICATIONS) << "updating a notification is not supported yet.";
}

