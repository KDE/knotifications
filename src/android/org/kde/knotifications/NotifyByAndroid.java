/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

package org.kde.knotifications;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Icon;
import android.os.Build;
import android.util.Log;
import java.util.HashSet;

/** Java side of the Android notfication backend. */
public class NotifyByAndroid extends BroadcastReceiver
{
    private static final String TAG = "org.kde.knotifications";

    private static final String NOTIFICATION_ACTION = ".org.kde.knotifications.NOTIFICATION_ACTION";
    private static final String NOTIFICATION_DELETED = ".org.kde.knotifications.NOTIFICATION_DELETED";
    private static final String NOTIFICATION_OPENED = ".org.kde.knotifications.NOTIFICATION_OPENED";
    private static final String NOTIFICATION_ID_EXTRA = "org.kde.knotifications.NOTIFICATION_ID";
    private static final String NOTIFICATION_ACTION_ID_EXTRA = "org.kde.knotifications.NOTIFICATION_ACTION_ID";

    private android.content.Context m_ctx;
    private NotificationManager m_notificationManager;
    private int m_uniquePendingIntentId = 0;
    private HashSet<String> m_channels = new HashSet();

    public NotifyByAndroid(android.content.Context context)
    {
        Log.i(TAG, context.getPackageName());
        m_ctx = context;
        m_notificationManager = (NotificationManager)m_ctx.getSystemService(Context.NOTIFICATION_SERVICE);

        IntentFilter filter = new IntentFilter();
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_ACTION);
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_DELETED);
        filter.addAction(m_ctx.getPackageName() + NOTIFICATION_OPENED);
        m_ctx.registerReceiver(this, filter);
    }

    public void notify(KNotification notification)
    {
        Log.i(TAG, notification.text);

        // notification channel
        if (!m_channels.contains(notification.channelId)) {
            m_channels.add(notification.channelId);

            if (Build.VERSION.SDK_INT >= 26) {
                NotificationChannel channel = new NotificationChannel(notification.channelId, notification.channelName, NotificationManager.IMPORTANCE_DEFAULT);
                channel.setDescription(notification.channelDescription);
                m_notificationManager.createNotificationChannel(channel);
            }
        }

        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= 26) {
            builder = new Notification.Builder(m_ctx, notification.channelId);
        } else {
            builder = new Notification.Builder(m_ctx);
        }

        if (Build.VERSION.SDK_INT >= 23) {
            builder.setSmallIcon((Icon)notification.icon);
        } else {
            builder.setSmallIcon(m_ctx.getApplicationInfo().icon);
        }
        builder.setContentTitle(notification.title);
        builder.setContentText(notification.text);

        // taping the notification shows the app
        Intent intent = new Intent(m_ctx.getPackageName() + NOTIFICATION_OPENED);
        intent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
        PendingIntent contentIntent = PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(contentIntent);

        // actions
        int actionId = 1;
        for (String actionName : notification.actions) {
            Intent actionIntent = new Intent(m_ctx.getPackageName() + NOTIFICATION_ACTION);
            actionIntent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
            actionIntent.putExtra(NOTIFICATION_ACTION_ID_EXTRA, actionId);
            PendingIntent pendingIntent = PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, actionIntent, PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Action action = new Notification.Action.Builder(0, actionName, pendingIntent).build();
            builder.addAction(action);
            ++actionId;
        }

        // notfication about user closing the notification
        Intent deleteIntent = new Intent(m_ctx.getPackageName() + NOTIFICATION_DELETED);
        deleteIntent.putExtra(NOTIFICATION_ID_EXTRA, notification.id);
        Log.i(TAG, deleteIntent.getExtras() + " " + notification.id);
        builder.setDeleteIntent(PendingIntent.getBroadcast(m_ctx, m_uniquePendingIntentId++, deleteIntent, PendingIntent.FLAG_UPDATE_CURRENT));

        m_notificationManager.notify(notification.id, builder.build());
    }

    public void close(int id)
    {
        m_notificationManager.cancel(id);
    }

    @Override
    public void onReceive(Context context, Intent intent)
    {
        String action = intent.getAction();
        int id = intent.getIntExtra(NOTIFICATION_ID_EXTRA, -1);
        Log.i(TAG, action + ": " + id + " " + intent.getExtras());

        if (action.equals(m_ctx.getPackageName() + NOTIFICATION_ACTION)) {
            int actionId = intent.getIntExtra(NOTIFICATION_ACTION_ID_EXTRA, -1);
            notificationActionInvoked(id, actionId);
        } else if (action.equals(m_ctx.getPackageName() + NOTIFICATION_DELETED)) {
            notificationFinished(id);
        } else if (action.equals(m_ctx.getPackageName() + NOTIFICATION_OPENED)) {
            Intent newintent = new Intent(m_ctx, m_ctx.getClass());
            newintent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            m_ctx.startActivity(newintent);
            notificationActionInvoked(id, 0);
        }
    }

    public native void notificationFinished(int notificationId);
    public native void notificationActionInvoked(int notificationId, int action);
}
