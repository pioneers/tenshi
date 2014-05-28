#include <ngl_package.h>
#include <ngl_alloc.h>
#include <stdbool.h>
#include <string.h>

ngl_error ngl_package_corrupt;
ngl_error ngl_package_blocked;
ngl_error ngl_package_inconsistent;

bool
ngl_package_patch_appliable(ngl_package_patch * patch, ngl_vm * vm) {
  (void) patch;
  (void) vm;
  return true;
}

#define NEXT_PATCH(patch) \
(ngl_package_patch *) ((uint8_t *)(patch) + \
(sizeof(ngl_package_patch) + (patch)->to_insert))

#define PATCH_DIFF_SIZE(patch) \
((ngl_int)(patch)->to_insert -(ngl_int)(patch)->to_delete)

#define OFFSET(base, amount) \
((void *)((uint8_t *)(base) +(amount)))


ngl_error *
ngl_package_relocate_obj(ngl_vm * vm,
                         ngl_package_patch * patch,
                         ngl_uint size_diff,
                         ngl_uint patch_count, ngl_uint to_insert) {
  ngl_uint id = patch->id;
  ngl_uint kind = patch->kind;
  ngl_obj *obj = NULL;
  ngl_globals_get_obj_from_ids(&vm->globals, kind, id, &obj);

  if (obj == NULL) {
    /* This is a new object. */
    obj = (ngl_obj *) ngl_alloc_simple(uint8_t, to_insert);
    if (obj == NULL) {
      return &ngl_out_of_memory;
    } else {
      obj->refc = 1;
    }
    ngl_ret_on_err(ngl_globals_set_obj_from_ids(&vm->globals, kind, id, obj));
  }


  if (size_diff > 0) {
    /* We need to actually relocate the object. */
    ngl_int old_size;
    if (kind == NGL_PATCH_KIND_OBJ) {
      old_size = ngl_obj_size(obj);
    } else if (kind == NGL_PATCH_KIND_BYTECODE) {
      old_size = *(uint32_t *) obj;
    } else {
      return &ngl_package_inconsistent;
    }
    ngl_int new_size = old_size + size_diff;
    ngl_obj *new_obj = (ngl_obj *) ngl_alloc_simple(uint8_t, new_size);
    if (!new_obj) {
      return &ngl_out_of_memory;
    }

    ngl_uint eq_size = patch->offset;
    ngl_uint in_offset = 0;
    ngl_uint out_offset = 0;

    for (uint32_t i = 0; i < patch_count; i++) {
      memcpy(OFFSET(new_obj, out_offset), OFFSET(obj, in_offset), eq_size);
      in_offset += eq_size;
      out_offset += eq_size + PATCH_DIFF_SIZE(patch);
      ngl_package_patch *next_patch = NEXT_PATCH(patch);
      eq_size = next_patch->offset - (patch->offset + PATCH_DIFF_SIZE(patch));

      patch = next_patch;
    }
  } else {
    /* We can re-use the already existing object. */

    ngl_uint eq_size = patch->offset;
    ngl_uint in_offset = 0;
    ngl_uint out_offset = 0;

    for (uint32_t i = 0; i < patch_count; i++) {
      memmove(OFFSET(obj, out_offset), OFFSET(obj, in_offset), eq_size);
      in_offset += eq_size;
      out_offset += eq_size + PATCH_DIFF_SIZE(patch);
      ngl_package_patch *next_patch = NEXT_PATCH(patch);
      eq_size = next_patch->offset - (patch->offset + PATCH_DIFF_SIZE(patch));

      patch = next_patch;
    }
  }
  return ngl_ok;
}

ngl_error *
ngl_package_patch_apply(ngl_vm * vm,
                        ngl_package_patch * patch,
                        ngl_uint count) {
  ngl_obj *obj = NULL;
  ngl_globals_get_obj_from_ids(&vm->globals, patch->kind, patch->id, &obj);

  if (obj == NULL) {
    return &ngl_package_inconsistent;
  }

  ngl_uint out_offset = 0;
  for (uint32_t i = 0; i < count; i++) {
    memcpy(OFFSET(obj,
                  out_offset + patch->offset),
           OFFSET(patch, sizeof(ngl_package_patch)), patch->to_insert);
    out_offset += PATCH_DIFF_SIZE(patch);
    patch = NEXT_PATCH(patch);
  }

  return ngl_ok;
}

