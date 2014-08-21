---
title: Adding Vector Data
layout: tutorial
---

HelloEarth is looking good, but still plain. Almost all map or globe apps have some sort of overlay which adds additional information or context. Let's add some country outlines to get a taste of what's involved when adding vector data in WG­Maply.

Go back to the resources directory and check out vectors/country_json_50m. These are the country outlines we'll use for this example. You can drag all of these .geojson files into your project or just your favorites.

*drag 'em in, screen shot*

Now, let's add them to the display. Open MainViewController.m and modify the @interface section to add a private method:

~~~objc
@interface MainViewController ()

(void) addCountries;

@end
~~~

Add a private NSDictionary member in the implementation section, to set our default vector characteristics:

~~~objc
NSDictionary *vectorDict;
~~~

Next, add some code to set the vector properties and call the addCountries method at the end of viewDidLoad:

~~~objc
// set the vector characteristics to be pretty and selectable
vectorDict = @{
  kMaplyColor: [UIColor whiteColor], 
  kMaplySelectable: @(true), 
  kMaplyVecWidth: @(4.0)};

// add the countries
[self addCountries];
~~~

Finally, add the addCountries method itself after viewDidLoad:

~~~objc
­(void)addCountries
{
// handle this in another thread
dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0),
^{
NSArray *allOutlines = [[NSBundle mainBundle]
pathsForResourcesOfType:@"geojson" inDirectory:nil];

for (NSString *outlineFile in allOutlines)
{
NSData *jsonData = [NSData dataWithContentsOfFile:outlineFile];
if (jsonData)
{
MaplyVectorObject *wgVecObj = [MaplyVectorObject
VectorObjectFromGeoJSON:jsonData];
~~~

for later

~~~objc
// the admin tag from the country outline geojson has the country name ­ save
NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
wgVecObj.userObject = vecName;

// add the outline to our view
MaplyComponentObject *compObj = [theViewC addVectors:[NSArray arrayWithObject:wgVecObj] desc:vectorDict];


// If you ever intend to remove these, keep track of the MaplyComponentObjects above.
}
}
});
}
~~~

Build and run the app. You should see the outlines of the countries you included in HelloEarth.

*Two sentences on VectorObjectFromGeoJSON and addVectors? The code is pretty self-
explanatory.*

Next up, taps and selection.

[Taps and Selection](taps_and_selection.html)

