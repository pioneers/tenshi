#ifndef NGL_TABLE_H_
#define NGL_TABLE_H_
/* This makes cpplint happy.  ^_^ */
#undef NGL_TABLE_H_

/*
 * This generates an api which is abi compatible with the data structure
 * described in ngl_table_ngl_val.{h, c}. All methods here forward to functions
 * there.
 */

#include <ngl_table_ngl_val.h>
#include <ngl_macros.h>

#ifndef ngl_table_key_t
#error Must define ngl_table_key_t.
#endif

#ifndef ngl_table_val_t
#error Must define ngl_table_val_t.
#endif

#ifndef ngl_table_key_name
#define ngl_table_key_name ngl_table_key_t
#endif

#ifndef ngl_table_val_name
#define ngl_table_val_name ngl_table_val_t
#endif


#ifndef ngl_table_key_kind
#error Please define ngl_table_key_kind to one of \
       integer, uinteger, floating, or pointer. /* NOLINT(*) */
#endif

#ifndef ngl_table_val_kind
#error Please define ngl_table_val_kind to one of \
       integer, uinteger, floating, or pointer. /* NOLINT(*) */
#endif


#ifndef ngl_table_key_ngl_type
#define ngl_table_key_ngl_type ngl_type_to_ngl_type(ngl_table_key_name)
#endif

#ifndef ngl_table_val_ngl_type
#define ngl_table_val_ngl_type ngl_type_to_ngl_type(ngl_table_val_name)
#endif


#ifndef ngl_table_key_hash
#define ngl_table_key_hash join_token(ngl_hash_, ngl_table_key_name)
#endif

#ifndef ngl_table_key_compare
#define ngl_table_key_compare join_token(ngl_compare_, ngl_table_key_name)
#endif


#ifndef ngl_table_name
#define ngl_table_name join_token(join_token(ngl_table_key_name, _to_), \
                                  ngl_table_val_name)
#endif

#define method(mname) join_token(join_token(ngl_table_, ngl_table_name), \
                                 join_token(_, mname))

#define ngl_table_struct join_token(ngl_table_, ngl_table_name)

#define ngl_table_interface join_token(ngl_table_, \
                                       join_token(ngl_table_name, _interface))

static ngl_table_elem_interface ngl_table_interface = {
  &ngl_table_key_hash,
  &ngl_table_key_compare
} END

#define ngl_table_struct_to_table(ngl_table_s) (&(ngl_table_s)->table)

/* Cast an element from ngl_val back to the api type. */
#define ngl_table_backcast_key(key) \
    ((ngl_table_key_t)((key).ngl_table_key_kind))
#define ngl_table_backcast_val(val) \
    ((ngl_table_val_t)((val).ngl_table_val_kind))

#ifdef pointer
#error pointer should not be defined
#endif

/* 
 * Define pointer so that we can check if ngl_array_elem_kind is defined to be
 * pointer.
 */
/* ngl_array_cast_key casts a key from the api type to ngl_val. */
#define pointer 0xefaced
#if ngl_table_key_kind == pointer
#define ngl_table_cast_key(key) \
((ngl_val) { .ngl_table_key_kind =(ngl_obj *) key })
#else
#define ngl_table_cast_key(key) ((ngl_val) { .ngl_table_key_kind = key })
#endif
#undef pointer

/* ngl_array_cast_val casts a val from the api type to ngl_val. */
#define pointer 0xefaced
#if ngl_table_val_kind == pointer
#define ngl_table_cast_val(val) \
((ngl_val) { .ngl_table_val_kind = (ngl_obj *) val })
#else
#define ngl_table_cast_val(val) ((ngl_val) { .ngl_table_val_kind = val })
#endif
#undef pointer

def_struct(ngl_table_struct, {
  ngl_table table;
})

static ngl_table_struct *method(new) (void) {
  return (ngl_table_struct *) ngl_table_new(ngl_table_key_ngl_type,
                                            ngl_table_val_ngl_type,
                                            &ngl_table_interface);
}

static ngl_error *method(init_to_size) (ngl_table_struct * self,
                                        size_t predicted_elems) {
  return ngl_table_init_to_size(ngl_table_struct_to_table(self),
                                ngl_table_key_ngl_type,
                                ngl_table_val_ngl_type,
                                predicted_elems, &ngl_table_interface);
}

static ngl_error *method(init) (ngl_table_struct * self) {
  return ngl_table_init(ngl_table_struct_to_table(self),
                        ngl_table_key_ngl_type,
                        ngl_table_val_ngl_type, &ngl_table_interface);
}

static ngl_error *method(get) (ngl_table_struct * self, ngl_table_key_t key,
                               ngl_table_val_t * dst) {
  return ngl_table_get_hash(ngl_table_struct_to_table(self),
                            ngl_table_cast_key(key),
                            (ngl_val *) dst,
                            ngl_table_key_hash(ngl_table_cast_key(key)));
}

static ngl_error *method(get_or_add) (ngl_table_struct * self,
                                      ngl_table_key_t key,
                                      ngl_table_val_t * dst,
                                      ngl_call * on_missing) {
  return ngl_table_get_or_add_hash(ngl_table_struct_to_table(self),
                                   ngl_table_cast_key(key),
                                   (ngl_val *) dst,
                                   on_missing,
                                   ngl_table_key_hash(ngl_table_cast_key(key)));
}

static ngl_error *method(get_default) (ngl_table_struct * self,
                                       ngl_table_key_t key,
                                       ngl_table_val_t * dst,
                                       ngl_table_val_t default_value) {
  return ngl_table_get_default_hash(ngl_table_struct_to_table(self),
                                    ngl_table_cast_key(key),
                                    (ngl_val *) dst,
                                    ngl_table_cast_val(default_value),
                                    ngl_table_key_hash(ngl_table_cast_key
                                                       (key)));
}

static ngl_error *method(set) (ngl_table_struct * self,
                               ngl_table_key_t key, ngl_table_val_t val) {
  return ngl_table_set_hash(ngl_table_struct_to_table(self),
                            ngl_table_cast_key(key),
                            ngl_table_cast_val(val),
                            ngl_table_key_hash(ngl_table_cast_key(key)));
}

static ngl_error *method(delete) (ngl_table_struct * self) {
  return ngl_table_delete(ngl_table_struct_to_table(self));
}

#undef ngl_table_backcast_key
#undef ngl_table_backcast_val
#undef ngl_table_cast_key
#undef ngl_table_cast_val
#undef ngl_table_interface
#undef ngl_table_key_compare
#undef ngl_table_key_hash
#undef ngl_table_key_kind
#undef ngl_table_key_name
#undef ngl_table_key_t
#undef ngl_table_key_ngl_type
#undef ngl_table_name
#undef ngl_table_struct
#undef ngl_table_struct_to_table
#undef ngl_table_val_kind
#undef ngl_table_val_name
#undef ngl_table_val_t
#undef ngl_table_val_ngl_type
#undef method

#endif  // NGL_TABLE_H_
