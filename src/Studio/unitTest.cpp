#include <QtCore>
#include "RealityAction.h"

#include <dlfcn.h>

int main( int argc, char** argv ) {
  printf("Loading the Reality library...\n");

  void* lib_handle = dlopen(argv[1], RTLD_LOCAL|RTLD_LAZY);
  // void* lib_handle = dlopen("Reality_3_DS64.dylib", RTLD_LOCAL|RTLD_LAZY);
  if (!lib_handle) {
      printf("[%s] Unable to load library: %s\n", __FILE__, dlerror());
      exit(EXIT_FAILURE);
  }

}
