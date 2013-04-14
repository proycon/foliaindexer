# $Id: Makefile.am 14823 2012-05-30 15:11:18Z sloot $
# $URL $

AM_CPPFLAGS = -I@top_srcdir@/include
AM_CXXFLAGS = -DSYSCONF_PATH=\"$(SYSCONFDIR)\"

bin_PROGRAMS = foliaindexer
foliaindexer_SOURCES = foliaindexer.cxx
foliaindexer_CPPFLAGS = $(BOOST_CPPFLAGS) -Wall -std=c++11
foliaindexer_LDFLAGS = $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
foliaindexer_LIBS = $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)


