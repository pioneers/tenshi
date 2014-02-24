var root = process.argv[2];
var binary = require ( root + '/binary.js' );
var misc = require ( root + '/misc.js' );
var buffer = require ( 'buffer' );

var b = new buffer.Buffer ( 0 );

binary.write_buffer ( b, 'creates.bin', function ( err ) {
  misc.assert ( false, 'Should not receive error when writing file!' );
  } );
