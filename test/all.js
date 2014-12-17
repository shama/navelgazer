var path = require('path');
var tests = [
  'multi',
  'events',
  //'attrib',
];

var limit = process.argv[2];

if (limit) {
  require(path.resolve(process.cwd(), limit));
} else {
  tests.forEach(function(name) {
    require(path.resolve(__dirname, name + '.js'));
  });
}
