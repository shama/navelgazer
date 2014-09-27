# navelgazer [![Build Status](http://img.shields.io/travis/shama/navelgazer.svg)](https://travis-ci.org/shama/navelgazer) [![Build status](https://ci.appveyor.com/api/projects/status/b7o6whgrdtwpyh6e/branch/master)](https://ci.appveyor.com/project/shama/navelgazer) [![gittip.com/shama](http://img.shields.io/gittip/shama.svg)](https://www.gittip.com/shama)

A platform layer for [gaze](https://github.com/shama/gaze). Watch files
consistently across Windows, OSX and Linux.

## About

**navelgazer** is a super fast, light weight and simple file watcher. It will only ever emit `change`, `delete` and `rename` events. This library is ideal if you're looking for a stable platform to build your own file watcher.

If you're looking for a more full featured file watching library, please use [gaze](https://github.com/shama/gaze) instead.

> Thank you [Bryce](https://github.com/brycebaril) for the naming this library.

## Usage

``` javascript
var watch = require('navelgazer');

watch(filepath, function(action, filepath) {
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

## Build

navelgazer comes with pre-built binaries for common architectures. If you would like to build the Node.js addon yourself, the following is required:

#### OSX

* Mac OS X >= 10.8
* Xcode >= 5.1
* Node.js x64

#### Windows

* Windows 2008 at least
* Visual Studio 2013
* Python 2.7
* Node.js ia32

#### Linux

* Node.js
* clang, development headers of GTK+ and libnotify

### Build CLI

Run the following command to force build the native addon instead of using the prebuilt binary:

``` shell
$ node node_modules/navelgazer/build.js --force
```

## Benchmarks

I encourage you to run and improve the benchmarks in [benchmarks/](https://github.com/shama/navelgazer/tree/master/benchmarks) (or write your own and test for yourself!)

**Please remember! These benchmarks are very academic. Each library has it's own advantages and will behave differently on different file systems. As well as changing with each version.**

Comparing: [fs.watch](http://nodejs.org/api/fs.html#fs_fs_watch_filename_options_listener), [chokidar](https://github.com/paulmillr/chokidar), [watch](https://github.com/mikeal/watch), [pathwatcher](https://github.com/atom/node-pathwatcher) (_I also tried [watchr](https://github.com/bevry/watchr) but had trouble getting it to respond_).

```shell
$ node benchmarks/change.js 

Benchmarking single change on darwin-x64-v8-3.14...
-------------------------------------------------------
pathwatcher@2.1.3:      8.28ms
fs.watch@v0.10.32:     66.35ms
chokidar@0.9.0:        61.06ms
navelgazer@1.0.0:       0.53ms
watch@0.11.0:       4,006.15ms
-------------------------------------------------------

$ tree test/fixtures/
test/fixtures/
├── one.js
└── renamed.js

0 directories, 2 files
```

> Important note: `watch` and some of these libraries watch file trees instead of single files which could partially be the reason for some of the time.

> Important note: `pathwatcher` is the library this one was forked from. It was forked as their use case is primarily to support file watching in the atom editor. Mine is to support gaze.

## Release History
* 1.0.0 - Initial release

## License
Copyright (c) 2014 Kyle Robinson Young  
Licensed under the MIT license.
