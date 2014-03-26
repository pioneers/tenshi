var misc = require ( './misc.js' );

// Selects and proxies to the appropriate fs implementation.

if ( misc.environment === 'xulrunner' ) {
  module.exports = require ( 'jetpack/sdk/io/fs.js' );
  }
else if ( misc.environment === 'node' ) {
  module.exports = require ( 'fs' );
  }
else {
  throw 'Unsupported environment!';
  }
