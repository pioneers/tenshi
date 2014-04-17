#ifndef NGL_BUILTINS_H_
#define NGL_BUILTINS_H_

#include <ngl_types_wrapper.h>

#include <ngl_macros.h>
#include <ngl_val.h>
#include <ngl_error.h>
#include <ngl_obj.h>

ngl_error *ngl_builtins_init();

ngl_declare_base(ngl_uint);
ngl_declare_base(ngl_int);
ngl_declare_base(ngl_float);

ngl_declare_composite(ngl_obj);
ngl_declare_composite(ngl_error);

ngl_declare_composite(ngl_table);
ngl_declare_composite(ngl_table_iter);

ngl_declare_composite(ngl_module);

ngl_declare_alien(ngl_builtin_alien);

ngl_error *ngl_print_float(ngl_float);
ngl_error *ngl_set_motor(ngl_float motor, ngl_float val);
ngl_error *ngl_get_sensor(ngl_float sensor, ngl_float *val);

#define ngl_call_name ngl_print_float
#define ngl_call_argc 1
#include <ngl_call_declare.h> /* NOLINT(build/include) */

#define ngl_call_name ngl_set_motor
#define ngl_call_argc 2
#include <ngl_call_declare.h> /* NOLINT(build/include) */

#define ngl_call_name ngl_get_sensor
#define ngl_call_argc 1
#include <ngl_call_declare.h> /* NOLINT(build/include) */

#endif  // NGL_BUILTINS_H_
