var misc = require ( './misc.js' );

// Selects and proxies to the appropriate buffer implementation.

if ( misc.environment === 'xulrunner' ) {
  module.exports = require ( 'sdk/io/buffer.js' );
  }
else if ( misc.environment === 'node' ) {
  module.exports = require ( 'buffer' );
  }
else {
  module.exports = require ( '../vendor-js/buffer.js' );
  }
