---
title: Screen Marker Selection
layout: tutorial
---

Screen marker selection works just like vector selection.  WhirlyGlobe-Maply is doing very different thing to make them happen, but on the user side it's all the same.

We'll need an XCode project here, so if you haven't done the [Screen Markers](screen_markers.html) tutorial go do that.  If you haven't got one yet, here's a suitable ViewController (for [Objective-C]({{ site.baseurl }}/tutorial/code/ViewController_screen_marker_selection.m) or [Swift]({{ site.baseurl }}/tutorial/code/ViewController_screen_marker_selection.swift))

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/screen_marker_selection_1.png)

### Selecting Screen Markers

If you've got screen markers displaying, selecting them is pretty simple.  Change your handleSelection method as follows.

{% multiple_code %}
  {% highlight objc %}
// Unified method to handle the selection
- (void) handleSelection:(MaplyBaseViewController *)viewC
         selected:(NSObject *)selectedObj
{
    // ensure it's a MaplyVectorObject. It should be one of our outlines.
    if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
    {
        MaplyVectorObject *theVector = (MaplyVectorObject *)selectedObj;
        MaplyCoordinate location;

        if ([theVector centroid:&location])
        {
        NSString *title = @"Selected:";
        NSString *subtitle = (NSString *)theVector.userObject;
        [self addAnnotation:title withSubtitle:subtitle at:location];
        }
    } else if ([selectedObj isKindOfClass:[MaplyScreenMarker class]])
    {
        // or it might be a screen marker
        MaplyScreenMarker *theMarker = (MaplyScreenMarker *)selectedObj;

        NSString *title = @"Selected:";
        NSString *subtitle = @"Screen Marker";
        [self addAnnotation:title withSubtitle:subtitle at:theMarker.loc];
    }
}
­  {% endhighlight %}

  {----}

  {% highlight swift %}
// Unified method to handle the selection
private func handleSelection(selectedObject: NSObject) {
    if let selectedObject = selectedObject as? MaplyVectorObject {
        let loc = selectedObject.centroid()
        if loc.x != kMaplyNullCoordinate {
            let title = "Selected:"
            let subtitle = selectedObject.userObject as! String
            addAnnotationWithTitle(title, subtitle: subtitle, loc: loc)
        }
    }
    else if let selectedObject = selectedObject as? MaplyScreenMarker {
        let title = "Selected:"
        let subtitle = "Screen Marker"
        addAnnotationWithTitle(title, subtitle: subtitle, loc: selectedObject.loc)
    }
}
  {% endhighlight %}
{% endmultiple_code %}

That's all there is to it.  We're just handed a MaplyScreenMarker back if that's what the user selected.  We pop up a little annotation over the marker like so.

![Xcode HelloEarth]({{ site.baseurl }}/images/tutorial/screen_marker_selection_2.png)

All the other features are exactly the same.  You check if you got that type back and do something appropriate if you did.
