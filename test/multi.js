var test = require('tape');
var watch = require('../lib/navelgazer.js');
var path = require('path');
var fs = require('graceful-fs');

var fixtures = path.resolve(__dirname, 'fixtures');

test('will fire on multiple times for the same file', function(t) {
  var times = 10;
  t.plan(times * 2);
  var expected = path.resolve(fixtures, 'one.js');
  var count = 0;
  function step(err, action, filepath) {
    count++;
    if (count === times) {
      fs.writeFileSync(expected, 'var one = true;');
    } else if (count > times) {
      var num = (count - times);
      t.equal(action, 'change', 'got change action for watcher #' + num);
      t.equal(filepath, expected, 'triggered on correct file for watcher #' + num);
      if (count == (times * 2)) {
        watch.closeAll();
        t.end();
      }
    }
  }
  for (var i = 0; i < times; i++) {
    watch(expected, step, step);
  }
});
