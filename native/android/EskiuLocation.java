// <location> on Android: LocationManager, after the runtime permission. location.c calls
// start and stop; positions and failures go to the "location" channel (src/location.esk).
package org.eskiu.location;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Looper;
import java.util.Locale;
import org.eskiu.ui.UiActivity;

public final class EskiuLocation {
    private static LocationListener listener;
    private static boolean running;

    public static void setup() {}

    public static void start(boolean precise) {
        Activity activity = UiActivity.current();
        if (activity == null) return;
        activity.runOnUiThread(() -> {
            running = true;
            String permission = precise ? Manifest.permission.ACCESS_FINE_LOCATION : Manifest.permission.ACCESS_COARSE_LOCATION;
            UiActivity.requestPermissions(new String[] {permission}, granted -> {
                if (!running) return;
                if (!granted) { UiActivity.emit("location", "denied"); return; }
                begin(activity, precise);
            });
        });
    }

    // The provider for the accuracy asked: GPS when precise and on, else the network's.
    @SuppressWarnings("MissingPermission")
    private static void begin(Activity activity, boolean precise) {
        LocationManager manager = (LocationManager)activity.getSystemService(Context.LOCATION_SERVICE);
        String provider = null;
        if (precise && manager.isProviderEnabled(LocationManager.GPS_PROVIDER)) provider = LocationManager.GPS_PROVIDER;
        else if (manager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) provider = LocationManager.NETWORK_PROVIDER;
        else if (manager.isProviderEnabled(LocationManager.GPS_PROVIDER)) provider = LocationManager.GPS_PROVIDER;
        if (provider == null) { UiActivity.emit("location", "unavailable"); return; }
        stopUpdates(activity);
        listener = new LocationListener() {
            @Override public void onLocationChanged(Location fix) { if (running) emit(fix); }
            @Override public void onStatusChanged(String name, int status, Bundle extras) {}
            @Override public void onProviderEnabled(String name) {}
            @Override public void onProviderDisabled(String name) { if (running) UiActivity.emit("location", "unavailable"); }
        };
        Location last = manager.getLastKnownLocation(provider);
        if (last != null) emit(last);
        manager.requestLocationUpdates(provider, 1000, 0, listener, Looper.getMainLooper());
    }

    private static void emit(Location fix) {
        String altitude = fix.hasAltitude() ? String.format(Locale.US, "%.2f", fix.getAltitude()) : "-";
        UiActivity.emit("location", String.format(Locale.US, "fix %.7f %.7f %.2f %s %.2f %.2f %d", fix.getLatitude(),
            fix.getLongitude(), fix.hasAccuracy() ? fix.getAccuracy() : 0f, altitude, fix.hasSpeed() ? fix.getSpeed() : -1f,
            fix.hasBearing() ? fix.getBearing() : -1f, fix.getTime()));
    }

    private static void stopUpdates(Activity activity) {
        if (listener == null) return;
        ((LocationManager)activity.getSystemService(Context.LOCATION_SERVICE)).removeUpdates(listener);
        listener = null;
    }

    public static void stop() {
        Activity activity = UiActivity.current();
        if (activity == null) return;
        activity.runOnUiThread(() -> {
            running = false;
            stopUpdates(activity);
        });
    }
}
