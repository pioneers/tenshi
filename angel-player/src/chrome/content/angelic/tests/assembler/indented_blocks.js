var root = process.argv[2];
var misc = require ( root + '/misc.js' );
var assembler = require ( root + '/assembler.js' );

var asm = assembler.make ( );

asm.begin_obj ( 'test_obj' );
  var label = asm.emit_label ( );
    asm.emit_li ( 'int', 1 );
    var label2 = asm.emit_label ( );
      asm.emit_li ( 'int', 1 );
  asm.emit_label_end ( label );
  asm.emit_li ( 'int', 1 );
asm.end_obj ( );

function line ( i, out ) {
  if ( i === 0 ) {
    return out;
    }
  else {
    return line ( i - 1, out.substr ( out.indexOf ( '\n' ) + 1 ) );
    }
  }

var o = asm.to_asm ( '', '  ' );

misc.print ( o );

misc.assert ( /  [^ ]/.test ( line ( 1, o ).substr ( 0, 3 ) ),
              'Indent should be 2 spaces.' );
misc.assert ( /    [^ ]/.test ( line ( 2, o ).substr ( 0, 5 ) ),
              'Indent should be 4 spaces.' );
misc.assert ( /      [^ ]/.test ( line ( 6, o ).substr ( 0, 7 ) ),
              'Indent should be 6 spaces.' );

var o = asm.to_asm ( '  ', '    ' );

misc.print ( o );

misc.assert ( /  [^ ]/.test ( line ( 1, o ) ),
              'Indent should be 2 spaces.' );
misc.assert ( /      [^ ]/.test ( line ( 2, o ) ),
              'Indent should be 6 spaces.' );
misc.assert ( /          [^ ]/.test ( line ( 6, o ) ),
              'Indent should be 10 spaces.' );
