# Changelog

All notable changes to this package are documented in this file. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- `location_supported()`: whether the device can measure where it is, before `use_location`
  mounts. True on iOS, on Android with a location service and in a browser with
  `navigator.geolocation`; false on the desktop.
