# navelgazer [![Build Status](http://img.shields.io/travis/shama/navelgazer.svg)](https://travis-ci.org/shama/navelgazer) [![Build status](https://ci.appveyor.com/api/projects/status/b7o6whgrdtwpyh6e/branch/master)](https://ci.appveyor.com/project/shama/navelgazer) [![gittip.com/shama](http://img.shields.io/gittip/shama.svg)](https://www.gittip.com/shama)

A platform layer for [gaze](https://github.com/shama/gaze). Watch files
consistently across Windows, OSX and Linux.

## About

**navelgazer** is a super fast, light weight and simple file watcher. It will only ever emit `change`, `delete` and `rename` events. This library is ideal if you're looking for a stable platform to build your own file watcher.

If you're looking for a more full featured file watching library, please use [gaze](https://github.com/shama/gaze) instead.

[![NPM](https://nodei.co/npm/navelgazer.png?downloads=true)](https://nodei.co/npm/navelgazer/)

## Usage

``` javascript
var watch = require('navelgazer');

watch(filepath, function(err, action, filepath) {
  // Called when file is changed
}, function(err, watcher) {
  // Watcher is ready
});
```

## API

#### `navelgazer(filepath, onEventCallback, watcherIsReadyCallback)`

Specify a `filepath` and `watcherIsReadyCallback(err, watcher)` will be called when the watcher is ready for events. `watcherIsReadyCallback(err, event, filepath, newFilePath)` will be called for each event detected on that file.

Events emitted are either `change`, `delete`, or `rename`.

#### `navelgazer.mode`

* `auto` will default to native events and fallback to stat polling if `EMFILE` is hit.
* `watch` will only use native events and just throw `EMFILE` errors if the limit is hit.
* `poll` will only use stat polling.

#### `navelgazer.closeAll()`

Closes all native and stat polled watchers.

#### `navelgazer.close(filepath, whenClosedCallback)`

Closes a single file watcher by the given `filepath` and calls `whenClosedCallback()` when it is closed.

#### `navelgazer.getWatchedPaths()`

Returns the file paths of every file watch by either native or stat polling.

#### `navelgazer.tick`

Tick incrementer for stat poll. If stat polling, please supply an interval at which to check the files. For example:

```js
setInterval(navelgazer.tick.bind(navelgazer), 500);
```

An interval of `>=500ms` is recommended as stat polling is taxing on the file system.

## Building

Please see the [Building page in the wiki](https://github.com/shama/navelgazer/wiki/Building).

## Benchmarks

Please see the [Benchmarks page in the wiki](https://github.com/shama/navelgazer/wiki/Benchmarks).

## Release History
* 1.1.0 - Updates for node v0.11 and io.js 1.0. Add pre-built binaries and many other minor fixes.
* 1.0.0 - Initial release

## License
Copyright (c) 2015 Kyle Robinson Young  
Licensed under the MIT license.

> Thank you [Bryce](https://github.com/brycebaril) for the naming this library.
