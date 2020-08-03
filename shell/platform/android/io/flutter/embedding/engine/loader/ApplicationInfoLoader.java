package io.flutter.embedding.engine.loader;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.XmlResourceParser;
import android.os.Bundle;
import android.security.NetworkSecurityPolicy;
import androidx.annotation.NonNull;
import java.io.IOException;
import org.json.JSONArray;
import org.xmlpull.v1.XmlPullParserException;

/** Loads application information. */
final class ApplicationInfoLoader {
  // XML Attribute keys supported in AndroidManifest.xml
  private static final String PUBLIC_AOT_SHARED_LIBRARY_NAME =
      FlutterLoader.class.getName() + '.' + FlutterLoader.AOT_SHARED_LIBRARY_NAME;
  private static final String PUBLIC_VM_SNAPSHOT_DATA_KEY =
      FlutterLoader.class.getName() + '.' + FlutterLoader.VM_SNAPSHOT_DATA_KEY;
  private static final String PUBLIC_ISOLATE_SNAPSHOT_DATA_KEY =
      FlutterLoader.class.getName() + '.' + FlutterLoader.ISOLATE_SNAPSHOT_DATA_KEY;
  private static final String PUBLIC_FLUTTER_ASSETS_DIR_KEY =
      FlutterLoader.class.getName() + '.' + FlutterLoader.FLUTTER_ASSETS_DIR_KEY;

  @NonNull
  private ApplicationInfo getApplicationInfo(@NonNull Context applicationContext) {
    try {
      return applicationContext
          .getPackageManager()
          .getApplicationInfo(applicationContext.getPackageName(), PackageManager.GET_META_DATA);
    } catch (PackageManager.NameNotFoundException e) {
      throw new RuntimeException(e);
    }
  }

  private String getString(Bundle metadata, String key) {
    if (metadata == null) {
      return null;
    }
    return metadata.getString(key, null);
  }

  private String getNetworkPolicy(ApplicationInfo appInfo, Context context) {
    // We cannot use reflection to look at networkSecurityConfigRes because
    // Android throws an error when we try to access fields marked as @hide.
    // Instead we rely on metadata.
    Bundle metadata = appInfo.metaData;
    if (metadata == null) {
      return null;
    }

    int networkSecurityConfigRes = metadata.getInt("network-policy", 0);
    if (networkSecurityConfigRes <= 0) {
      return null;
    }

    JSONArray output = new JSONArray();
    try {
      XmlResourceParser xrp = context.getResources().getXml(networkSecurityConfigRes);
      xrp.next();
      int eventType = xrp.getEventType();
      while (eventType != XmlResourceParser.END_DOCUMENT) {
        if (eventType == XmlResourceParser.START_TAG) {
          if (xrp.getName().equals("domain-config")) {
            parseDomainConfig(xrp, output, false);
          }
        }
        eventType = xrp.next();
      }
    } catch (IOException | XmlPullParserException e) {
      return null;
    }
    return output.toString();
  }

  private void parseDomainConfig(
      XmlResourceParser xrp, JSONArray output, boolean inheritedCleartextPermitted)
      throws IOException, XmlPullParserException {
    boolean cleartextTrafficPermitted =
        xrp.getAttributeBooleanValue(
            null, "cleartextTrafficPermitted", inheritedCleartextPermitted);
    while (true) {
      int eventType = xrp.next();
      if (eventType == XmlResourceParser.START_TAG) {
        if (xrp.getName().equals("domain")) {
          // There can be multiple domains.
          parseDomain(xrp, output, cleartextTrafficPermitted);
        } else if (xrp.getName().equals("domain-config")) {
          parseDomainConfig(xrp, output, cleartextTrafficPermitted);
        } else {
          throw new IllegalStateException("Unexpected tag " + xrp.getName());
        }
      } else if (eventType == XmlResourceParser.END_TAG) {
        break;
      }
    }
  }

  private void parseDomain(XmlResourceParser xrp, JSONArray output, boolean cleartextPermitted)
      throws IOException, XmlPullParserException {
    boolean includeSubDomains = xrp.getAttributeBooleanValue(null, "includeSubdomains", false);
    xrp.next();
    if (xrp.getEventType() != XmlResourceParser.TEXT) {
      throw new IllegalStateException("Expected text");
    }
    String domain = xrp.getText().trim();
    JSONArray outputArray = new JSONArray();
    outputArray.put(domain);
    outputArray.put(includeSubDomains);
    outputArray.put(cleartextPermitted);
    output.put(outputArray);
    xrp.next();
    if (xrp.getEventType() != XmlResourceParser.END_TAG) {
      throw new IllegalStateException("Expected end of domain tag");
    }
  }

  /**
   * Initialize our Flutter config values by obtaining them from the manifest XML file, falling back
   * to default values.
   */
  @NonNull
  public FlutterApplicationInfo initConfig(@NonNull Context applicationContext) {
    ApplicationInfo appInfo = getApplicationInfo(applicationContext);
    String networkPolicy = getNetworkPolicy(appInfo, applicationContext);
    System.err.println("mehmet: " + networkPolicy);
    // Prior to API 23, cleartext traffic is allowed.
    boolean preventInsecureSocketConnections = false;
    if (android.os.Build.VERSION.SDK_INT >= 23) {
      preventInsecureSocketConnections = !(NetworkSecurityPolicy.getInstance().isCleartextTrafficPermitted());
    }
    return new FlutterApplicationInfo(
        getString(appInfo.metaData, PUBLIC_AOT_SHARED_LIBRARY_NAME),
        getString(appInfo.metaData, PUBLIC_VM_SNAPSHOT_DATA_KEY),
        getString(appInfo.metaData, PUBLIC_ISOLATE_SNAPSHOT_DATA_KEY),
        getString(appInfo.metaData, PUBLIC_FLUTTER_ASSETS_DIR_KEY),
        networkPolicy,
        appInfo.nativeLibraryDir,
        preventInsecureSocketConnections);
  }
}

