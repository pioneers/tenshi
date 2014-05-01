#include <assert.h>
#include <ngl_api.h>
#include <stdio.h>

int main(void) {
  ngl_vm * vm = ngl_api_vm_new();
  assert(vm != NULL);
  ngl_module *core = ngl_api_vm_get_module(vm, "core");
  assert(core != NULL);
  void *ex_fn_t = ngl_api_module_get_symbol(core, "ngl_type_ngl_ex_func");
  assert(ex_fn_t != NULL);
  core = ngl_api_vm_get_module(vm, "core");
  assert(core != NULL);
  return 0;
}
