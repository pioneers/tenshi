#ifndef NGL_PACKAGE_H_
#define NGL_PACKAGE_H_
#include <stdint.h>

#include <ngl_macros.h>
#include <ngl_error.h>
#include <ngl_vm.h>

#define NGL_PATCH_KIND_EXTERNAL 0
#define NGL_PATCH_KIND_BINARY 1
#define NGL_PATCH_KIND_BYTECODE 2
#define NGL_PATCH_KIND_OBJ 3

def_struct(ngl_package, {
  uint32_t magic;
  uint32_t version;
  uint32_t fixup_table_offset;
  uint32_t patch_table_offset;
})

def_struct(ngl_package_fixup, {
  uint32_t kind;
  uint32_t id;
  uint32_t offset;
})

def_struct(ngl_package_fixup_table, {
  uint32_t count;
  ngl_package_fixup first_fixup;
})

def_struct(ngl_package_patch, {
  uint32_t kind;
  uint32_t id;
  uint32_t offset;
  uint32_t to_delete;
  uint32_t to_insert;
})

def_struct(ngl_package_patch_table, {
  uint32_t count;
  ngl_package_patch first_patch;
})

extern ngl_error ngl_package_corrupt;
extern ngl_error ngl_package_blocked;
extern ngl_error ngl_package_inconsistent;
extern uint32_t ngl_package_magic;
extern uint32_t ngl_package_version;

ngl_error *ngl_package_apply(ngl_package * package, ngl_vm * vm);

ngl_error *ngl_vm_run_package(ngl_vm * vm, ngl_package * pkg);

ngl_error *ngl_run_package(ngl_package * pkg);

#endif /* end of include guard: NGL_PACKAGE_H_ */ /* NOLINT(*) */
