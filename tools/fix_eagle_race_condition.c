#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

const char *BLOCK_RENAME_FROM = "/.~~~";
const char *BLOCK_RENAME_TO = "/.eaglerc";

// In order to avoid actually having to parse paths, we cheat and compare the
// paths starting from the end. This matches the filename portion of the path
// only.
int strcmp_backwards(const char *a, const char *b) {
  // Go to end of string
  const char *a_end = a + (strlen(a) - 1);
  const char *b_end = b + (strlen(b) - 1);

  while (a_end >= a && b_end >= b) {
    if ((*a_end) != (*b_end)) {
      return ((*a_end) < (*b_end)) ? -1 : 1;
    } else {
      a_end--;
      b_end--;
    }
  }

  return 0;
}

int rename(const char *old, const char *new) {
  if (strcmp_backwards(old, BLOCK_RENAME_FROM) == 0) {
    if (strcmp_backwards(new, BLOCK_RENAME_TO) == 0) {
      printf("To prevent race conditions, rename '%s' --> '%s' blocked\n",
        old, new);
      return 0;
    }
  }

  int (*orig_rename)(const char *old, const char *new);
  // Ask dynamic linker to search for the next instance of the "rename"
  // function, which will be the original libc version (unless we're hooked
  // multiple times).
  orig_rename = dlsym(RTLD_NEXT, "rename");
  return orig_rename(old, new);
}
