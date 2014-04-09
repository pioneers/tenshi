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

// TODO(kzentner): This is jank, but the build system is currently borked and
// can't include the right headers.
typedef struct tag_i2c_master_module {
  // No stuff here yet
} i2c_master_module;
extern i2c_master_module *i2c1_driver;
extern void *i2c_issue_transaction(i2c_master_module *module, uint8_t addr,
  uint8_t *data_out, size_t len_out, uint8_t *data_in, size_t len_in);
extern int i2c_transaction_status(i2c_master_module *module,
  void *transaction);
extern int i2c_transaction_finish(i2c_master_module *module,
  void *transaction);
#define I2C_TRANSACTION_STATUS_DONE       4
#define I2C_TRANSACTION_STATUS_ERROR      5
ngl_error *ngl_set_motor(ngl_uint motor, ngl_float val) {
  // TODO(rqou): Don't hardcode this. This is awful. Registers 0x01 to 0x08
  // inclusive.
  uint8_t out_buf[] = {0b00010011, 0, 0, 0, 0, 0, 0, 0};
  int32_t out_val = (int32_t)(val * 65536);
  out_buf[3] = (out_val >>  0) & 0xFF;
  out_buf[4] = (out_val >>  8) & 0xFF;
  out_buf[5] = (out_val >> 16) & 0xFF;
  out_buf[6] = (out_val >> 24) & 0xFF;

  // TODO(rqou): Don't hardcode this!
  uint8_t addr = (0x0f - motor) << 1;

  void *i2c_txn = i2c_issue_transaction(i2c1_driver, addr,
    out_buf, sizeof(out_buf), NULL, 0);
  // TODO(rqou): Make this not synchronous!
  int status;
  do {
    status = i2c_transaction_status(i2c1_driver, i2c_txn);
  } while (status != I2C_TRANSACTION_STATUS_DONE &&
           status != I2C_TRANSACTION_STATUS_ERROR);
  i2c_transaction_finish(i2c1_driver, i2c_txn);

  if (status == I2C_TRANSACTION_STATUS_DONE) {
    return ngl_ok;
  }
  // TODO(rqou): Better error!
  return &ngl_error_generic;
}

/* 
 * TODO(kzentner): Replace this function with function to actually interact
 * with hardware.
 */
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
#define ngl_call_return(x) (error = (x));
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
