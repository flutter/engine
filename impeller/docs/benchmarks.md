# Impeller Benchmarks

Here are some noteworthy benchmarks related to Impeller performance:

- **New Gallery** - Runs through the Flutter Gallery with a driver test.
  - Pixel 7 Pro
    - Skia vs Vulkan Impeller - frame raster time stats: [dashboard](https://flutter-flutter-perf.skia.org/e/?queries=device_type%3DPixel_7_Pro%26sub_result%3D90th_percentile_frame_rasterizer_time_millis%26sub_result%3D99th_percentile_frame_rasterizer_time_millis%26sub_result%3Daverage_frame_rasterizer_time_millis%26sub_result%3Dworst_frame_rasterizer_time_millis%26test%3Dnew_gallery__transition_perf%26test%3Dnew_gallery_impeller__transition_perf)
    - OpenGLES: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X080f13e1d6607d5ad3f4fe5c67e61538)
    - Vulkan vs OpenGLES - average rasterizer time: [dashboard](https://flutter-flutter-perf.skia.org/e/?queries=device_type%3DPixel_7_Pro%26sub_result%3Daverage_frame_rasterizer_time_millis%26test%3Dnew_gallery_impeller__transition_perf%26test%3Dnew_gallery_opengles_impeller__transition_perf)
  - Samsung S10
    - Vulkan: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X777844777514c7b34e736eadbc5dd002)
    - Skia vs Vulkan Impeller - 90th percentile frame rasterizer time: [dashboard](https://flutter-flutter-perf.skia.org/e/?begin=1707934850&end=1708021250&queries=device_type%3DSM-A025V%26sub_result%3D90th_percentile_frame_rasterizer_time_millis%26test%3Dnew_gallery__transition_perf%26test%3Dnew_gallery_impeller__transition_perf)
    - (deprecated) OpenGLES: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=Xeb13bfef4ef2947f899646422bbad8c6)
    - (deprecated) Vulkan vs OpenGLES - average rasterizer time: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=Xdfca283b38a86fc09129141792cf5a4b)
  - Moto G4 (OpenGLES): [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=Xaeae5aa39c9028be43e8a9ad40540bd8)
  - iPhone 11
    - Metal: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X9d52e54d0ac32151cc10feca61ea34cc)
    - Skia vs Metal Impeller - 90th percentile frame rasterizer time: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X836c18b955eb83a9102a4391672f37e0)

- **Animated Blur Backdrop Filter** - A driver test that scrolls to a screen and
  animates a Blur Backdrop filter to get progressively blurrier.  This covers a
  gap in the "New Gallery" tests we've seen in places like Wonderous.
  - Pixel 7 Pro
    - Vulkan: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X6d3dd43039c95ec80a8b3914cf386f48)
    - OpenGLES: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X48625544c02c75d050c4440405025d80)
    - Vulkan vs OpenGLES - Average rasterizer time: [dashboard](https://flutter-flutter-perf.skia.org/e/?queries=device_type%3DPixel_7_Pro%26sub_result%3Daverage_frame_rasterizer_time_millis%26test%3Danimated_blur_backdrop_filter_perf__timeline_summary%26test%3Danimated_blur_backdrop_filter_perf_opengles__timeline_summary)
  - Moto G4 (OpenGLES): [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X78023772ea9e94c81f37456a7fa7bf46)
  - iPhone 11 (Metal): [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X2f7504aba3db6aeff08cc896081ace55)

- **Animated Advanced Blend** - A driver test like the Animated Blur test, but
  is displaying a handful of advanced blurs since it represents a specific case
  exercised in Wonderous.
  - Samsung S10
    - Vulkan: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X4ad61cb8047db080bca0808550f0662f)
    - OpenGLES: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X0095f870c922720957aa4f6db5cefe76)
    - Vulkan vs OpenGLES - Average rasterizer time: [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X7ee143e5ef1da2f06950c5d281258377)
  - iPhone 11 (Metal): [dashboard](https://flutter-flutter-perf.skia.org/e/?keys=X65477f5b5026c0d5ee8fee79122427ab)
