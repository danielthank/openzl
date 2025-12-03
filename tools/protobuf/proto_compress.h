// Copyright (c) Meta Platforms, Inc. and affiliates.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct ZL_ProtoSerializer ZL_ProtoSerializer;
typedef struct ZL_ProtoDeserializer ZL_ProtoDeserializer;

// ============ Schema Identifier ============

typedef enum {
    ZL_PROTO_SCHEMA_OTLP_METRICS = 0,
    ZL_PROTO_SCHEMA_OTLP_TRACES = 1,
    ZL_PROTO_SCHEMA_OTAP = 2,
    ZL_PROTO_SCHEMA_TPCH = 3,
    ZL_PROTO_SCHEMA_OTLP_METRICS_DICT = 4,
} ZL_ProtoSchema;

// ============ Error Handling ============

// Get the last error message (thread-local)
// Returns empty string if no error
const char* ZL_Proto_getLastError(void);

// Clear the last error message
void ZL_Proto_clearLastError(void);

// ============ Serializer (Compression) ============

// Create serializer with generic graph
ZL_ProtoSerializer* ZL_ProtoSerializer_create(void);

// Create serializer with trained compressor
ZL_ProtoSerializer* ZL_ProtoSerializer_createWithCompressor(
    const void* compressor_bytes,
    size_t compressor_len);

void ZL_ProtoSerializer_free(ZL_ProtoSerializer* serializer);

// Compress proto bytes using schema-aware compression
// Returns bytes written, or 0 on error
size_t ZL_ProtoSerializer_compress(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len,
    ZL_ProtoSchema schema);

// ============ Deserializer (Decompression) ============

ZL_ProtoDeserializer* ZL_ProtoDeserializer_create(void);

void ZL_ProtoDeserializer_free(ZL_ProtoDeserializer* deserializer);

// Decompress proto bytes using schema-aware decompression
// Returns bytes written, or 0 on error
size_t ZL_ProtoDeserializer_decompress(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len,
    ZL_ProtoSchema schema);

// ============ Message Comparison ============

// Compare two proto messages for semantic equality
// Returns 1 if equal, 0 if not equal or on error
int ZL_Proto_compare(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len,
    ZL_ProtoSchema schema);

#ifdef __cplusplus
}
#endif
