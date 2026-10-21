// <location> on the web: navigator.geolocation, as an emscripten library defining the
// externs. The browser asks for permission on the first watch.
mergeInto(LibraryManager.library, {
  location_start: function(precise) {
    if (!navigator.geolocation) { Module.uiEmit('location', 'unavailable'); return; }
    if (Module.locationWatch != null) navigator.geolocation.clearWatch(Module.locationWatch);
    const known = value => (value == null || !Number.isFinite(value) ? -1 : value);
    Module.locationWatch = navigator.geolocation.watchPosition(position => {
      const c = position.coords;
      const altitude = c.altitude == null ? '-' : String(c.altitude);
      Module.uiEmit('location', `fix ${c.latitude} ${c.longitude} ${c.accuracy} ${altitude} ${known(c.speed)} ` +
        `${known(c.heading)} ${Math.round(position.timestamp)}`);
    }, error => {
      Module.uiEmit('location', error.code === error.PERMISSION_DENIED ? 'denied' : 'unavailable');
    }, {enableHighAccuracy: precise !== 0, maximumAge: 0});
  },
  location_stop: function() {
    if (Module.locationWatch != null && navigator.geolocation) navigator.geolocation.clearWatch(Module.locationWatch);
    Module.locationWatch = null;
  },
});
