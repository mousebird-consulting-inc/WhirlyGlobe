---
---
(function ($) {
  var $pairs = [
    { text: ' UIViewController', link: 'https://developer.apple.com/library/ios/documentation/Uikit/reference/UIViewController_Class/index.html' },
    { text: ' UIView', link: 'https://developer.apple.com/library/ios/documentation/Uikit/reference/UIView_Class/index.html' },
    { text: ' NSURLRequest', link: 'https://developer.apple.com/library/Mac/documentation/Cocoa/Reference/Foundation/Classes/NSURLRequest_Class/index.html' },
    { text: ' WhirlyGlobeViewController', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/WhirlyGlobeViewController.html' },
    { text: ' MaplyViewController', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyViewController.html' },
    { text: ' WhirlyGlobeViewControllerDelegate', link: '{{ site.baseurl }}/reference/ios_2_4/Protocols/WhirlyGlobeViewControllerDelegate.html' },
    { text: ' MaplyViewControllerDelegate', link: '{{ site.baseurl }}/reference/ios_2_4/Protocols/MaplyViewControllerDelegate.html' },
    { text: ' MaplyScreenMarker', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyScreenMarker.html' },
    { text: ' MaplyScreenLabel', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyScreenLabel.html' },
    { text: ' MaplyAnnotation', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyAnnotation.html' },
    { text: ' MaplyBaseViewController', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyBaseViewController.html' },
    { text: ' MaplyComponentObject', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyComponentObject.html' },
    { text: ' MaplyMBTileSource', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyMBTileSource.html' },
    { text: ' MaplyRemoteTileSource', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyRemoteTileSource.html' },
    { text: ' MaplyQuadImageTilesLayer', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyQuadImageTilesLayer.html' },
    { text: ' MaplyQuadPagingLayer', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyQuadPagingLayer.html' },
    { text: ' MaplyPagingDelegate', link: '{{ site.baseurl }}/reference/ios_2_4/Protocols/MaplyPagingDelegate.html' },
    { text: ' addScreenLabels', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyBaseViewController.html#//api/name/addScreenLabels:desc:' },
    { text: ' MaplyVectorObject', link: '{{ site.baseurl }}/reference/ios_2_4/Classes/MaplyVectorObject.html' }
  ];
  $.each($pairs, function (i) {
    $(".tutorial-main p:contains('" + $pairs[i].text + "')").each(function() {
      var $this = $(this);
      $this.html(function() {
        return $this.html().replace($pairs[i].text, '<a target="_blank" href="' + $pairs[i].link + '">' + $pairs[i].text + '</a>');
      })
    })
  })
})(jQuery);
