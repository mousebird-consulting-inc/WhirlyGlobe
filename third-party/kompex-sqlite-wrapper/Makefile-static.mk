# Include project Makefile
include Makefile

# Object Directory
objsdir=${builddir}/${CONF}

# Object Files
OBJS= \
	${objsdir}/KompexSQLiteBlob.o \
	${objsdir}/KompexSQLiteStatement.o \
	${objsdir}/KompexSQLiteDatabase.o \
	${objsdir}/sqlite3.o

# C Compiler Flags
CFLAGS= -MMD -MP

# CC Compiler Flags
CPPFLAGS= -I${includedir} -MMD -MP

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: .pre-build ${prelibdir}/lib${PRODUCT_NAME}.a

.pre-build:
	$(MKDIR) -p ${prelibdir}
	$(MKDIR) -p ${objsdir}
	$(RM) ${objsdir}/*.d
	$(RM) ${prelibdir}/lib${PRODUCT_NAME}.a

${prelibdir}/lib${PRODUCT_NAME}.a: ${OBJS}
	${AR} rv ${prelibdir}/lib${PRODUCT_NAME}.a ${OBJS} 
	$(RANLIB) ${prelibdir}/lib${PRODUCT_NAME}.a

${objsdir}/KompexSQLiteBlob.o: ${srcdir}/KompexSQLiteBlob.cpp 
	$(COMPILE.cc) -MF $@.d -o $@ $^

${objsdir}/KompexSQLiteStatement.o: ${srcdir}/KompexSQLiteStatement.cpp 
	$(COMPILE.cc) -MF $@.d -o $@ $^

${objsdir}/KompexSQLiteDatabase.o: ${srcdir}/KompexSQLiteDatabase.cpp 
	$(COMPILE.cc) -MF $@.d -o $@ $^

${objsdir}/sqlite3.o: ${srcdir}/sqlite3.c 
	$(COMPILE.c) ${CFLAGS} -MF $@.d -o $@ $^

