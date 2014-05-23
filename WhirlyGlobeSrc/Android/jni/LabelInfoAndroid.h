#import <jni.h>
#import "Maply_jni.h"
#import "WhirlyGlobe.h"

namespace WhirlyKit
{

/**
 * Android version of the Label Info class.
 * We put Typeface in here for convenience.
 */
class LabelInfoAndroid : public LabelInfo
{
public:
	LabelInfoAndroid();

	// Clear any global refs we may be holding
	void clearRefs(JNIEnv *env);

	// Add the typeface to the label info.  Needed for rendering
	void setTypeface(JNIEnv *env,jobject typefacObj);

	// Compare typefaces
	bool typefaceIsSame(const jobject inTypeface) const;

	// Globe reference to typeface object
	jobject typefaceObj;

	// Font size
	float fontSize;

	// Used to pass the JNI Env down into the depths
	JNIEnv *env;
	jobject labelInfoObj;
};

}
