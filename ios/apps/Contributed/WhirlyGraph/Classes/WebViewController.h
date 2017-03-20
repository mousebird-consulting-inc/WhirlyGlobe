/*
 
 File: WebViewController.h
 
 Abstract: Interface declaration for the WebViewController class.
 
 Version: <1.0>
 
 
 Copyright (C) 2009 Apple Inc. All Rights Reserved.
 
 */

#import <UIKit/UIKit.h>

@interface WebViewController : UIViewController 
{
	//  Web View attributes
	NSString *passStringURL;
	NSString *passStringTitle;
	
//	IBOutlet UIView *webView;
	IBOutlet UIWebView *webPageView;
	IBOutlet UINavigationItem *barTitle;
	IBOutlet UIActivityIndicatorView *activityIndicator;
	
	BOOL playSound;
		
}
//Display WebPage

@property (nonatomic, retain) UIWebView *webPageView;
@property (nonatomic, retain) UINavigationItem *barTitle;
@property (nonatomic, retain) UIActivityIndicatorView *activityIndicator;

@property (nonatomic, retain) IBOutlet NSString *passStringURL;
@property (nonatomic, retain) IBOutlet NSString *passStringTitle;

-(IBAction) goHome:(id) sender;
-(void)displayWebPage:(NSString *)urlAddress;
-(IBAction)btnReturn:(id)sender;

-(void)playClick;
@end

