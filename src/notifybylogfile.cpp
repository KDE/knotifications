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


#include "notifybylogfile.h"

#include <kdebug.h>
#include <knotifyconfig.h>
#include <KUrl>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

NotifyByLogfile::NotifyByLogfile(QObject *parent) : KNotifyPlugin(parent)
{
}


NotifyByLogfile::~NotifyByLogfile()
{
}



void NotifyByLogfile::notify( int id, KNotifyConfig * config )
{
	QString file=config->readEntry( "Logfile" );

//	kDebug() << file << KUrl(file).path();
	
	if ( file.isEmpty() )
	{
		finish( id );
		return;
	}
	
    // open file in append mode
    	QFile logFile(KUrl(file).path());
	if ( !logFile.open(QIODevice::WriteOnly | QIODevice::Append) )
	{
		finish( id );
		return;
	}

	QString text = config->text;
	if( text.isEmpty())
		text = config->readEntry( "Name" );
    // append msg
	QTextStream strm( &logFile );
	strm << "- KNotify " << QDateTime::currentDateTime().toString() << ": ";
	strm << text << endl;

    // close file
	logFile.close();
	
	finish( id );
}

#include "notifybylogfile.moc"
