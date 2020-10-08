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
CFLAGS= -fPIC -MMD -MP

# CC Compiler Flags
CPPFLAGS= -DKOMPEX_SQLITEWRAPPER_EXPORT -DKOMPEX_SQLITEWRAPPER_DYN -fPIC -MMD -MP -I${includedir}

# Link Libraries and Options
LDLIBSOPTIONS= -shared -fPIC

# Build Targets
.build-conf: .pre-build ${prelibdir}/lib${PRODUCT_NAME}.so

.pre-build:
	$(MKDIR) -p ${prelibdir}
	$(MKDIR) -p ${objsdir}
	$(RM) ${objsdir}/*.d

${prelibdir}/lib${PRODUCT_NAME}.so: ${OBJS}
	${LINK.cc} -o ${prelibdir}/lib${PRODUCT_NAME}.so ${OBJS} ${LDLIBSOPTIONS} 

${objsdir}/KompexSQLiteBlob.o: ${srcdir}/KompexSQLiteBlob.cpp 
	$(COMPILE.cc) ${CXXFLAGS} -MF $@.d -o $@ $^

${objsdir}/KompexSQLiteStatement.o: ${srcdir}/KompexSQLiteStatement.cpp 
	$(COMPILE.cc) ${CXXFLAGS} -MF $@.d -o $@ $^

${objsdir}/KompexSQLiteDatabase.o: ${srcdir}/KompexSQLiteDatabase.cpp 
	$(COMPILE.cc) ${CXXFLAGS} -MF $@.d -o $@ $^

${objsdir}/sqlite3.o: ${srcdir}/sqlite3.c 
	$(COMPILE.c) ${CFLAGS} -MF $@.d -o $@ $^

