MAINTAINERCLEANFILES = Makefile.in

SUBDIRS =

AUTOMAKE_OPTIONS = no-dependencies

#A test
exceptionstest_SOURCES= unittest.cpp exceptionstest.hpp exceptionstest.cpp
exceptionstest_CPPFLAGS= -I$(top_srcdir)/src/
exceptionstest_LDFLAGS= -static
exceptionstest_LDADD= $(top_builddir)/src/rofl/librofl_common.la -lcppunit

#Tests

check_PROGRAMS= exceptionstest
TESTS = exceptionstest
