#ifndef NGL_PACKAGE_H_
#define NGL_PACKAGE_H_

#include <ngl_types_wrapper.h>

#include <stdint.h>

#include <ngl_macros.h>
#include <ngl_error.h>
#include <ngl_vm.h>

extern ngl_error ngl_package_corrupt;
extern ngl_error ngl_package_blocked;
extern ngl_error ngl_package_inconsistent;

ngl_error *ngl_package_apply(ngl_package * package, ngl_vm * vm);

ngl_error *ngl_vm_run_package(ngl_vm * vm, ngl_package * pkg);

ngl_error *ngl_run_package(ngl_package * pkg);

#endif  // NGL_PACKAGE_H_
