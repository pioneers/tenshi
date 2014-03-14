// Possible block end caps:
// Flat face
exports.BLOCK_END_FLAT = 0;
// Arrow that points "into" the center of the block
exports.BLOCK_END_INNER_ARROW = 1;
// Arrow that points "out" away from the center of the block. This version is
// always visible.
exports.BLOCK_END_OUTER_ARROW_PERSISTENT = 2;
// Arrow that points "out" away from the center of the block. This version is
// not shown unless there is a nearby "inner arrow" being dragged at it.
exports.BLOCK_END_OUTER_ARROW_PROXIMITY = 3;

// A text comment
exports.BLOCK_TYPE_COMMENT = 0;
// Assignment lvalue, something like "foo = <>"
exports.BLOCK_TYPE_LVALUE = 1;
