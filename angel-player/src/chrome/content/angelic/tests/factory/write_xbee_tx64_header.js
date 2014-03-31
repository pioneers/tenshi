var root = process.argv[2];

var buffer = require ( root + '/buffer.js' );
var misc = require ( root + '/misc.js' );
var factory = require ( root + '/factory.js' );
var Int64 = require ( root + '/../vendor-js/Int64.js' );

var fact = factory.make ( );
fact.set_target_type ( 'ARM' );
fact.load_type_file ( root + '/../common_defs/xbee_typpo.yaml' );

var xbee_header = fact.create ( 'xbee_tx64_header' );

var addr = new Int64('0xfedcba8987654321');

xbee_header.set_slot ( 'xbee_api_type', 1 );
xbee_header.set_slot ( 'frameId', 0xff );
xbee_header.set_slot ( 'xbee_dest_addr', addr );
xbee_header.set_slot ( 'options', 10 );
xbee_header.set_slot ( 'data', [ 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb ] );

var buf = buffer.Buffer ( 1024 );
buf.fill ( 0xAA );

xbee_header.write ( buf );

var xb_h2 = fact.read ( 'xbee_tx64_header', buf ).unwrap ( );

misc.assert ( xb_h2.xbee_api_type === 1,
              'api type should be set correctly' );

misc.assert ( xb_h2.frameId === 0xff,
              'frame id should be set correctly' );

misc.print ( buf );
misc.print ( xb_h2.xbee_dest_addr );
misc.print ( addr );
misc.assert ( xb_h2.xbee_dest_addr.toString ( ) === addr.toString ( ),
              'dest addr should be set correctly' );

misc.assert ( xb_h2.options === 10,
              'options should be set correctly' );
