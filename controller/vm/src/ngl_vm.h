#ifndef NGL_VM_H
#define NGL_VM_H

#include <ngl_func.h>
#include <ngl_error.h>
#include <ngl_macros.h>
#include <ngl_globals.h>
#include <ngl_thread.h>

def_struct ( ngl_vm ) {
  ngl_obj header;
  ngl_globals globals;
  ngl_array threads;
  };

ngl_declare_composite ( ngl_vm );

ngl_error * ngl_vm_exec ( ngl_vm * vm, ngl_thread * thread, ngl_vm_func * func );
ngl_error * ngl_vm_init ( ngl_vm * vm );

#endif /* end of include guard: NGL_VM_H */
