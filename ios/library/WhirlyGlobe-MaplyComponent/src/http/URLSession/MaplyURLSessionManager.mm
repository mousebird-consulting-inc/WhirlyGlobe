//
//  MaplyURLSessionManager.mm
//  WhirlyGlobeMaplyComponent
//
//  Created by BACEM FATNASSI on 17/2/2022.
//  Copyright © 2022 mousebird consulting. All rights reserved.
//

#import "MaplyURLSessionManager.h"
#import "WhirlyKitLog.h"

static MaplyURLSessionManager * sharedManager = nil;

@interface MaplyURLSessionManager () <NSURLSessionDelegate>

@end

@implementation MaplyURLSessionManager
{
    NSURLSession *session;
    bool _allowCellularRequestsSet;
    bool _allowConstrainedRequestsSet;
    bool _allowExpensiveRequestsSet;
    bool _acceptCookiesSet;
    bool _usePipeliningSet;
    bool _defaultCachePolicySet;
    bool _waitForConnectivitySet;
    bool _prioritySet;
}

+ (instancetype)sharedManager
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (sharedManager == nil) {
            sharedManager = [[self alloc] init];
        }
    });
    return sharedManager;
}

- init
{
    if ((self = [super init]))
    {
        [self reset];
    }
    return self;
}

- (void)reset
{
    session = nil;
    _allowCellularRequests = true;
    _allowCellularRequestsSet = false;
    _allowConstrainedRequests = true;
    _allowConstrainedRequestsSet = false;
    _allowExpensiveRequests = true;
    _allowExpensiveRequestsSet = false;
    _additionalHeaders = nil;
    _acceptCookies = true;
    _acceptCookiesSet = false;
    _maxConnectionsPerHost = -1;
    _usePipelining = false;
    _usePipeliningSet = false;
    _defaultCachePolicy = NSURLRequestUseProtocolCachePolicy;
    _defaultCachePolicySet = false;
    _requestTimeout = -1;
    _resourceTimeout = -1;
    _waitForConnectivity = false;
    _waitForConnectivitySet = false;
    _priority = MaplyURLSessionPriorityDefault;
    _prioritySet = false;
    _logRequestMetrics = false;
}

- (void)setSessionManagerDelegate:(id<MaplyURLSessionManagerDelegate>)delegate {
    @synchronized(self) {
        if (delegate != _sessionManagerDelegate) {
            _sessionManagerDelegate = delegate;
            session = nil;
        }
    }
}
- (void)setAllowCellularRequests:(bool)value {
    @synchronized(self) {
        if (value != _allowCellularRequests) {
            _allowCellularRequests = value;
            _allowCellularRequestsSet = true;
            session = nil;
        }
    }
}
- (void)setAllowConstrainedRequests:(bool)value {
    @synchronized(self) {
        if (value != _allowConstrainedRequests) {
            _allowConstrainedRequests = value;
            _allowConstrainedRequestsSet = true;
            session = nil;
        }
    }
}
- (void)setAllowExpensiveRequests:(bool)value {
    @synchronized(self) {
        if (value != _allowExpensiveRequests) {
            _allowExpensiveRequests = value;
            _allowExpensiveRequestsSet = true;
            session = nil;
        }
    }
}
- (void)setAdditionalHeaders:(NSDictionary *)value {
    if (@available(iOS 13.0, *))
    @synchronized(self) {
        if (value != _additionalHeaders) {
            _additionalHeaders = value ? [NSDictionary dictionaryWithDictionary:value] : nil;
            session = nil;
        }
    }
}
- (void)setAcceptCookies:(bool)value {
    @synchronized(self) {
        if (value != _acceptCookies) {
            _acceptCookies = value;
            _acceptCookiesSet = true;
            session = nil;
        }
    }
}
- (void)setMaxConnectionsPerHost:(int)value {
    @synchronized(self) {
        if (value != _maxConnectionsPerHost) {
            _maxConnectionsPerHost = value;
            session = nil;
        }
    }
}
- (void)setUsePipelining:(bool)value {
    @synchronized(self) {
        if (value != _usePipelining) {
            _usePipelining = value;
            _usePipeliningSet = true;
            session = nil;
        }
    }
}
- (void)setDefaultCachePolicy:(NSURLRequestCachePolicy)value {
    @synchronized(self) {
        if (value != _defaultCachePolicy) {
            _defaultCachePolicy = value;
            _defaultCachePolicySet = true;
            session = nil;
        }
    }
}
- (void)setRequestTimeout:(NSTimeInterval)value {
    @synchronized(self) {
        if (value != _requestTimeout) {
            _requestTimeout = value;
            session = nil;
        }
    }
}
- (void)setResourceTimeout:(NSTimeInterval)value {
    @synchronized(self) {
        if (value != _resourceTimeout) {
            _resourceTimeout = value;
            session = nil;
        }
    }
}
- (void)setWaitForConnectivity:(bool)value {
    @synchronized(self) {
        if (value != _waitForConnectivity) {
            _waitForConnectivity = value;
            _waitForConnectivitySet = true;
            session = nil;
        }
    }
}
- (void)setPriority:(MaplyURLSessionPriority)value {
    @synchronized(self) {
        if (value != _priority) {
            _priority = value;
            _prioritySet = true;
            session = nil;
        }
    }
}


