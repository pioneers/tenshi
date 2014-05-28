#ifndef NGL_VM_H_
#define NGL_VM_H_

#include <ngl_types_wrapper.h>

#include <ngl_func.h>
#include <ngl_error.h>
#include <ngl_macros.h>
#include <ngl_globals.h>
#include <ngl_thread.h>

ngl_declare_composite(ngl_vm);

ngl_error *ngl_vm_exec(ngl_vm * vm, ngl_thread * thread, ngl_int ticks);
ngl_error *ngl_vm_init(ngl_vm * vm);

/* TODO(kzentner): Finish implementing modules and kill this hack. */
extern ngl_uint ngl_vm_core_length;
extern ngl_module_entry * ngl_vm_core;

#endif  // NGL_VM_H_
