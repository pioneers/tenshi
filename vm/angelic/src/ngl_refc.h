#ifndef NGL_REFC_H_
#define NGL_REFC_H_

#include <ngl_obj.h>
#include <ngl_alloc.h>
#include <ngl_val.h>

ngl_uint ngl_refc(ngl_obj * obj);
void ngl_refd(ngl_obj * obj);
void ngl_refi(ngl_obj * obj);

#endif  // NGL_REFC_H_
