---
title: Your First Map
layout: tutorial
prev_next:
  prev: globe_or_map.html
  next: local_image_layer.html
---

You'll need to start with the [HelloEarth app](hello_earth.html) you wrote earlier.  Go do that if you haven't already.

Open HelloEarth in Xcode.  The template we used earlier should have ViewController.h and ViewController.m files.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/globe_snap_1.png)

If you're using Objective-C, we'll need to import the headers. Open ViewController.h and and this to the list of imports.

{% highlight objc %}
#import <MaplyComponent.h>
{% endhighlight %}

Now let's actually add a MaplyViewController. We're going to keep it very simple at first – we just want to verify the project setup before getting much further along.

For Objective-C, open ViewController.m, and find the @implementation line.  For Swift, open your ViewController.swift file. Modify it like so.


{% multiple_code %}
  {% highlight objc %}
@implementation ViewController
{
  MaplyViewController *theViewC;
}
{% endhighlight %}

  {----}

  {% highlight swift %}
  class ViewController : UIViewController {
      private var theViewC: MaplyViewController?
    
    ...
  }
  {% endhighlight %}
{% endmultiple_code %}

Now we've got a private MaplyViewController. Let's set it up but leave it empty, and add it to our view. Modify the viewDidLoad method to read as follows.

{% multiple_code %}
  {% highlight objc %}
(void)viewDidLoad
{
  [super viewDidLoad];

  // Create an empty globe and add it to the view
  theViewC = [[MaplyViewController alloc] init];
  [self.view addSubview:theViewC.view];
  theViewC.view.frame = self.view.bounds;
  [self addChildViewController:theViewC];
}
  {% endhighlight %}

  {----}

  {% highlight swift %}
override func viewDidLoad() {
  super.viewDidLoad()
      
  // Create an empty globe and add it to the view
  theViewC = MaplyViewController()
  self.view.addSubview(theViewC!.view)
  theViewC!.view.frame = self.view.bounds
  addChildViewController(theViewC!)
}
  {% endhighlight %}
{% endmultiple_code %}

That's it! Pick a real or virtual iOS device and run the app. If you get a blank screen (and no build errors), you win. At this point you can be certain that your project setup is correct, and you can proceed to add WhirlyGlobe-­Maply features with confidence.

If you encounter build errors, or something other than a black screen comes up on the device review the above steps, and ensure that you have done everything correctly.

Next up, adding an image layer to your globe.