-(NSURLSession*)createURLSession
{
    @synchronized(self)
    {
        if (!session)
        {
            // If nothing is overridden, use the default session.  This shouldn't make
            // any difference but it seems to for some users in some network conditions.
            if (!_sessionManagerDelegate &&
                !_allowCellularRequestsSet &&
                !_allowConstrainedRequestsSet &&
                !_allowExpensiveRequestsSet &&
                !_additionalHeaders &&
                !_acceptCookiesSet &&
                !_usePipeliningSet &&
                !_defaultCachePolicySet &&
                !_waitForConnectivitySet &&
                !_prioritySet &&
                !_logRequestMetrics &&
                _requestTimeout <= 0 &&
                _resourceTimeout <= 0 &&
                _maxConnectionsPerHost <= 0)
            {
                wkLogLevel(Debug, "Using un-configured shared NSURLSession");
                session = [NSURLSession sharedSession];
                return session;
            }

            wkLogLevel(Debug, "Using configured NSURLSession");

            NSURLSessionConfiguration *config = [[NSURLSessionConfiguration defaultSessionConfiguration] mutableCopy];
            if (_allowCellularRequestsSet)
            {
                config.allowsCellularAccess = _allowCellularRequests;
            }
            if (_usePipeliningSet)
            {
                config.HTTPShouldUsePipelining = _usePipelining;
            }
            if (_defaultCachePolicySet)
            {
                config.requestCachePolicy = _defaultCachePolicy;
            }
            if (_waitForConnectivitySet)
            {
                config.waitsForConnectivity = _waitForConnectivity;
            }
            if (_acceptCookiesSet)
            {
                config.HTTPCookieAcceptPolicy = _acceptCookies ? NSHTTPCookieAcceptPolicyAlways : NSHTTPCookieAcceptPolicyNever;
            }

            if (_maxConnectionsPerHost > 0)
            {
                config.HTTPMaximumConnectionsPerHost = _maxConnectionsPerHost;
            }
            if (@available(iOS 13.0, *))
            {
                if (_allowExpensiveRequestsSet)
                {
                    config.allowsExpensiveNetworkAccess = _allowExpensiveRequests;
                }
                if (_allowConstrainedRequestsSet)
                {
                    config.allowsConstrainedNetworkAccess = _allowConstrainedRequests;
                }
            }
            if (_additionalHeaders)
            {
                config.HTTPAdditionalHeaders = _additionalHeaders;
            }
            if (_requestTimeout > 0)
            {
                config.timeoutIntervalForRequest = _requestTimeout;
            }
            if (_resourceTimeout > 0)
            {
                config.timeoutIntervalForResource = _resourceTimeout;
            }

            if (_prioritySet)
            {
                switch (_priority)
                {
                    default:
                    case MaplyURLSessionPriorityDefault: config.networkServiceType = NSURLNetworkServiceTypeDefault; break;
                    case MaplyURLSessionPriorityLow:     config.networkServiceType = NSURLNetworkServiceTypeBackground; break;
                    case MaplyURLSessionPriorityHigh:    config.networkServiceType = NSURLNetworkServiceTypeResponsiveData; break;
                }
            }

            session = [NSURLSession sessionWithConfiguration:config
                                                    delegate:(id <NSURLSessionDelegate>)self
                                               delegateQueue:NSOperationQueue.mainQueue];
        }
        return session;
    }
}

