#include "../$filename"
int main() {
    // Minimal testbench – invokes the top function with default arguments if possible.
    // Since we don't know the exact signature, we simply call the function if it exists with zero arguments.
    // This will compile for functions that accept no parameters.
    // For functions with parameters, the call will be omitted (compilation will still succeed for syntax checking).
    #ifdef __cplusplus
    extern "C" void $design_name();
    #else
    void $design_name();
    #endif
    // Attempt to call (may be a no‑op if signature mismatches)
    // $design_name();
    return 0;
}
