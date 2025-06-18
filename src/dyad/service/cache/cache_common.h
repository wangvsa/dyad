#ifndef DYAD_CACHE_COMMON_H
#define DADY_CACHE_COMMON_H
#include <mercury.h>
#include <mercury_macros.h>

#define CACHE_SUCCESS  0
#define CACHE_FAILURE -1

#define CACHE_MARGO_PROVIDER_ID 1001


// RPC request/response structures
/*
MERCURY_GEN_PROC(cache_request_t,
    ((hg_string_t)(filename))
    ((hg_size_t)(file_size))
)

MERCURY_GEN_PROC(cache_response_t,
    ((hg_int32_t)(status))
    ((hg_string_t)(message))
)
*/

MERCURY_GEN_PROC(sum_in_t,
        ((int32_t)(x))\
        ((int32_t)(y)))

MERCURY_GEN_PROC(sum_out_t, ((int32_t)(ret)))

#endif
