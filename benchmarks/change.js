var path = require('path');
var fs = require('graceful-fs');

var fixture = path.resolve(__dirname, '..', 'test', 'fixtures', 'one.js');

var pkg = require('../package.json');
var arch = process.arch;
var platform = process.platform;
var v8 = /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];
console.log('\nBenchmarking single change on ' + platform + '-' + arch + '-v8-' + v8 + '...');
console.log('-------------------------------------------------------');

function diffToMs(diff) {
  return ((diff[0] * 1e9 + diff[1]) * 0.000001).toFixed(2).replace(/\d(?=(\d{3})+\.)/g, '$&,') + 'ms';
}
function writeReturnStart() {
  start = process.hrtime();
  fs.writeFile(fixture, 'var one = true;');
  return start;
}

function navelgazer(done) {
  var watch = require('../lib/navelgazer.js');
  var start = null;
  watch(fixture, function(err, action, filepath) {
    var diff = process.hrtime(start);
    watch.closeAll();
    done(diffToMs(diff));
  }, function() {
    start = writeReturnStart();
  });
}

function corefswatch(done) {
  var watch = require('fs').watch;
  var start = null;
  var watcher = watch(fixture, function(action, filename) {
    var diff = process.hrtime(start);
    watcher.close();
    done(diffToMs(diff));
  });
  start = writeReturnStart();
}

function chokidar(done) {
  var watch = require('chokidar').watch;
  var start = null;
  var watcher = watch(fixture);
  watcher.on('all', function(action, filename) {
    var diff = process.hrtime(start);
    watcher.close();
    done(diffToMs(diff));
  });
  start = writeReturnStart();
}

function watchr(done) {
  var watch = require('watchr').watch;
  var start = null;
  var watcher = watch({
    path: fixture,
    listeners: {
      change: function(action, filename) {
        var diff = process.hrtime(start);
        watcher.close();
        done(diffToMs(diff));
      },
    },
    next: function() {
      start = writeReturnStart();
    },
  });
}

function mikealwatch(done) {
  var watch = require('watch').createMonitor;
  var start = null;
  watch(path.dirname(fixture), function(watcher) {
    watcher.on('changed', function(f, curr, prev) {
      var diff = process.hrtime(start);
      watcher.stop();
      done(diffToMs(diff));
    });
    // Needs 1s before it actually starts tracking changes
    setTimeout(function() {
      start = writeReturnStart();
    }, 1000);
  });
}

function pathwatcher(done) {
  var watch = require('pathwatcher').watch;
  var start = null;
  watch(fixture, function(action, filename) {
    var diff = process.hrtime(start);
    require('pathwatcher').closeAllWatchers();
    done(diffToMs(diff));
  });
  start = writeReturnStart();
}

corefswatch(function(ms) {
  console.log('fs.watch@' + process.version + ':\t' + ms);
});
chokidar(function(ms) {
  console.log('chokidar@' + pkg.devDependencies['chokidar'] + ':\t\t' + ms);
  console.log('NOTE: chokidar throttles change events at 50ms to resolve stability issues');
});
// TODO: Not working for me on OSX atm
// watchr(function(ms) {
//   console.log('watchr@' + pkg.devDependencies['watchr'] + ':\t\t' + ms);
// });
mikealwatch(function(ms) {
  console.log('watch@' + pkg.devDependencies['watch'] + ':\t\t' + ms);
});
pathwatcher(function(ms) {
  console.log('pathwatcher@' + pkg.devDependencies['pathwatcher'] + ':\t' + ms);
});
navelgazer(function(ms) {
  console.log('navelgazer@' + pkg.version + ':\t' + ms);
});

// Needs to manually process.exit after awhile as pathwatcher keeps the process alive forever
setTimeout(process.exit, 8000);

process.on('exit', function() {
  console.log('-------------------------------------------------------\n');
  process.exit();
});
