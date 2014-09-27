var test = require('tape');
var watch = require('../lib/navelgazer.js');
var path = require('path');
var fs = require('graceful-fs');

var fixtures = path.resolve(__dirname, 'fixtures');

test('will detect change (native and statpoll)', function(t) {
  t.plan(20);
  var count = 0;
  var tick = null;
  var expected = path.resolve(fixtures, 'one.js');
  function writeFile() {
    fs.writeFileSync(expected, 'var one = true;');
    count++;
  }
  (function watchAndChange() {
    watch(expected, function(err, action, filepath) {
      t.equal(action, 'change', 'action should have been changed (mode is ' + watch.mode + ')');
      t.equal(expected, filepath, 'filepath should have matched expected (mode is ' + watch.mode + ')');
      watch.closeAll();
      clearInterval(tick);
      watch.mode = 'auto';
      if (count < 5) {
        watchAndChange();
      } else if (count < 10) {
        watch.mode = 'poll';
        tick = setInterval(watch.tick.bind(watch), 500);
        setTimeout(watchAndChange, 500);
      } else {
        t.end();
      }
    }, function() {
      if (watch.mode === 'poll') {
        setTimeout(writeFile, 500);
      } else {
        writeFile();
      }
    })
  }());
});

test('will detect delete (native and statpoll)', function(t) {
  t.plan(20);
  var count = 0;
  var tick = null;
  var expected = path.resolve(fixtures, 'added.js');

  function addFile(cb) {
    if (fs.existsSync(expected)) fs.unlinkSync(expected);
    fs.writeFile(expected, 'var added = true;', function(err) {
      process.nextTick(cb);
    });
  }

  function deleteFile() {
    fs.unlink(expected);
    count++;
  }

  (function watchAndDelete() {
    addFile(function() {
      watch(expected, function fileEvent(err, action, filepath) {
        t.equal(action, 'delete', 'action should have been delete (mode is ' + watch.mode + ')');
        t.equal(expected, filepath, 'filepath should have matched expected (mode is ' + watch.mode + ')');
        watch.closeAll();
        clearInterval(tick);
        watch.mode = 'auto';
        if (count < 5) {
          watchAndDelete();
        } else if (count < 10) {
          watch.mode = 'poll';
          tick = setInterval(watch.tick.bind(watch), 500);
          setTimeout(watchAndDelete, 500);
        } else {
          t.end();
        }
      }, function(err) {
        if (watch.mode === 'poll') {
          setTimeout(deleteFile, 500);
        } else {
          deleteFile();
        }
      });
    });
  }());
});

// TODO: Skip for linux atm, get rename support man!
if (process.platform !== 'linux') {
  test('will detect rename (only native for now)', function(t) {
    t.plan(15);
    var count = 0;
    var beforefile = path.resolve(fixtures, 'added.js');
    var expected = path.resolve(fixtures, 'renamed.js');

    function addFile(cb) {
      if (fs.existsSync(beforefile)) fs.unlinkSync(beforefile);
      if (fs.existsSync(expected)) fs.unlinkSync(expected);
      fs.writeFile(beforefile, 'var added = true;', function(err) {
        process.nextTick(cb);
      });
    }

    function renameFile() {
      fs.renameSync(beforefile, expected);
      count++;
    }

    (function watchAndRename() {
      addFile(function() {
        watch(beforefile, function(err, action, filepath, newPath) {
          t.equal(action, 'rename', 'action should have been rename');
          t.equal(beforefile, filepath, 'filepath should have equaled before renamed filepath');
          t.equal(expected, newPath, 'newPath should have equaled the expected new path');
          watch.closeAll();
          watch.mode = 'auto';
          if (count < 5) {
            watchAndRename();
          } else {
            t.end();
          }
        }, function() {
          renameFile();
        });
      });
    }());
  });
}
