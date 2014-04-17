#ifndef NGL_API_H_

#define NGL_API_H_

#include <ngl_types_wrapper.h>

typedef char *ngl_api_error;

ngl_vm *ngl_api_vm_new();

ngl_module *ngl_api_module_new();

ngl_api_error ngl_api_add_symbol(ngl_module *module, char *name, void *obj);

ngl_api_error ngl_api_vm_load_module(ngl_vm *vm, char *name,
                                     ngl_module * mod);

ngl_api_error ngl_api_vm_load_pkg(ngl_vm *vm, ngl_package *pkg);

ngl_api_error ngl_api_vm_run(ngl_vm *vm);

ngl_api_error ngl_api_vm_run_ticks(ngl_vm *vm, uint32_t ticks);

ngl_api_error ngl_api_start_main(ngl_vm *vm);


#endif  // NGL_API_H_
