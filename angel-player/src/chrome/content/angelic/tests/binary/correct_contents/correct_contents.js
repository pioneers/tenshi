var root = process.argv[2];
var binary = require ( root + '/binary.js' );
var misc = require ( root + '/misc.js' );
var buffer = require ( 'buffer' );

var b = new buffer.Buffer ( 1024 );

for ( var i = 0; i < b.length; i++ ) {
  b[i] = i & 0xff;
  }

binary.write_buffer ( b, 'correct_contents.bin', function ( err ) {
  misc.assert ( false, 'Should not receive error when writing file!' );
  } );
