var test = require('tape');
var watch = require('../lib/navelgazer.js');
var path = require('path');
var fs = require('graceful-fs');
var touch = require('touch');

var fixtures = path.resolve(__dirname, 'fixtures');

// TODO: Ignoring this use case until Windows/OSX support confirmed
test('will detect change from modifying attributes', function(t) {
  t.plan(2);
  var expected = path.resolve(fixtures, 'one.js');
  watch(expected, function(err, action, filepath) {
    t.equal(action, 'change', 'action should have been changed when touched');
    t.equal(expected, filepath, 'filepath should have matched expected when touched');
    watch.closeAll();
    t.end();
  }, function() {
    touch(expected, {atime:true,mtime:true});
  })
});