ngl_error *
ngl_package_apply(ngl_package * pkg, ngl_vm * vm) {
  if (pkg->magic != NGL_PACKAGE_MAGIC) {
    return &ngl_package_corrupt;
  } else if (pkg->version != NGL_PACKAGE_VERSION) {
    return &ngl_package_corrupt;
  }
  ngl_package_fixup_table *fixups = OFFSET(pkg, pkg->fixup_table_offset);
  ngl_package_patch_table *patches = OFFSET(pkg, pkg->patch_table_offset);

  /*
   * For patch in table:
   *   check that the patch can be applied
   * For patch in table:
   *   compute new object size
   *   If new object is larger, relocate it, with holes.
   *   Delete the old version.
   * For each fixup, lookup the(new) object's location, and write it.
   * For patch in table:
   *   Write data from patch.
   */

  ngl_package_patch *patch = patches->patches;
  for (uint32_t p = 0; p < patches->count; p++) {
    if (!ngl_package_patch_appliable(patch, vm)) {
      return &ngl_package_blocked;
    }
    patch = NEXT_PATCH(patch);
  }

  patch = patches->patches;
  for (uint32_t p = 0; p < patches->count;) {
    ngl_package_patch *first_patch = patch;
    uint32_t id = patch->id;
    uint32_t kind = patch->kind;
    ngl_int size_diff = 0;
    ngl_uint count = 0;
    ngl_uint to_insert = 0;
    while (patch->id == id && patch->kind == kind && p < patches->count) {
      size_diff += PATCH_DIFF_SIZE(patch);
      to_insert += patch->to_insert;
      patch = NEXT_PATCH(patch);
      ++count;
      ++p;
    }
    ngl_package_relocate_obj(vm, first_patch, size_diff, count, to_insert);
  }

  ngl_package_fixup *fixup = fixups->fixups;

  for (uint32_t f = 0; f < fixups->count; f++) {
    ngl_obj *to_write;
    if (ngl_globals_get_obj_from_ids(&vm->globals,
                                     fixup->kind,
                                     fixup->id, &to_write) != ngl_ok) {
      return &ngl_package_inconsistent;
    }

    /*
     * Note that this implies that code is different between 64 and 32 bit
     * machines. Alternatively, all immediates will need 64 bits of space, 32
     * bits of which will not be used on 32 bit systems.
     */
    *(ngl_obj **) OFFSET(pkg, fixup->offset) = to_write;
    fixup += 1;
  }

  patch = patches->patches;
  uint32_t p = 0;
  while (p < patches->count) {
    ngl_package_patch *first_patch = patch;
    uint32_t id = patch->id;
    uint32_t kind = patch->kind;
    ngl_uint count = 0;
    while (patch->id == id && patch->kind == kind && p < patches->count) {
      patch = NEXT_PATCH(patch);
      ++count;
      ++p;
    }
    ngl_ret_on_err(ngl_package_patch_apply(vm, first_patch, count));
  }
  return ngl_ok;
}

ngl_error *
ngl_vm_run_package(ngl_vm * vm, ngl_package * pkg) {
  ngl_ret_on_err(ngl_package_apply(pkg, vm));
  /*
   * TODO(kzentner): This function is hacked to work until the
   * compiler outputs objects properly.
   * TODO(kzentner): Instead of directly executing the 0th bytecode
   * buffer, execute the main function.
   */
  ngl_vm_func *main_func = ngl_alloc_simple(ngl_vm_func, 1);
  ngl_buffer *code;
  ngl_ret_on_err(ngl_globals_get_obj_from_ids(&vm->globals,
                                              2, 0, (ngl_obj **) & code));
  ngl_vm_func_init((ngl_func *) main_func, code, 8);
  ngl_thread *main_thread;
  ngl_ret_on_err(ngl_array_get(&vm->threads, 0, ngl_val_addr(&main_thread)));
  main_thread->current_func = main_func;
  ngl_ret_on_err(ngl_vm_exec(vm, main_thread, -1));
  /* TODO(kzentner): Free allocated memory. */
  return ngl_ok;
}

ngl_error *
ngl_run_package(ngl_package * pkg) {
  ngl_vm vm;
  ngl_ret_on_err(ngl_vm_init(&vm));
  return ngl_vm_run_package(&vm, pkg);
}
