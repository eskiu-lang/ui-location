// <location> on Windows: not yet. Windows' location service is a WinRT API
// (Windows.Devices.Geolocation), out of reach of a MinGW-built app, and the older Location API
// is retired. So every use is unavailable (LocationUnavailable): the app builds and links and
// can say so. It never reports a made-up position.
#include "ui_native.h"

void location_start(int precise) {
    (void)precise;
    ui_native_emit("location", "unavailable");
}

void location_stop(void) {}
