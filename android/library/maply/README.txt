To generate the header files for Java class files we use an external tool in Android Studio:
Program: javac
Arguments: $FileName$ -classpath "&Classpath&" -h $SourcepathEntry$/../../../jni/include/generated/
Working Dir: $FileDir$

