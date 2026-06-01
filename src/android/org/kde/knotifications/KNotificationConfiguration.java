/*
    SPDX-FileCopyrightText: 2020-2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.knotifications;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.provider.Settings;

public class KNotificationConfiguration
{
    public static void show(Context context)
    {
        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_APP_NOTIFICATION_SETTINGS);
        intent.putExtra(Settings.EXTRA_APP_PACKAGE, context.getPackageName());

        Activity activity = (Activity)context;
        if (activity != null) {
            activity.startActivity(intent);
        }
    }
}
