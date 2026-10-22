/*
 * kiwi_ext.h - the C ABI for Kiwi native extensions.
 *
 * A native extension is a shared library (.so / .dylib / .dll) that exports
 *
 *     KIWI_EXPORT int kiwi_ext_init(const KiwiApi* api, KiwiCtx* ctx);
 *
 * Kiwi loads it with `native::load(path)`. `kiwi_ext_init` registers the
 * extension's functions with `api->define` and returns 0 on success.
 *
 * Values cross the boundary as `KiwiValue` handles. A handle is valid until
 * the native call that received or created it returns; never store one.
 * Handle 0 means "no value": a function that returns 0 returns `null` to
 * Kiwi, and accessors return 0 on failure.
 *
 * To report an error, call `api->raise(ctx, "message")` and return 0. The
 * error is thrown in Kiwi as a `NativeError` once the function returns.
 * Accessors that fail (for example `to_int` on a string) raise for you.
 *
 * Strings are UTF-8. Pointers returned by `string_get` and `bytes_get` stay
 * valid until the native call returns.
 *
 * New members are only ever appended to KiwiApi. Check `api->abi_version`
 * (or `api->size`) before using a member newer than the version you target.
 */
#ifndef KIWI_EXT_H
#define KIWI_EXT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KIWI_ABI_VERSION 1

#if defined(_WIN32)
#  define KIWI_EXPORT __declspec(dllexport)
#else
#  define KIWI_EXPORT __attribute__((visibility("default")))
#endif

typedef struct KiwiCtx KiwiCtx;
typedef uint64_t KiwiValue;

typedef enum KiwiType {
    KIWI_NONE    = 0,
    KIWI_INT     = 1,
    KIWI_FLOAT   = 2,
    KIWI_BOOL    = 3,
    KIWI_STRING  = 4,
    KIWI_LIST    = 5,
    KIWI_HASHMAP = 6,
    KIWI_BYTES   = 7,
    KIWI_POINTER = 8,
    KIWI_LAMBDA  = 9,
    KIWI_OTHER   = 10
} KiwiType;

/* A native function. `argv` holds `argc` argument handles. */
typedef KiwiValue (*KiwiFn)(KiwiCtx* ctx, int argc, const KiwiValue* argv);

/* Arity passed to `define` for a function that takes any number of arguments. */
#define KIWI_VARIADIC (-1)

typedef struct KiwiApi {
    uint32_t abi_version;
    uint32_t size;

    /* Registration (only valid inside kiwi_ext_init). Returns 0 on success. */
    int (*define)(KiwiCtx* ctx, const char* name, KiwiFn fn, int arity);

    /* Errors */
    void (*raise)(KiwiCtx* ctx, const char* message);

    /* Inspection */
    int (*type_of)(KiwiCtx* ctx, KiwiValue v);

    /* Construction */
    KiwiValue (*new_none)(KiwiCtx* ctx);
    KiwiValue (*new_int)(KiwiCtx* ctx, int64_t v);
    KiwiValue (*new_float)(KiwiCtx* ctx, double v);
    KiwiValue (*new_bool)(KiwiCtx* ctx, int v);
    KiwiValue (*new_string)(KiwiCtx* ctx, const char* utf8, size_t len);
    KiwiValue (*new_bytes)(KiwiCtx* ctx, const uint8_t* data, size_t len);
    KiwiValue (*new_pointer)(KiwiCtx* ctx, void* p);

    /* Conversion. These raise and return 0 if the value has the wrong type. */
    int64_t (*to_int)(KiwiCtx* ctx, KiwiValue v);
    double  (*to_float)(KiwiCtx* ctx, KiwiValue v);
    int     (*to_bool)(KiwiCtx* ctx, KiwiValue v);
    void*   (*to_pointer)(KiwiCtx* ctx, KiwiValue v);
    const char*    (*string_get)(KiwiCtx* ctx, KiwiValue v, size_t* len);
    const uint8_t* (*bytes_get)(KiwiCtx* ctx, KiwiValue v, size_t* len);

    /* Lists */
    KiwiValue (*new_list)(KiwiCtx* ctx);
    int64_t   (*list_len)(KiwiCtx* ctx, KiwiValue list);
    KiwiValue (*list_get)(KiwiCtx* ctx, KiwiValue list, int64_t index);
    int       (*list_push)(KiwiCtx* ctx, KiwiValue list, KiwiValue item);

    /* Hashmaps. hashmap_get returns 0 (without raising) for a missing key. */
    KiwiValue (*new_hashmap)(KiwiCtx* ctx);
    KiwiValue (*hashmap_get)(KiwiCtx* ctx, KiwiValue map, KiwiValue key);
    int       (*hashmap_set)(KiwiCtx* ctx, KiwiValue map, KiwiValue key, KiwiValue value);
    KiwiValue (*hashmap_keys)(KiwiCtx* ctx, KiwiValue map);

    /* Calls a Kiwi lambda or function. Returns 0 if it threw; return 0 yourself
       to let the error propagate. */
    KiwiValue (*call)(KiwiCtx* ctx, KiwiValue callable, int argc, const KiwiValue* argv);
} KiwiApi;

typedef int (*KiwiExtInit)(const KiwiApi* api, KiwiCtx* ctx);

#ifdef __cplusplus
}
#endif

#endif /* KIWI_EXT_H */
