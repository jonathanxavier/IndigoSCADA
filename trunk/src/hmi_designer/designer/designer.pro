TEMPLATE	= subdirs
SUBDIRS		=  uic \
		   uilib \
		   designer \
		   app
dll:SUBDIRS *=  editor plugins
shared:SUBDIRS *=  editor plugins
CONFIG += ordered
