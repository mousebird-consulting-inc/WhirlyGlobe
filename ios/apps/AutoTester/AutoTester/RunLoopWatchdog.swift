//
//  RunLoopWatchdog.swift
//  AutoTester
//
//  Created by Tim Sylvester on 1/12/23.
//  Copyright © 2023 mousebird consulting. All rights reserved.
//

import Foundation


final public class Watchdog: NSObject, WatchdogRunLoopObserverDelegate {
    @objc
    public static let shared = Watchdog()

    override private init() {
        super.init()
    }

    deinit {
        stop()
    }

    public func start() {
        if !isStarted,
           let obs = WatchdogRunLoopObserver(runLoop: CFRunLoopGetMain(),
                                             stallingThreshold: 1.0) {
            observer = obs
            
            obs.delegate = self
            obs.start()

            print("[Watchdog] started")
            isStarted = true
        }
    }

    public func stop() {
        print("[Watchdog] stopped")
        observer?.stop()
        observer = nil
        isStarted = false
    }

    // MARK: WatchdogRunLoopObserverDelegate

    public func runLoopDidStall(withDuration duration: TimeInterval) {
        // TODO: implement your custom logging here
        //    - what task is currently running?
        //    - which view controller is currently on screen?
        print("⚠️ [Watchdog] main thread blocked for \(duration) seconds")
    }

    
    private var observer: WatchdogRunLoopObserver? = nil
    private var isStarted = false
}

