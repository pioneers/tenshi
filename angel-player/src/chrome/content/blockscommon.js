var EXPORTED_SYMBOLS = ['blocksCommon'];

var blocksCommon = {};

// Possible block end caps:
// Flat face
blocksCommon.BLOCK_END_FLAT = 0;
// Arrow that points "into" the center of the block
blocksCommon.BLOCK_END_INNER_ARROW = 1;
// Arrow that points "out" away from the center of the block. This version is
// always visible.
blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT = 2;
// Arrow that points "out" away from the center of the block. This version is
// not shown unless there is a nearby "inner arrow" being dragged at it.
blocksCommon.BLOCK_END_OUTER_ARROW_PROXIMITY = 3;

// A text comment
blocksCommon.BLOCK_TYPE_COMMENT = 0;
