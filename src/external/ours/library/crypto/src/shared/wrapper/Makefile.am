AUTOMAKE_OPTIONS=foreign
noinst_LTLIBRARIES=libcryptosharedwrapper.la
libcryptosharedwrapper_la_CXXFLAGS=-I$(crypto_dir)/include \
	-I$(stlport_dir)/stlport \
	-Wno-ctor-dtor-privacy
libcryptosharedwrapper_la_SOURCES=CryptoBufferTransform.h \
	Hash.cpp \
	Hash.h \
	MD5Hash.cpp \
	MD5Hash.h \
	TwofishCrypt.cpp \
	TwofishCrypt.h \
	TwofishDecryptor.cpp \
	TwofishDecryptor.h \
	TwofishEncryptor.cpp \
	TwofishEncryptor.h
