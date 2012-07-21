/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "config.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

Config::Config( const QString &fn )
    : filename( fn )
{
    git = groups.end();
    read();
}

Config::~Config()
{
    write();
}

void Config::setGroup( const QString &gname )
{
    QMap< QString, ConfigGroup>::Iterator it = groups.find( gname );
    if ( it == groups.end() ) {
	ConfigGroup grp;
	git = groups.insert( gname, grp );
	return;
    }
    git = it;
}

void Config::writeEntry( const QString &key, const QString &value )
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    ( *git ).insert( key, value );
}

void Config::writeEntry( const QString &key, int num )
{
    QString s;
    s.setNum( num );
    writeEntry( key, s );
}

#if !defined(Q_NO_BOOL_TYPE)
void Config::writeEntry( const QString &key, bool b )
{
    QString s;
    s.setNum( ( int )b );
    writeEntry( key, s );
}
#endif

void Config::writeEntry( const QString &key, const QStringList &lst, const QChar &sep )
{
    QString s;
    QStringList::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it )
	s += *it + sep;
    writeEntry( key, s );
}

QString Config::readEntry( const QString &key, const QString &deflt )
{
    if ( git == groups.end() ) {
	//qWarning( "no group set" );
	return deflt;
    }
    ConfigGroup::Iterator it = ( *git ).find( key );
    if ( it != ( *git ).end() )
	return *it;
    else
	return deflt;
}

int Config::readNumEntry( const QString &key, int deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return deflt;
    else
	return s.toInt();
}

bool Config::readBoolEntry( const QString &key, bool deflt )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return deflt;
    else
	return (bool)s.toInt();
}

QStringList Config::readListEntry( const QString &key, const QChar &sep )
{
    QString s = readEntry( key );
    if ( s.isEmpty() )
	return QStringList();
    else
	return QStringList::split( sep, s );
}

void Config::clearGroup()
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    ( *git ).clear();
}

void Config::write( const QString &fn )
{
    if ( !fn.isEmpty() )
	filename = fn;

    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
	qWarning( "could not open for writing `%s'", filename.latin1() );
	git = groups.end();
	return;
    }

    QTextStream s( &f );
    QMap< QString, ConfigGroup >::Iterator g_it = groups.begin();
    for ( ; g_it != groups.end(); ++g_it ) {
	s << "[" << g_it.key() << "]" << "\n";
	ConfigGroup::Iterator e_it = ( *g_it ).begin();
	for ( ; e_it != ( *g_it ).end(); ++e_it )
	    s << e_it.key() << " = " << *e_it << "\n";
    }

    f.close();
}

void Config::read()
{
    if ( !QFileInfo( filename ).exists() ) {
// 	qWarning( "`%s' doesn't exist", filename.latin1() );
	git = groups.end();
	return;
    }

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) ) {
	qWarning( "could not open for reading `%s'", filename.latin1() );
	git = groups.end();
	return;
    }

    QTextStream s( &f );

    QString line;
    while ( !s.atEnd() ) {
	line = s.readLine();
	parse( line );
    }

    f.close();
}

void Config::parse( const QString &l )
{
    QString line = l.stripWhiteSpace();
    if ( line[ 0 ] == QChar( '[' ) ) {
	QString gname = line;
	gname = gname.remove( 0, 1 );
	if ( gname[ (int)gname.length() - 1 ] == QChar( ']' ) )
	    gname = gname.remove( gname.length() - 1, 1 );
	ConfigGroup grp;
	git = groups.insert( gname, grp );
    } else {
	if ( git == groups.end() ) {
	    qWarning( "line `%s' out of group", line.latin1() );
	    return;
	}
	QStringList lst = QStringList::split( '=', line );
	if ( lst.count() != 2 && line.find( '=' ) == -1 ) {
	    qWarning( "corrupted line `%s' in group `%s'",
		      line.latin1(), git.key().latin1() );
	    return;
	}
	( *git ).insert( lst[ 0 ].stripWhiteSpace(), lst[ 1 ].stripWhiteSpace() );
    }
}
