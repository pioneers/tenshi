var root = process.argv[2];

var buffer = require ( root + '/buffer.js' );
var misc = require ( root + '/misc.js' );
var factory = require ( root + '/factory.js' );

var fact = factory.make ( );
fact.set_target_type ( 'ARM' );
fact.load_type_file ( root + '/../common_defs/ngl_types.yaml' );

misc.assert ( fact.get_const ( 'NGL_PACKAGE_MAGIC' ) === 0x008fa5e5,
              'NGL_PACKAGE_MAGIC should be 0x008fa5e5.' );
