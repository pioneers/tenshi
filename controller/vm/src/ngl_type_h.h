#ifndef NGL_TYPE_H_H_
#define NGL_TYPE_H_H_
#include <ngl_macros.h>
/*
 * This file exists because there would otherwise be several unresolved
 * circular dependencies:
 *   - ngl_type -> ngl_obj -> ngl_type
 *   - ngl_type -> ngl_obj -> ngl_val -> ngl_type
 *   - ngl_type -> ngl_str -> ngl_obj -> ngl_type
 *   - ngl_type -> ngl_val -> ngl_obj -> ngl_type
 *   - ngl_type -> ngl_bits -> ngl_val -> ngl_type
 *   - ngl_type -> ngl_builtins -> ngl_type
 *   ... probably more.
 */

declare_struct(ngl_type) /* NOLINT(*) */
declare_struct(ngl_type_alien) /* NOLINT(*) */
declare_struct(ngl_type_base) /* NOLINT(*) */
declare_struct(ngl_type_composite) /* NOLINT(*) */
declare_struct(ngl_type_pointer) /* NOLINT(*) */

#endif /* end of include guard: NGL_TYPE_H_H_ */ /* NOLINT(*) */
