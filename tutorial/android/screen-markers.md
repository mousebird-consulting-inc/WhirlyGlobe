---
title: Screen Markers
layout: android-tutorial
---

*Tutorial by Nicholas Hallahan.*

A basic feature found in almost every mapping toolkit is the marker. Screen markers are 2D markers that follow a location on the globe or map. As the user moves, they move, but they don’t get any bigger or smaller.

In our last tutorial, we the border of Russia as a GeoJSON feature on the map. Now we are going to add the [top five most populous cities in Russia](https://en.wikipedia.org/wiki/List_of_cities_and_towns_in_Russia_by_population).

### Import Icon as Image Asset

<figure style="float:right;"><a href="https://s3.amazonaws.com/whirlyglobedocs/tutorialsupport/ic_city.png" download><img src="https://s3.amazonaws.com/whirlyglobedocs/tutorialsupport/ic_city.png" alt="ic_city.png" style="max-width:120px;"/></a><figcaption style="text-align:center"><a href="https://s3.amazonaws.com/whirlyglobedocs/tutorialsupport/ic_city.png" download>Download Icon</a></figcaption></figure>WhirlyGlobe-Maply uses [`Bitmap`](https://developer.android.com/reference/android/graphics/Bitmap.html) objects to draw markers on the map. The easiest way to create a `Bitmap` in your app is to add a PNG to your app's resources. It turns out that Google's [Material Design Icon](https://design.google.com/icons/#ic_location_city) library has many great icons that can be used for a screen marker. Usually these icons come in SVG and PNG formats, including several different resolutions. Depending on how large you want your icon to be, choose whatever PNG size makes sense. For our tutorial, we chose an `drawable-xxhdpi` icon called `ic_location_city_black_48dp.png`. This image is 144x144px. The actual pixel dimensions are how we specify icon size in the toolkit, so these are what you want to look at when considering your icon resolution.

To get a PNG into your application's assets as an `R.drawable`, you need to right click on your `res` directory in your project. explorer. Select a new __Image Asset__.

![New Image Asset](resources/image-asset.png)

The options to select are a bit counter-intuitive. Select in the top dropdown __Action Bar and Tab Icons__. The asset type needs to be __Image__. From there, you can select the path to the image you want to import.

![Configure Image Asset](resources/configure-image-asset.png)

### First Marker

In your [HelloMapFragment.java](https://github.com/mousebird/AndroidTutorialProject/blob/8156d82acd42ec2c238fbe06c5658bd4cc796f5b/app/src/main/java/io/theoutpost/helloearth/HelloMapFragment.java#L115), we are going to insert our markers right after we add our GeoJSON to the map. Create a new method called `insertMarkers`.

```java
@Override
protected void controlHasStarted() {
    ...
    // Insert Markers
    insertMarkers();
}
```

Adding your first marker is easy.

```java
private void insertMarkers() {
    MarkerInfo markerInfo = new MarkerInfo();
    Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.ic_city);
    Point2d markerSize = new Point2d(144, 144);

    // Moskow - Москва
    ScreenMarker moskow = new ScreenMarker();
    moskow.loc = Point2d.FromDegrees(37.616667, 55.75); // Longitude, Latitude
    moskow.image = icon;
    moskow.size = markerSize;

    mapControl.addScreenMarker(moskow, markerInfo, MaplyBaseController.ThreadMode.ThreadAny);
}
```

`MarkerInfo` is an options object used to configure special options for a marker, such as clustering and zoom restrictions. You specify the marker size with a `Point2d` object, and this dictates the (x, y) dimensions of the icon. In our example we gave it the exact width and height of the image supplied. However, you can change these parameters as you see fit, and the toolkit will scale the icon accordingly.

The basic properties of the object are set by assigning values to public members of the `ScreenMarker` object. Here we are setting the location, image, and marker size.

Finally, you can add an individual marker to your map controller with the `addScreenMarker` method.

![Marker for Moscow](resources/moscow.png)

### Add More Markers

To make a tutorial a little bit more interesting, we are going to add some more markers. Below you can see the code that adds to the map the five most populous cities in Russia.

```java
private void insertMarkers() {
    List<ScreenMarker> markers = new ArrayList<>();

    MarkerInfo markerInfo = new MarkerInfo();
    Bitmap icon = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.ic_city);
    Point2d markerSize = new Point2d(144, 144);

    // Moskow - Москва
    ScreenMarker moskow = new ScreenMarker();
    moskow.loc = Point2d.FromDegrees(37.616667, 55.75); // Longitude, Latitude
    moskow.image = icon;
    moskow.size = markerSize;
    markers.add(moskow);

    //  Saint Petersburg - Санкт-Петербург
    ScreenMarker stPetersburg = new ScreenMarker();
    stPetersburg.loc = Point2d.FromDegrees(30.3, 59.95);
    stPetersburg.image = icon;
    stPetersburg.size = markerSize;
    markers.add(stPetersburg);

    // Novosibirsk - Новосибирск
    ScreenMarker novosibirsk = new ScreenMarker();
    novosibirsk.loc = Point2d.FromDegrees(82.95, 55.05);
    novosibirsk.image = icon;
    novosibirsk.size = markerSize;
    markers.add(novosibirsk);

    // Yekaterinburg - Екатеринбург
    ScreenMarker yekaterinburg = new ScreenMarker();
    yekaterinburg.loc = Point2d.FromDegrees(60.583333, 56.833333);
    yekaterinburg.image = icon;
    yekaterinburg.size = markerSize;
    markers.add(yekaterinburg);

    // Nizhny Novgorod - Нижний Новгород
    ScreenMarker nizhnyNovgorod = new ScreenMarker();
    nizhnyNovgorod.loc = Point2d.FromDegrees(44.0075, 56.326944);
    nizhnyNovgorod.image = icon;
    nizhnyNovgorod.size = markerSize;
    nizhnyNovgorod.rotation = Math.PI;
    markers.add(nizhnyNovgorod);

    // Add your markers to the map controller.
    ComponentObject markersComponentObject = mapControl.addScreenMarkers(markers, markerInfo, MaplyBaseController.ThreadMode.ThreadAny);
}
```

Here we are adding all of our markers to an `ArrayList`. Then, we add this list to the map with `addScreenMarkers` instead of `addScreenMarker`.

![5 Cities](resources/5cities.png)

You might notice that the icon for Нижний Новгород is upside down. This is because we adjusted the `rotation` property of `ScreenMarker nizhnyNovgorod`. You can rotate a marker 360°. The parameter is in radians, so you can assign any value between 0 and 2π.

Lastly, `addScreenMarkers` returns a `ComponentObject` that you can keep track of. This object is your handle to the marker, vector, or other "component" object that you have added to your map controller. You can then later remove, enable, or disable your object in the controller.

```java
mapControl.removeObject(markersComponentObject, MaplyBaseController.ThreadMode.ThreadAny);
```

The [completed code](https://github.com/mousebird/AndroidTutorialProject/blob/8156d82acd42ec2c238fbe06c5658bd4cc796f5b/app/src/main/java/io/theoutpost/helloearth/HelloMapFragment.java) for this tutorial is on Github for your reference.
