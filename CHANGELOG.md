# Changelog

All notable changes to this package are documented in this file. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- `location_supported()`: whether the device can measure where it is, before `use_location`
  mounts. True on iOS and macOS, on Android with a location service, in a browser with
  `navigator.geolocation` and on Linux where GeoClue is installed; false on Windows.
- macOS: positions from CoreLocation, as on iOS.
- Linux: positions from GeoClue 2 over D-Bus.
- Windows: native code that answers `LocationUnavailable`, since Windows' location service is
  a WinRT API a MinGW-built app cannot reach.
- CI builds the example on the native desktop hosts (`tools/ci_desktop.sh`).

### Fixed

- Native desktop builds of an app using the package link (before, the externs were missing on
  macOS, Linux and Windows).
