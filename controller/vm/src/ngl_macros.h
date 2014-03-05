#ifndef NGL_MACROS_H_
#define NGL_MACROS_H_

#define ngl_dbg_prnt(format, thing) \
  printf(#thing " == " format "\n", (thing));

/* Structs and typedefs. */
#define declare_struct(name) \
  struct name; \
  typedef struct name name;

#define def_struct(name, body) \
  struct name; \
  typedef struct name name; \
  struct name body;

#define define_struct(name, body) \
  struct name body;

#define def_union(name, body) \
  union name; \
  typedef union name name; \
  union name body;

#define END ; /* NOLINT(whitespace/semicolon) */

/* Token manipulation. */
#define join_token(a, b) join_token_inner(a, b)
#define join_token_inner(a, b) a ## b
#define join_token3(a, b, c) join_token(join_token(a, b), c)

#define string_of_macro(m) string_of_token(m)
#define string_of_token(t) #t

#define ngl_type_to_ngl_type(type) (join_token(ngl_type_, type))

#define ngl_check_static_test(type) (join_token(ngl_check_, type))

#define ngl_check_to_ngl_val_pointer(type, value) \
  ((ngl_val) { .pointer =(ngl_obj *) ngl_check_static_test(type)(value) })
#define ngl_check_to_ngl_val_integer(type, value) \
  ((ngl_val) { .integer = ngl_check_static_test(type)(value) })
#define ngl_check_to_ngl_val_uinteger(type, value) \
  ((ngl_val) { .uinteger = ngl_check_static_test(type)(value) })
#define ngl_check_to_ngl_val_floating(type, value) \
  ((ngl_val) { .floating = ngl_check_static_test(type)(value) })


#define ngl_declare_alien(alien_type) \
  extern ngl_type * join_token(ngl_type_, alien_type); \
  alien_type ngl_check_static_test(alien_type) (alien_type to_check); \
  ngl_declare_ptr(alien_type)

#define ngl_define_alien(alien_type) \
  alien_type ngl_check_static_test(alien_type) (alien_type to_check) { \
    return to_check; \
    } \
  ngl_type * join_token(ngl_type_, alien_type); \
  ngl_define_ptr(alien_type)

#define ngl_init_alien(alien_type) \
  join_token(ngl_type_, alien_type) = \
      (ngl_type *) ngl_alloc_simple(ngl_type_alien, 1); \
  if (join_token(ngl_type_, alien_type) == NULL) { \
    ngl_alloc_error(); \
    } \
  join_token(ngl_type_, alien_type)->header.type = ngl_type_ngl_type_alien; \
  join_token(ngl_type_, alien_type)->name = \
      ngl_str_lit(string_of_macro(alien_type)); \
  join_token(ngl_type_, alien_type)->size = sizeof(alien_type); \
  ngl_init_ptr(alien_type); \
  join_token(ngl_type_, alien_type)->ptr_of = \
      join_token3(ngl_type_, alien_type, _ptr)


#define ngl_declare_base(base_type) \
  extern ngl_type * join_token(ngl_type_, base_type); \
  base_type ngl_check_static_test(base_type) (base_type to_check); \
  ngl_declare_ptr(base_type)

#define ngl_define_base(base_type) \
  ngl_type * join_token(ngl_type_, base_type); \
  base_type ngl_check_static_test(base_type) (base_type to_check) { \
    return to_check; \
    } \
  ngl_define_ptr(base_type)

#define ngl_init_base(base_type) \
  join_token(ngl_type_, base_type) = \
      (ngl_type *) ngl_alloc_simple(ngl_type_base, 1); \
  if (join_token(ngl_type_, base_type) == NULL) { \
    ngl_alloc_error(); \
    } \
  join_token(ngl_type_, base_type)->header.type = ngl_type_ngl_type_base; \
  join_token(ngl_type_, base_type)->name = \
      ngl_str_lit(string_of_macro(base_type)); \
  join_token(ngl_type_, base_type)->size = sizeof(base_type); \
  ngl_init_ptr(base_type); \
  join_token(ngl_type_, base_type)->ptr_of = \
      join_token3(ngl_type_, base_type, _ptr);


#define ngl_declare_composite(composite_type) \
  extern ngl_type * join_token(ngl_type_, composite_type); \
  composite_type ngl_check_static_test(composite_type) \
      (composite_type to_check); \
  ngl_declare_ptr(composite_type)

#define ngl_define_composite(composite_type) \
  ngl_type * join_token(ngl_type_, composite_type); \
  composite_type ngl_check_static_test(composite_type) \
      (composite_type to_check) { \
    return to_check; \
    } \
  ngl_define_ptr(composite_type)

#define ngl_init_composite(composite_type) \
  join_token(ngl_type_, composite_type) = \
      (ngl_type *) ngl_alloc_simple(ngl_type_composite, 1); \
  if (join_token(ngl_type_, composite_type) == NULL) { \
    ngl_alloc_error(); \
    } \
  join_token(ngl_type_, composite_type)->header.type = \
      ngl_type_ngl_type_composite; \
  join_token(ngl_type_, composite_type)->name = \
      ngl_str_lit(string_of_macro(composite_type)); \
  join_token(ngl_type_, composite_type)->size = sizeof(composite_type); \
((ngl_type_composite *) join_token(ngl_type_, composite_type))-> \
      names_to_types = \
      ngl_table_new(ngl_type_ngl_str_ptr, \
                    ngl_type_ngl_type_ptr, \
                    &ngl_str_table_i); \
((ngl_type_composite *) join_token(ngl_type_, composite_type))-> \
      names_to_offsets = \
      ngl_table_new(ngl_type_ngl_str_ptr, \
                    ngl_type_ngl_uint, \
                    &ngl_str_table_i); \
  if (!((ngl_type_composite *) join_token(ngl_type_, \
                                          composite_type))->names_to_types || \
       !((ngl_type_composite *) join_token(ngl_type_, \
            composite_type))->names_to_offsets) { \
    ngl_table_delete(((ngl_type_composite *) \
          join_token(ngl_type_, composite_type))->names_to_types); \
    ngl_table_delete(((ngl_type_composite *) \
          join_token(ngl_type_, composite_type))->names_to_offsets); \
    ngl_alloc_error(); \
    } \
  ngl_init_ptr(composite_type); \
  join_token(ngl_type_, composite_type)->ptr_of = \
      join_token3(ngl_type_, composite_type, _ptr);


#define ngl_declare_ptr(base_type) \
  typedef base_type * join_token(base_type, _ptr); \
  extern ngl_type * join_token3(ngl_type_, base_type, _ptr); \
  base_type * ngl_check_static_test(join_token(base_type, _ptr))(base_type *); \
  typedef base_type ** join_token(base_type, _ptr_ptr); \
  extern ngl_type * join_token3(ngl_type_, base_type, _ptr_ptr); \
  base_type ** ngl_check_static_test \
      (join_token(base_type, _ptr_ptr))(base_type **)

#define ngl_define_ptr(base_type) \
  ngl_type * join_token3(ngl_type_, base_type, _ptr); \
  base_type * ngl_check_static_test(join_token(base_type, _ptr)) \
      (base_type * to_check) { \
    return to_check; \
    } \
  base_type ** ngl_check_static_test(join_token(base_type, _ptr_ptr)) \
      (base_type ** to_check) { \
    return to_check; \
    } \
  ngl_type * join_token3(ngl_type_, base_type, _ptr_ptr)

#define ngl_init_ptr(base_type) \
  join_token3(ngl_type_, base_type, _ptr) = \
      (ngl_type *) ngl_alloc_simple(ngl_type_pointer, 1); \
  if (join_token3(ngl_type_, base_type, _ptr) == NULL) { \
    ngl_alloc_error(); \
    } \
  join_token3(ngl_type_, base_type, _ptr)->header.type = \
      ngl_type_ngl_type_pointer; \
  join_token3(ngl_type_, base_type, _ptr)->name = \
      ngl_str_lit(string_of_macro(join_token(base_type, _ptr))); \
  join_token3(ngl_type_, base_type, _ptr)->size = \
      sizeof(base_type *); \
  ((ngl_type_pointer *) join_token3(ngl_type_, base_type, _ptr))->subtype = \
        ngl_type_to_ngl_type(base_type); \
  join_token3(ngl_type_, base_type, _ptr_ptr) = \
        (ngl_type *) ngl_alloc_simple(ngl_type_pointer, 1); \
  if (join_token(ngl_type_, base_type) == NULL) { \
    ngl_alloc_error(); \
    } \
  join_token3(ngl_type_, base_type, _ptr_ptr)->header.type = \
      ngl_type_ngl_type_pointer; \
  join_token3(ngl_type_, base_type, _ptr_ptr)->ptr_of = NULL; \
  join_token3(ngl_type_, base_type, _ptr_ptr)->name = \
      ngl_str_lit(string_of_macro(join_token(base_type, _ptr_ptr))); \
  join_token3(ngl_type_, base_type, _ptr_ptr)->size = \
      sizeof(base_type **); \
((ngl_type_pointer *) join_token3(ngl_type_, base_type, \
      _ptr_ptr))->subtype = \
      ngl_type_to_ngl_type(join_token(base_type, _ptr)); \
  join_token3(ngl_type_, base_type, _ptr)->ptr_of = \
      join_token3(ngl_type_, base_type, _ptr_ptr);

#define ngl_obj_cast(target_type, value) \
(value &&(value)->header.type == ngl_type_to_ngl_type(target_type) ? \
     ((join_token(target_type, _ptr))(value)) : \
     NULL)

/*
 * To be used like this:
 * ngl_var_of(i, ngl_token_infix, v) {
 *   return ngl_i->lbp;
 *   } ngl_var_end
 * else ngl_var_of(p, ngl_token_prefix, v) {
 *   return 0;
 *   } ngl_var_end
 * else {
 *   return 0;
 *   }
 */

#define ngl_var_of(varname, target_type, value) \
  if (ngl_obj_cast(target_type, value)) { \
      if (false) { \
        value = value; \
        } \
      target_type * varname = ngl_obj_cast(target_type, value);

#define ngl_var_end }

#define ngl_obj_of(obj) \
(&((obj)->header.type) ==(ngl_type **)(obj) ?(ngl_obj *)(obj) : NULL)

#define ngl_err(var, expr) \
  if ((var) != ngl_ok) { \
    (expr); \
  } else { \
    (var) = (expr); \
  }

#define ngl_obj_to(target_type, store_ptr, expr) \
  do { \
    ngl_obj * macro_the_expr =(expr); \
    target_type ** macro_the_store_ptr = &(store_ptr); \
    if (ngl_obj->header.type == target_type##_type) { \
      *macro_the_store_ptr =(target_type *) ngl_obj; \
    } else { \
      *macro_the_store_ptr = NULL; \
      NGL_OBJ_CAST_ERROR(target_type, store_ptr, macro_the_expr); \
    } \
  } while (0)

#define ngl_obj_expr(target_type, varname, expr) \
  do { \
    target_type * varname; \
    ngl_obj_to(target_type, varname, expr); \
  } while (0)

#define ngl_ret_on_err(expr) \
  do { \
    ngl_error * macro_error; \
    if ((macro_error =(expr)) != ngl_ok) { \
      return macro_error; \
    } \
  } while (0)

#endif /* end of include guard: NGL_MACROS_H_ */ /* NOLINT(*) */
