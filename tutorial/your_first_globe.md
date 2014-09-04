---
title: Your First Globe
layout: tutorial
---

Open HelloEarth in Xcode, if it's not already open. From the Xcode File menu, select File → New → File... and, when the Choose a template for your new file dialog appears, select Cocoa Touch and then Objective­-C class.

Next, Xcode will ask you what kind of class you want. Call your new class MainViewController, and set it to be a subclass of UIViewController.

Finally, verify that the directory in which Xcode proposes to save your new class is correct (you should see a greyed ­out HelloEarth­Info.plist in the list of files). The other defaults should be fine. Click Create.

Next, we'll need to import the headers. Open MainViewController.h and this to the list of imports.

~~~objc
#import "WhirlyGlobeComponent.h"
~~~

Now let's actually add a WG-­Maply component. We're going to keep it very simple at first – we just want to verify the project setup before getting much further along.

Open MainViewController.m, and replace the @implementation line with the following:

~~~objc
@implementation MainViewController
{
  WhirlyGlobeViewController *theViewC;
}
~~~

Now we've got a private WhirlyGlobeViewController. Let's set it up but leave it empty, and add it to our view. Modify the viewDidLoad method to read as follows:

~~~objc
(void)viewDidLoad
{
  [super viewDidLoad];

  // Create an empty globe and add it to the view
  theViewC = [[WhirlyGlobeViewController alloc] init];
  [self.view addSubview:theViewC.view];
  theViewC.view.frame = self.view.bounds;
  [self addChildViewController:theViewC];
}
~~~

Almost there... now we've got to bring our new view controller up when the app starts.

Open AppDelegate.m, and add the following after the import of AppDelegate.h

~~~objc
#import "MainViewController.h"
~~~

Now, modify didFinishLaunchingWithOptions to read:

~~~objc
(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

  // Override point for customization after application launch.
  MainViewController *viewC = [[MainViewController alloc] init];
  self.window.rootViewController = viewC;
  [self.window makeKeyAndVisible];

  return YES;
}
~~~

That's it! Pick a real or virtual iOS device and run the app. If you get a blank screen (and no build errors), you win. At this point you can be certain that your project setup is correct, and you can proceed to add WG­-Maply features with confidence.

If you encounter build errors, or something other than a black screen comes up on the device review the above steps, and ensure that you have done everything correctly!

Next up, adding a layer to your globe.

[Adding a Layer](adding_a_layer.html)
