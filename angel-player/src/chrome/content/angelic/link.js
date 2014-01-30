var make = function make ( func ) {
  return {
    type: 'external',
    func: func,
    };
  };

exports.make = make;
