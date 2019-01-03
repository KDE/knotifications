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

import android.graphics.drawable.Icon;
import java.util.ArrayList;

/** Java side of KNotification.
 *  Used to convey the relevant notification data to Java.
 */
public class KNotification
{
    public int id;
    public String text;
    public String title;
    public Icon icon;
    public ArrayList<String> actions = new ArrayList<String>();

    public void setIconFromData(byte[] data, int length)
    {
        icon = Icon.createWithData(data, 0, length);
    }

    public void addAction(String action)
    {
        actions.add(action);
    }
}
