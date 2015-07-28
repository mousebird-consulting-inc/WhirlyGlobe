---
---
(function ($) {
  var $pairs = [
    { text: ' UIViewController', link: 'https://developer.apple.com/library/ios/documentation/Uikit/reference/UIViewController_Class/index.html' },
    { text: ' UIView', link: 'https://developer.apple.com/library/ios/documentation/Uikit/reference/UIView_Class/index.html' },
    { text: ' NSURLRequest', link: 'https://developer.apple.com/library/Mac/documentation/Cocoa/Reference/Foundation/Classes/NSURLRequest_Class/index.html' },
    { text: ' WhirlyGlobeViewController', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/WhirlyGlobeViewController.html' },
    { text: ' MaplyViewController', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyViewController.html' },
    { text: ' WhirlyGlobeViewControllerDelegate', link: '{{ site.baseurl }}/reference/ios_2_3/Protocols/WhirlyGlobeViewControllerDelegate.html' },
    { text: ' MaplyViewControllerDelegate', link: '{{ site.baseurl }}/reference/ios_2_3/Protocols/MaplyViewControllerDelegate.html' },
    { text: ' MaplyScreenMarker', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyScreenMarker.html' },
    { text: ' MaplyScreenLabel', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyScreenLabel.html' },
    { text: ' MaplyAnnotation', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyAnnotation.html' },
    { text: ' MaplyBaseViewController', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyBaseViewController.html' },
    { text: ' MaplyComponentObject', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyComponentObject.html' },
    { text: ' MaplyMBTileSource', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyMBTileSource.html' },
    { text: ' MaplyRemoteTileSource', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyRemoteTileSource.html' },
    { text: ' MaplyQuadImageTilesLayer', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyQuadImageTilesLayer.html' },
    { text: ' MaplyQuadPagingLayer', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyQuadPagingLayer.html' },
    { text: ' MaplyPagingDelegate', link: '{{ site.baseurl }}/reference/ios_2_3/Protocols/MaplyPagingDelegate.html' },
    { text: ' addScreenLabels', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyBaseViewController.html#//api/name/addScreenLabels:desc:' },
    { text: ' MaplyVectorObject', link: '{{ site.baseurl }}/reference/ios_2_3/Classes/MaplyVectorObject.html' }
  ];

  $.each($pairs, function (i) {
    $(".tutorial-main p:contains('" + $pairs[i].text + "')").each(function() {
      var $this = $(this);
      $this.html(function() {
        return $this.html().replace($pairs[i].text, '<a target="_blank" href="' + $pairs[i].link + '">' + $pairs[i].text + '</a>');
      })
    })
  });

})(jQuery);

(function ($) {

  var $multipleCode = $('.multiple_code');
  var $multipleCodeHeader = $('.multiple_code_header');

  var parse_langs = {
    'objc': 'Objective-C',
    'java': 'Java',
    'swift': 'Swift'
  };

  var bindClickLink = function($link, $code, $codeLinks, $codeItems){
    $link.on('click', function(e){
      e.preventDefault();

      var $link = $(this);

      $codeItems.hide();
      $code.show();

      $codeLinks.removeClass('selected');
      $link.addClass('selected');

    });

  };

  var populateLink = function($link, $code){
    var originalText = $code.find('[data-lang]').attr('data-lang');
    var parsedText = (parse_langs[originalText]) ? parse_langs[originalText] : originalText;
    $link.text(parsedText);
  };

  var initLinks = function($codeLinks, $codeItems){

    $.each($codeLinks, function(){

      var $link = $(this);
      var index = $link.attr('data-index');
      var $code = $codeItems.filter('li[data-index="' + index + '"]');

      populateLink($link, $code);
      bindClickLink($link, $code, $codeLinks, $codeItems);

    });

  };

  var initMultipleCode = function(){

    var $el = $(this);
    var $codeItems = $el.find('li');
    var $codeLinks = $el.prev('.multiple_code_header').find('a');

    $codeItems.hide();
    $codeItems.eq(0).show();
    $codeLinks.eq(0).addClass('selected');

    initLinks($codeLinks, $codeItems);

  };

  $.each($multipleCode, initMultipleCode);

  // $multipleCodeHeader.find('a')
  //   .each(populateLinks)
  //   .on('click', clickOnMultipleCodeLink);

})(jQuery);
