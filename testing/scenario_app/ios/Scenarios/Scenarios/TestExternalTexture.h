//
//  TestExternalTexture.h
//  Scenarios
//
//  Created by lujunchen on 2019/8/30.
//  Copyright © 2019 flutter. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Flutter/Flutter.h>

NS_ASSUME_NONNULL_BEGIN

@interface TestExternalTexture : NSObject<FlutterShareTexture>
- (instancetype)initWithWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar;
- (void)startWithID:(int64_t)textureID;
@end

NS_ASSUME_NONNULL_END
