/* A native extension exercising the KiwiApi surface, built and loaded by tests/lib/stdlib/native.kiwi. */
#include <string.h>
#include <stdlib.h>
#include "kiwi_ext.h"

static const KiwiApi* api;

static KiwiValue add(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    if (api->type_of(ctx, argv[0]) == KIWI_FLOAT || api->type_of(ctx, argv[1]) == KIWI_FLOAT)
        return api->new_float(ctx, api->to_float(ctx, argv[0]) + api->to_float(ctx, argv[1]));
    return api->new_int(ctx, api->to_int(ctx, argv[0]) + api->to_int(ctx, argv[1]));
}

static KiwiValue reverse(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    size_t len;
    const char* s = api->string_get(ctx, argv[0], &len);
    if (!s) return 0;
    char* out = malloc(len);
    for (size_t i = 0; i < len; i++) out[i] = s[len - 1 - i];
    KiwiValue r = api->new_string(ctx, out, len);
    free(out);
    return r;
}

static KiwiValue sum(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    int64_t total = 0;
    for (int i = 0; i < argc; i++) total += api->to_int(ctx, argv[i]);
    return api->new_int(ctx, total);
}

static KiwiValue squares(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    int64_t n = api->list_len(ctx, argv[0]);
    if (n < 0) return 0;
    KiwiValue out = api->new_list(ctx);
    for (int64_t i = 0; i < n; i++) {
        int64_t x = api->to_int(ctx, api->list_get(ctx, argv[0], i));
        api->list_push(ctx, out, api->new_int(ctx, x * x));
    }
    return out;
}

static KiwiValue describe(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    KiwiValue map = api->new_hashmap(ctx);
    KiwiValue name = api->hashmap_get(ctx, argv[0], api->new_string(ctx, "name", 4));
    api->hashmap_set(ctx, map, api->new_string(ctx, "has_name", 8), api->new_bool(ctx, name != 0));
    api->hashmap_set(ctx, map, api->new_string(ctx, "keys", 4), api->hashmap_keys(ctx, argv[0]));
    return map;
}

static KiwiValue apply(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    return api->call(ctx, argv[0], argc - 1, argv + 1);
}

static KiwiValue fail(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    api->raise(ctx, api->string_get(ctx, argv[0], NULL));
    return 0;
}

static KiwiValue nothing(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)ctx; (void)argc; (void)argv;
    return 0;
}

static KiwiValue bytes_xor(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    size_t len;
    const uint8_t* data = api->bytes_get(ctx, argv[0], &len);
    if (!data) return 0;
    uint8_t key = (uint8_t)api->to_int(ctx, argv[1]);
    uint8_t* out = malloc(len ? len : 1);
    for (size_t i = 0; i < len; i++) out[i] = data[i] ^ key;
    KiwiValue r = api->new_bytes(ctx, out, len);
    free(out);
    return r;
}

static int counter;

static KiwiValue counter_ptr(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc; (void)argv;
    return api->new_pointer(ctx, &counter);
}

static KiwiValue counter_bump(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    (void)argc;
    int* p = api->to_pointer(ctx, argv[0]);
    if (!p) return 0;
    return api->new_int(ctx, ++*p);
}

KIWI_EXPORT int kiwi_ext_init(const KiwiApi* host, KiwiCtx* ctx) {
    if (host->abi_version < KIWI_ABI_VERSION) return 1;
    api = host;
    api->define(ctx, "add", add, 2);
    api->define(ctx, "reverse", reverse, 1);
    api->define(ctx, "sum", sum, KIWI_VARIADIC);
    api->define(ctx, "squares", squares, 1);
    api->define(ctx, "describe", describe, 1);
    api->define(ctx, "apply", apply, KIWI_VARIADIC);
    api->define(ctx, "fail", fail, 1);
    api->define(ctx, "nothing", nothing, 0);
    api->define(ctx, "bytes_xor", bytes_xor, 2);
    api->define(ctx, "counter_ptr", counter_ptr, 0);
    api->define(ctx, "counter_bump", counter_bump, 1);
    return 0;
}
