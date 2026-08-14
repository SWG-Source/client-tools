TEMPLATE = subdirs

unix:!embedded:!mac:CONFIG += x11

SUBDIRS	 =	aclock \
		action \
		action/toggleaction \
		addressbook \
		application \
		buttongroups \
		checklists \
		cursor \
		customlayout \
		dclock \
		demo \
		dirview \
		distributor \
		dragdrop \
		drawdemo \
		drawlines \
		extension \
		fonts \
		forever \
		gridview \
		hello \
		helpdemo \
		helpviewer \
		i18n \
		layout \
		life \
		lineedits \
		listboxcombo \
		listbox \
		listviews \
		menu \
		movies \
		picture \
		popup \
		process \
		progress \
		progressbar \
		qdir \
		qfd \
		qmag \
		qtl \
		qwerty \
		rangecontrols \
		richtext \
		rot13 \
		scribble \
		scrollview \
		showimg \
		splitter \
		tabdialog \
		tetrix \
		textedit \
		themes \
		tictac \
		tooltip \
		toplevel \
		tux \
		widgets \
		wizard \
		xform

thread:SUBDIRS +=   thread/prodcons \
		    thread/semaphores

canvas:SUBDIRS +=   canvas
canvas:table:SUBDIRS += chart

opengl:SUBDIRS +=   opengl/box \
		    opengl/gear \
		    opengl/glpixmap \
		    opengl/overlay \
		    opengl/sharedbox \
		    opengl/texture

x11:nas:SUBDIRS += 	    sound
mac|win32:SUBDIRS +=	    sound

iconview:SUBDIRS += fileiconview \
		    iconview \
		    iconview/simple_dd


network:SUBDIRS +=  network/archivesearch \
		    network/clientserver/client \
		    network/clientserver/server \
		    network/ftpclient \
		    network/httpd \
		    network/infoprotocol/infoclient \
		    network/infoprotocol/infoserver \
		    network/infoprotocol/infourlclient \
		    network/mail \
		    network/networkprotocol \
		    network/remotecontrol

workspace:SUBDIRS+= mdi

table:SUBDIRS +=    table/statistics \
		    table/small-table-demo \
		    regexptester \
		    table/bigtable \
			helpsystem

tablet:SUBDIRS += tablet

sql:SUBDIRS += sql

xml:SUBDIRS +=	xml/outliner \
		xml/tagreader \
		xml/tagreader-with-features

embedded:SUBDIRS += launcher

embedded:SUBDIRS -= showimg

#win32:SUBDIRS += trayicon
embedded:SUBDIRS += trayicon

x11:SUBDIRS	+= biff \
		   desktop
