package io.flutter.embedding.engine.loader;

/** */
final class FlutterApplicationInfo {
  private static final String DEFAULT_AOT_SHARED_LIBRARY_NAME = "libapp.so";
  private static final String DEFAULT_VM_SNAPSHOT_DATA = "vm_snapshot_data";
  private static final String DEFAULT_ISOLATE_SNAPSHOT_DATA = "isolate_snapshot_data";
  private static final String DEFAULT_FLUTTER_ASSETS_DIR = "flutter_assets";

  final String aotSharedLibraryName;
  final String vmSnapshotData;
  final String isolateSnapshotData;
  final String flutterAssetsDir;
  final String domainNetworkPolicy;
  final String nativeLibraryDir;
  final boolean preventInsecureSocketConnections;

  FlutterApplicationInfo(
      String aotSharedLibraryName,
      String vmSnapshotData,
      String isolateSnapshotData,
      String flutterAssetsDir,
      String domainNetworkPolicy,
      String nativeLibraryDir,
      boolean preventInsecureSocketConnections) {
    this.aotSharedLibraryName =
        aotSharedLibraryName == null ? DEFAULT_AOT_SHARED_LIBRARY_NAME : aotSharedLibraryName;
    this.vmSnapshotData =
        vmSnapshotData == null ? DEFAULT_VM_SNAPSHOT_DATA : vmSnapshotData;
    this.isolateSnapshotData =
        isolateSnapshotData == null ? DEFAULT_ISOLATE_SNAPSHOT_DATA : isolateSnapshotData;
    this.flutterAssetsDir =
        flutterAssetsDir == null ? DEFAULT_FLUTTER_ASSETS_DIR : flutterAssetsDir;
    this.nativeLibraryDir = nativeLibraryDir;
    // Can be null
    this.domainNetworkPolicy = domainNetworkPolicy;
    this.preventInsecureSocketConnections = preventInsecureSocketConnections;
  }
}

