// Copyright (c) Meta Platforms, Inc. and affiliates.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct ZL_ProtoSerializer ZL_ProtoSerializer;
typedef struct ZL_ProtoDeserializer ZL_ProtoDeserializer;

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

// Compress OTLP metrics: proto bytes -> compressed bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoSerializer_compressOtlpMetrics(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// Compress OTLP traces: proto bytes -> compressed bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoSerializer_compressOtlpTraces(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// Compress OTAP (BatchArrowRecords): proto bytes -> compressed bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoSerializer_compressOtap(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// Compress TPC-H batch: proto bytes -> compressed bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoSerializer_compressTpch(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// ============ Deserializer (Decompression) ============

ZL_ProtoDeserializer* ZL_ProtoDeserializer_create(void);

void ZL_ProtoDeserializer_free(ZL_ProtoDeserializer* deserializer);

// Decompress OTLP metrics: compressed bytes -> proto bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoDeserializer_decompressOtlpMetrics(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// Decompress OTLP traces: compressed bytes -> proto bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoDeserializer_decompressOtlpTraces(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// Decompress OTAP (BatchArrowRecords): compressed bytes -> proto bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoDeserializer_decompressOtap(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// Decompress TPC-H batch: compressed bytes -> proto bytes
// Returns bytes written, or 0 on error
size_t ZL_ProtoDeserializer_decompressTpch(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len);

// ============ Message Comparison ============

// Compare two OTLP metrics proto messages for semantic equality
// Returns 1 if equal, 0 if not equal or on error
int ZL_Proto_compareOtlpMetrics(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len);

// Compare two OTLP traces proto messages for semantic equality
// Returns 1 if equal, 0 if not equal or on error
int ZL_Proto_compareOtlpTraces(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len);

// Compare two OTAP (BatchArrowRecords) proto messages for semantic equality
// Returns 1 if equal, 0 if not equal or on error
int ZL_Proto_compareOtap(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len);

// Compare two TPC-H batch proto messages for semantic equality
// Returns 1 if equal, 0 if not equal or on error
int ZL_Proto_compareTpch(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len);

#ifdef __cplusplus
}
#endif
