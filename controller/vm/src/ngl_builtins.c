#include <assert.h>
#include <stdbool.h>
#include <ngl_builtins.h>
#include <ngl_str.h>
#include <ngl_alloc.h>
#include <ngl_thread.h>
#include <ngl_type.h>
#include <ngl_val.h>
#include <ngl_vm.h>
#include <ngl_error.h>
#include <ngl_func.h>
#include <ngl_obj.h>

bool ngl_builtins_initialized = false;

ngl_define_base(ngl_val);
ngl_define_base(ngl_uint);
ngl_define_base(ngl_int);
ngl_define_base(ngl_float);
ngl_define_composite(ngl_obj);

ngl_define_base(ngl_func);
ngl_define_base(ngl_ex_func);
ngl_define_base(ngl_vm_func);

ngl_define_composite(ngl_error);
ngl_define_composite(ngl_str);
ngl_define_composite(ngl_buffer);

ngl_define_alien(ngl_builtin_alien);

ngl_call ngl_null_call;

ngl_error *
ngl_print_float(ngl_float f) {
  printf("%f\n", (double) f);
  return ngl_ok;
}

/* 
 * TODO(kzentner): Replace these functions with functions to actually interact
 * with hardware.
 */
ngl_error *ngl_set_motor(ngl_uint motor, ngl_float val) {
  (void) motor;
  (void) val;
  return ngl_ok;
}

ngl_error *ngl_get_sensor(ngl_uint sensor, ngl_float *val) {
  (void) sensor;
  (void) val;
  return ngl_ok;
}

#define ngl_call_name ngl_print_float
#define ngl_call_args NGL_ARG_FLOAT(0, ngl_float)
#define ngl_call_argc 1
#include <ngl_call_define.c> /* NOLINT(build/include) */

#define ngl_call_name ngl_set_motor
#define ngl_call_args NGL_ARG_UINT(0, ngl_uint), NGL_ARG_FLOAT(1, ngl_float)
#define ngl_call_argc 2
#include <ngl_call_define.c> /* NOLINT(build/include) */

#define ngl_call_name ngl_get_sensor
#define ngl_call_args NGL_ARG_UINT(0, ngl_uint), NGL_ARG_PTR(1, ngl_float*)
#define ngl_call_argc 2
#include <ngl_call_define.c> /* NOLINT(build/include) */

ngl_error *
ngl_builtins_init() {
  if (ngl_builtins_initialized) {
    return ngl_ok;
  }
#define ngl_alloc_error() \
    return &ngl_out_of_memory;
  ngl_init_base(ngl_uint);
  ngl_init_base(ngl_int);
  ngl_init_base(ngl_float);
  ngl_init_base(ngl_val);

  ngl_init_composite(ngl_type);
  ngl_init_composite(ngl_type_base);
  ngl_init_composite(ngl_type_composite);
  ngl_init_composite(ngl_type_pointer);

  ngl_init_composite(ngl_str);
  ngl_init_composite(ngl_type);
  ngl_init_composite(ngl_call);
  ngl_init_composite(ngl_error);
  ngl_init_composite(ngl_obj);
  ngl_init_base(ngl_func);
  ngl_init_base(ngl_ex_func);
  ngl_init_base(ngl_vm_func);
  ngl_init_composite(ngl_thread);
  ngl_init_composite(ngl_vm);
  ngl_init_composite(ngl_buffer);

  ngl_init_alien(ngl_builtin_alien);

  ngl_init_null_call(&ngl_null_call);

#define ngl_call_name ngl_print_float
#include <ngl_call_init.c> /* NOLINT(build/include) */

#define ngl_call_name ngl_set_motor
#include <ngl_call_init.c> /* NOLINT(build/include) */

#define ngl_call_name ngl_get_sensor
#include <ngl_call_init.c> /* NOLINT(build/include) */
  return ngl_ok;
}

#undef init_builtin
