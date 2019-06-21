NS_ASSUME_NONNULL_BEGIN

@protocol FlutterBinaryMessenger;

@protocol FlutterBinaryMessengerContainer
- (NSObject<FlutterBinaryMessenger>*)binaryMessenger;
@end

NS_ASSUME_NONNULL_END
