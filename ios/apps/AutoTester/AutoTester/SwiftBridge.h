#if !defined(STRINGIFY) && !defined(_STRINGIFY)
# define STRINGIFY_(x) #x
# define STRINGIFY(x) STRINGIFY_(x)
# define CONCAT(x,y) x##y
#endif

#if !defined(SWIFT_BRIDGE)
# define SWIFT_BRIDGE AutoTester-Swift.h
#endif

#import STRINGIFY(SWIFT_BRIDGE)
