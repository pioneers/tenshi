#include <ngl_refc.h>

ngl_uint ngl_refc ( ngl_obj * obj ) {
  return obj->refc;
  }

void ngl_refd ( ngl_obj * obj ) {
  if ( ! obj ) {
    return;
    }
  if ( obj->refc <= 1 ) {
    ngl_free ( obj );
    }
  else {
    --obj->refc;
    }
  }

void ngl_refi ( ngl_obj * obj ) {
  if ( ! obj ) {
    return;
    }
  else {
    ++obj->refc;
    }
  }
