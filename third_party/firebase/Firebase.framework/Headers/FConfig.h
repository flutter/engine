/*
 * Firebase iOS Client Library
 *
 * Copyright © 2015 Firebase - All Rights Reserved
 * https://www.firebase.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binaryform must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY FIREBASE AS IS AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL FIREBASE BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>

/**
 * Configuration object for Firebase.  You can get the default FConfig object via
 * `[Firebase defaultConfig]` and modify it.  You must make all changes to it before
 * you create your first Firebase instance.
 */
@interface FConfig : NSObject

/**
 * By default the Firebase client will keep data in memory while your application is running, but not
 * when it is restarted. By setting this value to YES, the data will be persisted to on-device (disk)
 * storage and will thus be available again when the app is restarted (even when there is no network
 * connectivity at that time). Note that this property must be set before creating your first Firebase
 * reference and only needs to be called once per application.
 *
 * If your app uses Firebase Authentication, the client will automatically persist the user's authentication
 * token across restarts, even without persistence enabled. But if the auth token expired while offline and
 * you've enabled persistence, the client will pause write operations until you successfully re-authenticate
 * (or explicitly unauthenticate) to prevent your writes from being sent unauthenticated and failing due to
 * security rules.
 */
@property (nonatomic) BOOL persistenceEnabled;

/**
 * By default Firebase will use up to 10MB of disk space to cache data. If the cache grows beyond this size,
 * Firebase will start removing data that hasn't been recently used. If you find that your application caches too
 * little or too much data, call this method to change the cache size. This property must be set before creating
 * your first Firebase reference and only needs to be called once per application.
 *
 * Note that the specified cache size is only an approximation and the size on disk may temporarily exceed it
 * at times.
 */
@property (nonatomic) NSUInteger persistenceCacheSizeBytes;

/**
 * Sets the dispatch queue on which all events are raised. The default queue is the main queue.
 */
@property (nonatomic, strong) dispatch_queue_t callbackQueue;

@end
