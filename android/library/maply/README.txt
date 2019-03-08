To generate the header files for Java class files we use an external tool in Android Studio:
Program: javac
Arguments: $FileDirRelativeToSourcepath$/$FileName$ -h ../../../jni/include/generated/ -d /tmp/android_studio_junk/
Working Dir: $SourcepathEntry$