- (void)URLSession:(NSURLSession *)session
    didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
      completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential * _Nullable))completionHandler
{
    id <MaplyURLSessionManagerDelegate> delegate = _sessionManagerDelegate;
    if ([delegate respondsToSelector:@selector(didReceiveChallenge:completionHandler:)])
    {
        [delegate didReceiveChallenge:challenge completionHandler:completionHandler];
    }
    else
    {
        NSURLCredential *credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, credential);
    }
}

static bool isCacheHit(const NSURLResponse *response)
{
    if (@available(iOS 13.0, *))
    if ([response isKindOfClass:[NSHTTPURLResponse class]])
    if (NSString *cacheHeader = [((NSHTTPURLResponse *)response) valueForHTTPHeaderField:@"x-cache"])
    {
        // may report multiple levels of cache, look for a miss at any level
        return ([cacheHeader rangeOfString:@"hit " options:NSCaseInsensitiveSearch].length &&
               ![cacheHeader rangeOfString:@"miss " options:NSCaseInsensitiveSearch].length);
    }
    return false;
}

- (void)URLSession:(NSURLSession *)session
              task:(NSURLSessionTask *)task
    didFinishCollectingMetrics:(NSURLSessionTaskMetrics *)metrics
{
    if (!_logRequestMetrics || !metrics.transactionMetrics.count)
    {
        return;
    }

    const NSURLSessionTaskTransactionMetrics *first = metrics.transactionMetrics.firstObject;
    const NSURLSessionTaskTransactionMetrics *last = metrics.transactionMetrics.lastObject;

    const NSTimeInterval redirectTime = (metrics.transactionMetrics.count > 1) ? [last.fetchStartDate timeIntervalSinceDate:first.fetchStartDate] : 0;
    const NSTimeInterval dnsTime = [last.domainLookupEndDate timeIntervalSinceDate:last.domainLookupStartDate];
    const NSTimeInterval connectTime = [last.connectEndDate timeIntervalSinceDate:last.connectStartDate];
    const NSTimeInterval secureTime = [last.secureConnectionEndDate timeIntervalSinceDate:last.secureConnectionStartDate];
    const NSTimeInterval requestTime = [last.requestEndDate timeIntervalSinceDate:last.requestStartDate];
    const NSTimeInterval responseTime = [last.responseEndDate timeIntervalSinceDate:last.responseStartDate];
    const NSTimeInterval responseLatency = [last.responseStartDate timeIntervalSinceDate:last.requestEndDate];
    const NSTimeInterval totalTime = [last.responseEndDate timeIntervalSinceDate:first.fetchStartDate];

    // TODO: How can we get these to MaplyRemoteTileFetcherStats?
    // TODO: Can we tell if the task was delayed before starting, e.g., by max connections per host?
    const NSURLResponse *resp = last.response;
    const bool cacheHit = isCacheHit(resp);
    const char* url = task.originalRequest.URL.absoluteString.UTF8String;
    wkLog("URL Metrics: redir=%.4f (%d) dns=%.4f con=%.4f sec=%.4f req=%.4f"
          " rsp=%.4f lat=%.4f tot=%.4f tx=%.2fMiB rx=%.2fMiB cache=%s for %s",
          redirectTime, (int)metrics.redirectCount, dnsTime, connectTime, secureTime,
          requestTime, responseTime, responseLatency, totalTime,
          task.countOfBytesSent/1024.0/1024, task.countOfBytesReceived/1024.0/1024,
          cacheHit ? " (cache hit)" : "", url ? url : "(none)");
}

@end
