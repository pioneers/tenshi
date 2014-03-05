#include <stdio.h>
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("%s: Usage '%s <package>'\n", argv[0], argv[0]);
    return 1;
  }
  ngl_buffer *program = ngl_buffer_from_filename(ngl_str_from_static(argv[1]));
  if (program == NULL) {
    printf("%s: Could not load package '%s'\n", argv[0], argv[1]);
    return 1;
  }
  ngl_error *e = ngl_run_package((ngl_package *) NGL_BUFFER_DATA(program));
  if (e != ngl_ok) {
    printf("%s: Encountered error running package.\n", argv[0]);
  }
  return 0;
}
