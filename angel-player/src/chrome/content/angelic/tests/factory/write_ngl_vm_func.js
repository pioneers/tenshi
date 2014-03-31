var root = process.argv[2];

var buffer = require ( root + '/buffer.js' );
var misc = require ( root + '/misc.js' );
var factory = require ( root + '/factory.js' );

var fact = factory.make ( );
fact.set_target_type ( 'ARM' );
fact.load_type_file ( root + '/../common_defs/ngl_types.yaml' );

var func = fact.create ( 'ngl_vm_func' );
func.set_slot ( 'header', { 'refc' : 1, 'type' : 0xff } );
func.set_slot ( 'code', 0xbee );
func.set_slot ( 'stack_space', 10 );

var buf = buffer.Buffer ( fact.get_size ( 'ngl_vm_func' ) );
func.write ( buf );

misc.assert ( buf.readUInt32LE ( 0 ) === 1, 'refc should be 1' );
misc.assert ( buf.readUInt32LE ( 4 ) === 0xff, 'type should be 0xff' );
misc.assert ( buf.readUInt32LE ( 8 ) === 0xbee, 'code should be 0xbee' );
misc.assert ( buf.readUInt32LE ( 12 ) === 10, 'stack_space should be 10' );
