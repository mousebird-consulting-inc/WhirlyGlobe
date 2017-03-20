/*
 
 File: WebViewController.m
 
 Abstract: Manages the Web view. 
 Launches an email composition interface inside the application if the iPhone OS is 3.0 or greater. 
 Launches the Mail application on the device if the iPhone OS version is lower than 3.0. 
 
 Version: <1.0>
 
 
 */

#import "WebViewController.h"
#import <AudioToolbox/AudioToolbox.h>

@implementation WebViewController

WebViewController *webViewController;

@synthesize webPageView, barTitle, activityIndicator;
@synthesize passStringURL, passStringTitle;


-(void)viewDidLoad {
	
    playSound = NO;
//	playSound = YES;	// first declare and set to YES;
	
	NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
	[userDefaults synchronize];
	
	int soundPreference = 0;
	soundPreference = [userDefaults integerForKey:@"sound_preference"]; // get actual setting
	
	if (soundPreference == 0) {
		playSound = YES;
	}	else if (soundPreference == 1) {
		playSound = NO;
	}
	
    
    if (passStringTitle == @"About Us") {
        NSLog (@"passStringURL %@",passStringURL);
        [webPageView loadRequest:[NSURLRequest requestWithURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:passStringURL ofType:@"html"]isDirectory:NO]]];
        
    } else {
        barTitle.title = passStringTitle;
        [webPageView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:passStringURL]]];
    }
    
    [passStringURL release];
    [passStringTitle release];


//	[self displayWebPage:passStringURL];

//Declare activity Indicator
	activityIndicator.hidesWhenStopped = YES;
	[activityIndicator startAnimating];	
	
//	[webPageView loadHTMLString:@"html/index.html" baseURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]]];
	
//	[webPageView loadHTMLString:passStringURL baseURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]]];
	 
}



-(void)displayWebPage:(NSString *)urlAddress
{
	//Declare activity Indicator
	activityIndicator.hidesWhenStopped = YES;
	[activityIndicator startAnimating];	
	
	
	//Create a URL object.
	NSURL *url = [NSURL URLWithString:urlAddress];
	
	//URL Requst Object
	NSURLRequest *requestObj = [NSURLRequest requestWithURL:url];
    	
	//Load the request in the UIWebView.
	[webPageView loadRequest:requestObj];
	
}

- (void)webViewDidFinishLoad:(UIWebView *)webPageView {
	[activityIndicator stopAnimating];  
}

- (void)webViewDidStartLoad:(UIWebView *)webPageView {     
	[activityIndicator startAnimating];     
}

-(IBAction) goHome:(id) sender
{

	[webPageView loadRequest:[NSURLRequest requestWithURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:passStringURL ofType:@"html"]isDirectory:NO]]];
	
//	[webPageView loadHTMLString:passStringURL baseURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]]];

//	[self displayWebPage:passStringURL];
}

-(IBAction) btnReturn:(id) sender {
	if (playSound == YES) {
		[self playClick];
	}

	[self dismissModalViewControllerAnimated:YES];
		
}


// click sound
-(void)playClick {		
//	SystemSoundID soundID;
//	AudioServicesCreateSystemSoundID((CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"click" ofType:@"wav"]], &soundID);
//	AudioServicesPlaySystemSound(soundID);
	
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	// Return YES for supported orientations
	//	return (interfaceOrientation == UIInterfaceOrientationPortrait);
	
	return YES;
}


#pragma mark -
#pragma mark Compose Mail

#pragma mark -
#pragma mark Unload views

- (void)viewDidUnload 
{
	self.webPageView = nil;
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}

#pragma mark -
#pragma mark Memory management

- (void)dealloc 
{
	[webPageView release];
	[super dealloc];
}

@end
