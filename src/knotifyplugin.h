/*
   Copyright (C) 2005-2006 by Olivier Goffart <ogoffart at kde.org>


   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */



#ifndef KNOTIFYPLUGIN_H
#define KNOTIFYPLUGIN_H

#include <QtCore/QObject>
#include <KPluginFactory>

#include "knotify_export.h"

class KNotifyPluginPrivate;
class KNotifyConfig;


/**
 * @brief abstract class for KNotify actions
 *
 * A KNotifyPlugin is responsible of one presentation.  You can subclass it to have your own knotify presentation.
 *
 * You should reimplement the KNotifyPlugin::notify method to display the notification.
 *
 * @author Olivier Goffart <ogoffart at kde.org>
*/
class KNOTIFY_EXPORT KNotifyPlugin : public QObject
{ Q_OBJECT
	public:
	        KNotifyPlugin(QObject *parent=0l, const QVariantList &args=QVariantList());
		virtual ~KNotifyPlugin();

		/**
		 * @brief return the name of this plugin.
		 *
		 * this is the name that should appear in the .knotifyrc file,
		 * in the field Action=... if a notification is set to use this plugin
		 */
		virtual QString optionName() =0;
		/**
		 * This function is called when the notification is sent.
		 * (or re-sent)
		 * You should implement this function to display a notification
		 *
		 * for each call to this function (even for re-notification), you MUST call finish(int)
		 *
		 * @param id is the notification id
		 * @param config is the configuration of the notification
		 */
		virtual void notify(int id , KNotifyConfig *config )=0;

		/**
		 * This function is called when the notification has changed (such as the text or the icon)
		 */
		virtual void update(int id, KNotifyConfig *config);

		/**
		 * This function is called when the notification has been closed
		 */
		virtual void close(int id);

	protected:
		/**
		 * emit the finished signal
		 * you MUST call this function for each call to notify(), even if you do nothing there
		 *
		 * call it when the presentation is finished (because the user closed the popup or the sound is finished)
		 *
		 * If your presentation is synchronous, you can even call this function from the notify() call itself
		 */
		void finish(int id);

	Q_SIGNALS:
		/**
		 * the presentation is finished.
		 */
		void finished(int id);
		/**
		 * emit this signal if one action was invoked
		 * @param id is the id of the notification
		 * @param action is the action number.  zero for the default action
		 */
		void actionInvoked(int id , int action);

	private:
		KNotifyPluginPrivate *const d;

};

#define K_EXPORT_KNOTIFY_METHOD(libname,classname) \
K_PLUGIN_FACTORY(KNotifyMethodPluginFactory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(KNotifyMethodPluginFactory("knotify_method_" #libname))

#endif
