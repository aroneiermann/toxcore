if BUILD_TESTS

TESTS = groupchat_test
#encryptsave_test messenger_autotest crypto_test network_test assoc_test onion_test TCP_test tox_test dht_autotest
check_PROGRAMS = groupchat_test
#encryptsave_test messenger_autotest crypto_test network_test assoc_test onion_test TCP_test tox_test dht_autotest

AUTOTEST_CFLAGS = \
                         $(LIBSODIUM_CFLAGS) \
                         $(NACL_CFLAGS) \
                         $(CHECK_CFLAGS)

AUTOTEST_LDADD = \
                        $(LIBSODIUM_LDFLAGS) \
                        $(NACL_LDFLAGS) \
                        libtoxcore.la \
                        libtoxencryptsave.la \
                        $(LIBSODIUM_LIBS) \
                        $(NACL_OBJECTS) \
                        $(NACL_LIBS) \
                        $(CHECK_LIBS)



if BUILD_AV
TESTS += toxav_basic_test toxav_many_test
check_PROGRAMS += toxav_basic_test toxav_many_test
AUTOTEST_LDADD += libtoxav.la
endif

#messenger_autotest_SOURCES = ../auto_tests/messenger_test.c

#messenger_autotest_CFLAGS = $(AUTOTEST_CFLAGS)

#messenger_autotest_LDADD = $(AUTOTEST_LDADD)


#crypto_test_SOURCES = ../auto_tests/crypto_test.c

#crypto_test_CFLAGS = $(AUTOTEST_CFLAGS)

#crypto_test_LDADD = $(AUTOTEST_LDADD)


#network_test_SOURCES = ../auto_tests/network_test.c

#network_test_CFLAGS = $(AUTOTEST_CFLAGS)

#network_test_LDADD = $(AUTOTEST_LDADD)


#assoc_test_SOURCES = ../auto_tests/assoc_test.c

#assoc_test_CFLAGS = $(AUTOTEST_CFLAGS)

#assoc_test_LDADD = $(AUTOTEST_LDADD)


#onion_test_SOURCES = ../auto_tests/onion_test.c

#onion_test_CFLAGS = $(AUTOTEST_CFLAGS)

#onion_test_LDADD = $(AUTOTEST_LDADD)


#TCP_test_SOURCES = ../auto_tests/TCP_test.c

#TCP_test_CFLAGS = $(AUTOTEST_CFLAGS)

#TCP_test_LDADD = $(AUTOTEST_LDADD)


#tox_test_SOURCES = ../auto_tests/tox_test.c

#tox_test_CFLAGS = $(AUTOTEST_CFLAGS)

#tox_test_LDADD = $(AUTOTEST_LDADD)


#dht_autotest_SOURCES = ../auto_tests/dht_test.c

#dht_autotest_CFLAGS = $(AUTOTEST_CFLAGS)

#dht_autotest_LDADD = $(AUTOTEST_LDADD)


if BUILD_AV
toxav_basic_test_SOURCES = ../auto_tests/toxav_basic_test.c

toxav_basic_test_CFLAGS = $(AUTOTEST_CFLAGS)

toxav_basic_test_LDADD = $(AUTOTEST_LDADD) $(AV_LIBS)


toxav_many_test_SOURCES = ../auto_tests/toxav_many_test.c

toxav_many_test_CFLAGS = $(AUTOTEST_CFLAGS)

toxav_many_test_LDADD = $(AUTOTEST_LDADD)
endif

endif


#encryptsave_test_SOURCES = ../auto_tests/encryptsave_test.c

#encryptsave_test_CFLAGS = $(AUTOTEST_CFLAGS)

#encryptsave_test_LDADD = $(AUTOTEST_LDADD)


groupchat_test_SOURCES = ../auto_tests/groupchat_test.c

groupchat_test_CFLAGS = $(AUTOTEST_CFLAGS)

groupchat_test_LDADD = $(AUTOTEST_LDADD)


EXTRA_DIST += $(top_srcdir)/auto_tests/friends_test.c
