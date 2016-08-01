#
#    Author        : Paul Onions
#    Creation date : 30 June 2016
#
#    Copyright 2016 Silicon Infusion Limited
#
#    Silicon Infusion Limited
#    CP House
#    Otterspool Way
#    Watford WD25 8HP
#    Hertfordshire, UK
#    Tel: +44 (0)1923 650404
#    Fax: +44 (0)1923 650374
#    Web: www.siliconinfusion.com
#
#    Licence: MIT, see LICENCE file for details.
#
PREFIX=/usr
EXTRA_CFLAGS=-Wall -std=c99
INCS=zwexc.h zwspi.h libzaltys-zwire.h
SRCS=zwexc.c zwspi.c
OBJS=$(SRCS:c=o)
SHOBJS=$(SRCS:c=so)
LIB=libzaltys-zwire.a
SHLIB=libzaltys-zwire.so
LIBS=$(LIB) $(SHLIB)

all : $(INCS) $(LIBS)

$(LIB) : $(OBJS)
	$(AR) rv $(LIB) $(OBJS)

$(SHLIB) : $(SHOBJS)
	$(CC) -shared -o $(SHLIB) $(SHOBJS)

%.o : %.c
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

%.so : %.c
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) -fPIC $< -o $@

install : all
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	cp -a $(INCS) $(PREFIX)/include
	cp -a $(LIBS) $(PREFIX)/lib

clean :
	@rm -f *.o *.so *.a
