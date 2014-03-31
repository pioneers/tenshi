var root = process.argv[2];

var buffer = require ( root + '/buffer.js' );
var misc = require ( root + '/misc.js' );
var factory = require ( root + '/factory.js' );

var fact = factory.make ( );
fact.set_target_type ( 'ARM' );
fact.load_type_file ( root + '/../common_defs/ngl_types.yaml' );

misc.assert ( fact.get_size ( 'ngl_obj' ) === 4 * 2,
              'Size of ngl_obj should be 8 bytes.' );
