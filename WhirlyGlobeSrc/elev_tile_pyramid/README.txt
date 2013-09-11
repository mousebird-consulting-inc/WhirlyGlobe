Build elev_tile_pyramid
----
Here's how to build elev_tile_pyramid.

First, you'll need the GDAL libraries as a framework.  Get them here:
http://www.kyngchaos.com/software:frameworks

Then load up the xcode project and build.  It should build cleanly as long as you've pulled in the various submodules.

You'll also want the GDAL utilities, so put the associated bin in your path.

Example Database
---
The example database is the whole world at a fairly low resolution and the Pacific Northwest at a higher resolution.

First, you've need source data.  Go to this web page http://www.viewfinderpanoramas.org/dem3.html
Get all the 15" data from here: http://www.viewfinderpanoramas.org/Coverage%20map%20viewfinderpanoramas_org15.htm

Once you've got them all in one place, run the following:
gdal_merge.py -o whole_world.tif 15-?.tif

Now you can run off the multires database.  It takes two steps.

elev_tile_pyramid -targetdb pacnw.sqlite -ps 32 32 -levels 9 -t_srs EPSG:3857 -te -20037508.34 -20037508.34 20037508.34 20037508.34 whole_earth.tif
elev_tile_pyramid -updatedb pacnw.sqlite -ue 9 13  -13803616.8583659 5160979.44404978 -13024380.422813 6274861.39400658 whole_earth.tif

Assuming that succeeded, you can copy the pacnw.sqlite file into your project and then load it with an ElevationDatabase in WhirlyGlobe-Maply.
