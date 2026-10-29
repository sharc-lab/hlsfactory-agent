// Host-side stand-in for Dynamatic's integration header, so a translated kernel file compiles and runs its
// main() under plain clang. The real header (in the Dynamatic image) expands CALL_KERNEL to the same call,
// or to argument dumping for profiling and C/RTL co-simulation.
#ifndef DYNAMATIC_INTEGRATION_H
#define DYNAMATIC_INTEGRATION_H
#define CALL_KERNEL(kernel, args...) kernel(args)
#endif
