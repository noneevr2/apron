# Makefile
#
# APRON Library / Java Binding
#
# Copyright (C) Antoine Mine' 2010

# This file is part of the APRON Library, released under LGPL license
# with an exception allowing the redistribution of statically linked
# executables.

# Please read the COPYING file packaged in the distribution.

include ../Makefile.config
include ../vars.mk

############################

IFLAGS = $(ICFLAGS) \
-I$(GMP_PREFIX)/include \
-I$(MPFR_PREFIX)/include \
-I$(PPL_PREFIX)/include \
-I${JAVA_HOME}/include \
-I${JAVA_HOME}/include/linux \
-I../apron \
-I../box \
-I../newpolka \
-I../octagons \
-I../ppl \
-I../products \
$(JNIINC) 

LFLAGS = $(LDFLAGS) \
-L$(GMP_PREFIX)/lib \
-L$(MPFR_PREFIX)/lib \
-L$(PPL_PREFIX)/lib \
-L. \
-L../apron \
-L../box \
-L../newpolka \
-L../octagons \
-L../ppl \
-L../products


CFLAGSN = $(CFLAGS) -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-bad-function-cast

############################
#SOURCES
########

GMPLIBS   = -lmpfr -lgmp -lm

# .java / .c pairs
GMPMODS = Mpz MpzRef Mpq Mpfr RandState

# lone .java
GMPJAVA = Test

APRONLIBS = -lboxD -loctD -lpolkaMPQ -lapron -ljgmp $(GMPLIBS)

# .java / .c pairs
APRONMODS = Dimchange Dimperm Linexpr0 Lincons0 \
            Texpr0Intern Manager Abstract0 Environment Var \
	    Octagon Polka PolkaEq Box

ifneq ($(HAS_PPL),)
APRONMODS := $(APRONMODS) PplPoly PplGrid PolkaGrid
APRONLIBS := $(APRONLIBS) -lap_pkgrid -lap_ppl -lppl
endif

# long .java
APRONJAVA = Scalar MpqScalar MpfrScalar DoubleScalar Interval Coeff StringVar \
            Linterm0 Dimension Texpr0Node Texpr0BinNode Texpr0UnNode \
            Texpr0DimNode Texpr0CstNode Tcons0 Generator0 Linterm1 Linexpr1 \
            Lincons1 Texpr1Intern Texpr1Node Texpr1BinNode Texpr1UnNode \
            Texpr1VarNode Texpr1CstNode Tcons1 Generator1 Abstract1 \
            ApronException NotImplementedException OutOfSpaceException \
            OverflowException TimeoutException \
            Test

############################
#OBJECTS
########

GMPJ       = $(addprefix gmp/,$(addsuffix .java,$(GMPMODS) $(GMPJAVA)))
GMPCLASS   = $(subst .java,.class,$(GMPJ))
GMPC       = $(addprefix gmp/gmp_,$(addsuffix .c,$(GMPMODS)))
GMPO       = $(addprefix gmp/gmp_,$(addsuffix .o,$(GMPMODS)))
GMPH       = $(addprefix gmp/gmp_,$(addsuffix .h,$(GMPMODS)))

APRONJ     = $(addprefix apron/,$(addsuffix .java,$(APRONMODS) $(APRONJAVA)))
APRONCLASS = $(subst .java,.class,$(APRONJ))
APRONC     = $(addprefix apron/apron_,$(addsuffix .c,$(APRONMODS)))
APRONO     = $(addprefix apron/apron_,$(addsuffix .o,$(APRONMODS)))
APRONH     = $(addprefix apron/apron_,$(addsuffix .h,$(APRONMODS)))

GMPALL     = $(GMPCLASS) $(GMPH) libjgmp.$(EXT_DLL)
APRONALL   = $(APRONCLASS) $(APRONH) $(APRONO) libjapron.$(EXT_DLL)


# built and installed in JAVA_PREFIX
JAVAINST   = gmp.jar apron.jar

# built and installed in APRON_LIB
SOINST     = libjgmp.$(EXT_DLL) libjapron.$(EXT_DLL)

############################
#TARGETS
########

all: $(GMPALL) $(APRONALL) $(JAVAINST) $(SOINST)

ifneq ($(JAVAC_HAS_H),)

$(GMPCLASS): $(GMPJ)
	$(JAVAC) -h gmp $+

else

$(GMPCLASS): $(GMPJ)
	$(JAVAC) $+

gmp/gmp_%.h: gmp/%.java
	$(JAVAH) -o $@ gmp.$*
	touch $@

endif

gmp/%.o: gmp/%.c $(GMPH) gmp/jgmp.h
	$(CC) $(CFLAGSN) -c $(IFLAGS) $< -o $@

libjgmp.$(EXT_DLL): gmp/jgmp.o $(GMPO)
	$(CC) $(CFLAGSN) -shared $+ -o $@ $(LFLAGS) $(GMPLIBS)

gmp.jar: $(GMPCLASS)
	$(JAR) cf $@ $+
	$(JAR) i $@


ifneq ($(JAVAC_HAS_H),)

$(APRONCLASS): $(APRONJ)
	$(JAVAC) -h apron $+

# javac -h does not generate an apron_Var.h
apron/apron_Var.h:
	touch $@

else

$(APRONCLASS): $(APRONJ)
	$(JAVAC) $+

apron/apron_%.h: apron/%.java
	$(JAVAH) -o $@ apron.$*
	touch $@
endif

apron/%.o: apron/%.c $(APRONH)  gmp/jgmp.h apron/japron.h
	$(CC) $(CFLAGSN) -c $(IFLAGS) -Igmp $< -o $@

libjapron.$(EXT_DLL): apron/japron.o $(APRONO)
	$(CC) $(CFLAGSN) -shared $+ -o $@ $(LFLAGS) $(APRONLIBS)

apron.jar: $(APRONCLASS)
	$(JAR) cf $@ $+
	$(JAR) i $@

doc:
	$(JAVADOC) -d doc gmp apron

install:
	$(INSTALLd) $(APRON_LIB); \
	for i in $(SOINST); do \
		$(INSTALL) $$i $(APRON_LIB); \
	done; \
	$(INSTALLd) $(JAVA_PREFIX); \
	for i in $(JAVAINST); do \
		$(INSTALL) $$i $(JAVA_PREFIX); \
	done

uninstall:
	for i in $(SOINST); do \
		/bin/rm -f $(APRON_LIB)/$$i; \
	done; \
	for i in $(JAVAINST); do \
		/bin/rm -f $(JAVA_PREFIX)/$$i; \
	done

distclean: clean

test:
	$(JAVA) -cp gmp.jar -ea -esa -Xcheck:jni gmp.Test
	$(JAVA) -cp gmp.jar:apron.jar -ea -esa apron.Test

dist: $(GMPJ) $(APRONJ) $(GMPC) $(APRONC) apron/japron.c apron/japron.h gmp/jgmp.c gmp/jgmp.h Makefile README COPYING
	(cd ..; tar zcvf japron.tgz $(^:%=japron/%))


clean:
	rm -f *~ \#* */*~ *.o *.class *.$(EXT_DLL) *.jar */\#* */*.o */*.class */*.$(EXT_DLL) $(GMPH) $(APRONH)
	rm -rf doc

.PHONY: clean doc install unsintall distclean dist

