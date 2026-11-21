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
| macOS | `CLLocationManager`, as on iOS; a Mac finds itself from the Wi-Fi networks around it | Location Services, asked on first use (`NSLocationUsageDescription`) |
| Linux | GeoClue 2 over D-Bus (GIO): a client for the app's id, exact or street-level accuracy | The desktop's agent asks the user, by the app's `.desktop` name; `LocationDenied` when GeoClue refuses |
| Windows | none yet: every use is `LocationUnavailable` | none |

The native code is in `native/`: `ios/location.m`, `android/EskiuLocation.java` with
`android/location.c` (the externs, through JNI), `web/location.js`, `macos/location.m`,
`linux/location.c` and `windows/location.c`.

On Linux the position comes only where GeoClue runs (most desktops have it; a minimal system
or a container may not, and then `use_location` says `LocationUnavailable`). GeoClue gives no
speed or heading for a position from Wi-Fi, so those are -1 there. Windows' location service
is a WinRT API (`Windows.Devices.Geolocation`), which an app built with MinGW-w64 cannot reach,
so the package answers `LocationUnavailable` there and never makes up a position. The
desktop's `--webview` builds use the web's `navigator.geolocation`.

## The example

`examples/where` shows the position, how precise it is and how far four cities are, with copy
and share:

```bash
cd examples/where
ui run ios        # or android, web, macos, linux
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
