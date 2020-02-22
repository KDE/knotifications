#ifndef NOTIFYBYMACOSNOTIFICATIONCENTER_H
#define NOTIFYBYMACOSNOTIFICATIONCENTER_H

#include "knotificationplugin.h"

class NotifyByMacOSNotificationCenter : public KNotificationPlugin
{
    Q_OBJECT

public:
    NotifyByMacOSNotificationCenter(QObject* parent);
    ~NotifyByMacOSNotificationCenter() override;

    QString optionName() override { return QStringLiteral("Popup"); }
    void notify(KNotification *notification, KNotifyConfig *config) override;
    void update(KNotification *notification, KNotifyConfig *config) override;
    void close(KNotification *notification) override;
};

#endif // NOTIFYBYMACOSNOTIFICATIONCENTER_H
