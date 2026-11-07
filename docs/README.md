# ui-location documentation

Guides and reference for ui-location. The [README](../README.md) has the overview.

<!-- pages -->
- [API reference](api.md): every public function and type
<!-- /pages -->

## Platforms

| Platform | Service | Permission |
|---|---|---|
| iOS | `CLLocationManager`, best or hundred-meter accuracy | When in use, asked on first use |
| Android | `LocationManager`: GPS when precise and on, else the network's | `ACCESS_FINE_LOCATION` or `ACCESS_COARSE_LOCATION` at run time |
| Web | `navigator.geolocation.watchPosition` | The browser's prompt; it needs `https://` or `localhost` |

The native code is in `native/`: `ios/location.m`, `android/EskiuLocation.java` with
`android/location.c` (the externs, through JNI), and `web/location.js`.

## The example

`examples/where` shows the position, how precise it is and how far four cities are, with copy
and share:

```bash
cd examples/where
ui run ios        # or android, web
```

On a simulator or emulator, set a position:

```bash
xcrun simctl location booted set 19.4326,-99.1332
adb emu geo fix -99.1332 19.4326
```

## Tests

`eskiuc test` runs `tests/location.esk` on this computer, with stand-ins for the platform's
side that count their calls and the events a platform would send: one service for two
components, every field of a position, unknown altitude and speed, a denied permission, the
stop when the last component unmounts, and the distance from Paris to London.
