// <location> on Linux: GeoClue 2 (org.freedesktop.GeoClue2), the desktop's location service,
// over D-Bus with GIO, which GTK already links. location_start gets a client for the app's id
// (its .desktop file's name: the desktop's agent asks the user once, by that name), sets the
// accuracy and starts it; each LocationUpdated becomes a "fix" on the "location" channel.
// "denied" when GeoClue or its agent refuses the app, "unavailable" when there is no GeoClue
// (or no session and system bus to reach it).
#include <float.h>
#include <stdio.h>
#include <string.h>
#include "ui_native.h"

static const char *kName = "org.freedesktop.GeoClue2";

static GDBusConnection *bus;
static char *client;         // the client's object path, once GeoClue gave one
static int running, precise;
static guint generation;     // a start or stop makes the answers of earlier calls stale

static void emitFailure(GError *error) {
    int denied = error && (g_dbus_error_is_remote_error(error) &&
                           strstr(g_dbus_error_get_remote_error(error), "AccessDenied") != NULL);
    ui_native_emit("location", denied ? "denied" : "unavailable");
}

static double property(GVariant *properties, const char *name, double unknown) {
    double value = unknown;
    if (!g_variant_lookup(properties, name, "d", &value)) return unknown;
    return value;
}

// A Location object's properties, read at once, as a fix.
static void readLocation(GObject *source, GAsyncResult *result, gpointer data) {
    guint asked = GPOINTER_TO_UINT(data);
    GVariant *reply = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source), result, NULL);
    if (!reply) return;
    if (running && asked == generation) {
        GVariant *properties = NULL;
        g_variant_get(reply, "(@a{sv})", &properties);
        double altitude = property(properties, "Altitude", -DBL_MAX);
        double speed = property(properties, "Speed", -1), heading = property(properties, "Heading", -1);
        guint64 seconds = 0, micros = 0;
        GVariant *stamp = g_variant_lookup_value(properties, "Timestamp", G_VARIANT_TYPE("(tt)"));
        if (stamp) {
            g_variant_get(stamp, "(tt)", &seconds, &micros);
            g_variant_unref(stamp);
        }
        char height[32];
        if (altitude > -DBL_MAX) snprintf(height, sizeof height, "%.2f", altitude);
        else snprintf(height, sizeof height, "-");
        char payload[256];
        snprintf(payload, sizeof payload, "fix %.7f %.7f %.2f %s %.2f %.2f %lld", property(properties, "Latitude", 0),
                 property(properties, "Longitude", 0), property(properties, "Accuracy", 0), height,
                 speed >= 0 ? speed : -1.0, heading >= 0 ? heading : -1.0,
                 (long long)(seconds * 1000 + micros / 1000));
        ui_native_emit("location", payload);
        g_variant_unref(properties);
    }
    g_variant_unref(reply);
}

static void onUpdated(GDBusConnection *connection, const char *sender, const char *path, const char *interface,
                      const char *signal, GVariant *parameters, gpointer data) {
    (void)connection; (void)sender; (void)path; (void)interface; (void)signal; (void)data;
    const char *before = NULL, *now = NULL;
    g_variant_get(parameters, "(&o&o)", &before, &now);
    if (!running || !now) return;
    g_dbus_connection_call(bus, kName, now, "org.freedesktop.DBus.Properties", "GetAll",
                           g_variant_new("(s)", "org.freedesktop.GeoClue2.Location"), G_VARIANT_TYPE("(a{sv})"),
                           G_DBUS_CALL_FLAGS_NONE, 5000, NULL, readLocation, GUINT_TO_POINTER(generation));
}

static void started(GObject *source, GAsyncResult *result, gpointer data) {
    GError *error = NULL;
    GVariant *reply = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source), result, &error);
    if (reply) g_variant_unref(reply);
    else if (running && GPOINTER_TO_UINT(data) == generation) emitFailure(error);
    if (error) g_error_free(error);
}

static void setProperty(const char *name, GVariant *value) {
    g_dbus_connection_call(bus, kName, client, "org.freedesktop.DBus.Properties", "Set",
                           g_variant_new("(ssv)", "org.freedesktop.GeoClue2.Client", name, value), NULL,
                           G_DBUS_CALL_FLAGS_NONE, 5000, NULL, NULL, NULL);
}

// Start the client: its id, the accuracy (8 exact, 5 street level), then Start, in order on
// the bus.
static void startClient(void) {
    const char *app = g_get_prgname();
    setProperty("DesktopId", g_variant_new_string(app ? app : "org.eskiu.app"));
    setProperty("RequestedAccuracyLevel", g_variant_new_uint32(precise ? 8 : 5));
    g_dbus_connection_call(bus, kName, client, "org.freedesktop.GeoClue2.Client", "Start", NULL, NULL,
                           G_DBUS_CALL_FLAGS_NONE, 30000, NULL, started, GUINT_TO_POINTER(generation));
}

static void gotClient(GObject *source, GAsyncResult *result, gpointer data) {
    GError *error = NULL;
    GVariant *reply = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source), result, &error);
    if (!reply) {
        if (running && GPOINTER_TO_UINT(data) == generation) emitFailure(error);
        g_error_free(error);
        return;
    }
    const char *path = NULL;
    g_variant_get(reply, "(&o)", &path);
    client = g_strdup(path);
    g_variant_unref(reply);
    g_dbus_connection_signal_subscribe(bus, kName, "org.freedesktop.GeoClue2.Client", "LocationUpdated",
                                                 client, NULL, G_DBUS_SIGNAL_FLAGS_NONE, onUpdated, NULL, NULL);
    if (running && GPOINTER_TO_UINT(data) == generation) startClient();
}

void location_start(int wanted) {
    running = 1;
    precise = wanted;
    generation++;
    if (!bus) bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if (!bus) {
        ui_native_emit("location", "unavailable");
        return;
    }
    if (client) {
        startClient();
        return;
    }
    g_dbus_connection_call(bus, kName, "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.GeoClue2.Manager",
                           "GetClient", NULL, G_VARIANT_TYPE("(o)"), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, gotClient,
                           GUINT_TO_POINTER(generation));
}

void location_stop(void) {
    running = 0;
    generation++;
    if (!bus || !client) return;
    g_dbus_connection_call(bus, kName, client, "org.freedesktop.GeoClue2.Client", "Stop", NULL, NULL,
                           G_DBUS_CALL_FLAGS_NONE, 5000, NULL, NULL, NULL);
}
