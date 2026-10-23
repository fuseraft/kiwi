/*
 * ffi_test_lib.c - functions for tests/lib/stdlib/ffi.kiwi to call through ffi.
 *
 * The weighted sums return a different result if any argument lands in the wrong register
 * or stack slot, so they check the calling-convention handling in src/Runtime/Ffi.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __attribute__((visibility("default")))
#endif

/* Every integer width, at its edges. */
EXPORT int8_t   neg_i8(int8_t x)    { return (int8_t)-x; }
EXPORT uint8_t  inc_u8(uint8_t x)   { return (uint8_t)(x + 1); }
EXPORT int16_t  neg_i16(int16_t x)  { return (int16_t)-x; }
EXPORT uint16_t inc_u16(uint16_t x) { return (uint16_t)(x + 1); }
EXPORT int32_t  add_i32(int32_t a, int32_t b) { return a + b; }
EXPORT uint32_t inc_u32(uint32_t x) { return x + 1; }
EXPORT int64_t  add_i64(int64_t a, int64_t b) { return a + b; }
EXPORT uint64_t max_u64(void)       { return UINT64_MAX; }
EXPORT bool     is_even(int32_t x)  { return x % 2 == 0; }
EXPORT int32_t  not_bool(bool b)    { return !b; }

/* Floating point. */
EXPORT float  scale_f(float x, float k)    { return x * k; }
EXPORT double scale_d(double x, double k)  { return x * k; }
EXPORT double widen(float x)               { return (double)x; }
EXPORT float  narrow(double x)             { return (float)x; }

/* Integer and floating-point arguments interleaved. */
EXPORT double mixed(int32_t a, double b, int64_t c, float d, uint8_t e, double f) {
    return a + b * 10 + c * 100 + d * 1000 + e * 10000 + f * 100000;
}

/* Each mix of integer (int64_t) and double in the first four positions. */
EXPORT double combo_0(int64_t a0, int64_t a1, int64_t a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_1(double a0, int64_t a1, int64_t a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_2(int64_t a0, double a1, int64_t a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_3(double a0, double a1, int64_t a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_4(int64_t a0, int64_t a1, double a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_5(double a0, int64_t a1, double a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_6(int64_t a0, double a1, double a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_7(double a0, double a1, double a2, int64_t a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_8(int64_t a0, int64_t a1, int64_t a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_9(double a0, int64_t a1, int64_t a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_10(int64_t a0, double a1, int64_t a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_11(double a0, double a1, int64_t a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_12(int64_t a0, int64_t a1, double a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_13(double a0, int64_t a1, double a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_14(int64_t a0, double a1, double a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }
EXPORT double combo_15(double a0, double a1, double a2, double a3) { return a0 * 1 + a1 * 10 + a2 * 100 + a3 * 1000; }

/* More arguments than registers, so some go on the stack. */
EXPORT int64_t ints10(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
                      int64_t f, int64_t g, int64_t h, int64_t i, int64_t j) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

EXPORT double doubles10(double a, double b, double c, double d, double e,
                        double f, double g, double h, double i, double j) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

EXPORT double mixed12(int64_t a, double b, int64_t c, double d, int64_t e, double f,
                      int64_t g, double h, int64_t i, double j, int64_t k, double l) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10 + k * 11 + l * 12;
}

/* A 4-byte argument on the stack: macOS arm64 packs these, which ffi rejects there. */
EXPORT int64_t small_on_stack(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
                              int64_t f, int64_t g, int64_t h, int64_t i, int64_t j,
                              int64_t k, int32_t l) {
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Strings, buffers and pointers. */
static char greeting[64];

EXPORT const char* greet(const char* name) {
    strcpy(greeting, "hello, ");
    strncat(greeting, name ? name : "nobody", sizeof greeting - 8);
    return greeting;
}

EXPORT const char* no_string(void) { return NULL; }
EXPORT int64_t length(const char* s) { return s ? (int64_t)strlen(s) : -1; }

EXPORT void fill_squares(int32_t* out, int32_t n) {
    for (int32_t i = 0; i < n; i++) out[i] = i * i;
}

EXPORT int64_t sum_bytes(const uint8_t* data, size_t n) {
    int64_t total = 0;
    for (size_t i = 0; i < n; i++) total += data[i];
    return total;
}

EXPORT void* identity(void* p) { return p; }

typedef int32_t (*binary_fn)(int32_t, int32_t);
EXPORT binary_fn get_adder(void) { return add_i32; }

/* A struct passed by pointer: { int32_t x; double y; }, so y is at offset 8. */
typedef struct { int32_t x; double y; } point;
EXPORT double point_sum(const point* p) { return p->x + p->y; }
EXPORT void point_scale(point* p, int32_t k) { p->x *= k; p->y *= k; }

/* An exported variable, and an array of strings ending in NULL. */
EXPORT int32_t counter = 41;
EXPORT const char* words[] = { "kiwi", "ffi", NULL };
