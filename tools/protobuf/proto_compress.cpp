// Copyright (c) Meta Platforms, Inc. and affiliates.
#include "tools/protobuf/proto_compress.h"
#include "tools/protobuf/ProtoDeserializer.h"
#include "tools/protobuf/ProtoSerializer.h"
#include "tools/protobuf/schema_otlp_metrics.pb.h"
#include "tools/protobuf/schema_otlp_traces.pb.h"
#include "tools/protobuf/schema_otap.pb.h"
#include "tools/protobuf/schema_tpch.pb.h"
#include "tools/protobuf/schema_otlpmetricsdict.pb.h"
#include "tools/protobuf/schema_otlptracesdict.pb.h"
#include <cstring>
#include <string>

using OtlpMetrics =
    opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using OtlpTraces =
    opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using Otap =
    opentelemetry::proto::experimental::arrow::v1::BatchArrowRecords;
using TpchBatch = tpch::TpchBatch;
using OtlpMetricsDict = otlpmetricsdict::MetricsDictBatch;
using OtlpTracesDict = otlptracesdict::TracesDictBatch;

// Thread-local storage for last error message
static thread_local std::string g_last_error;

struct ZL_ProtoSerializer {
    openzl::protobuf::ProtoSerializer serializer;
};

struct ZL_ProtoDeserializer {
    openzl::protobuf::ProtoDeserializer deserializer;
};

// ============ Template Helpers ============

template<typename T>
static size_t compressImpl(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len,
    const char* type_name)
{
    T message;
    if (!message.ParseFromArray(src, src_len)) {
        g_last_error = std::string("Failed to parse ") + type_name + " proto";
        return 0;
    }
    std::string compressed = serializer->serializer.serialize(message);
    if (compressed.size() > dst_capacity) {
        g_last_error = "Buffer too small: need " + std::to_string(compressed.size()) +
                       " bytes, have " + std::to_string(dst_capacity);
        return 0;
    }
    memcpy(dst, compressed.data(), compressed.size());
    return compressed.size();
}

template<typename T>
static size_t decompressImpl(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len,
    const char* type_name)
{
    std::string compressed(static_cast<const char*>(src), src_len);
    T message;
    deserializer->deserializer.deserialize(compressed, message);

    std::string proto_bytes = message.SerializeAsString();
    if (proto_bytes.size() > dst_capacity) {
        g_last_error = "Buffer too small: need " + std::to_string(proto_bytes.size()) +
                       " bytes, have " + std::to_string(dst_capacity);
        return 0;
    }
    memcpy(dst, proto_bytes.data(), proto_bytes.size());
    return proto_bytes.size();
}

template<typename T>
static int compareImpl(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len,
    const char* type_name)
{
    T msg1, msg2;
    if (!msg1.ParseFromArray(proto1, proto1_len)) {
        g_last_error = std::string("Failed to parse first ") + type_name + " proto";
        return 0;
    }
    if (!msg2.ParseFromArray(proto2, proto2_len)) {
        g_last_error = std::string("Failed to parse second ") + type_name + " proto";
        return 0;
    }
    return msg1.SerializeAsString() == msg2.SerializeAsString() ? 1 : 0;
}

extern "C" {

// ============ Error Handling ============

const char* ZL_Proto_getLastError()
{
    return g_last_error.c_str();
}

void ZL_Proto_clearLastError()
{
    g_last_error.clear();
}

// ============ Serializer ============

ZL_ProtoSerializer* ZL_ProtoSerializer_create()
{
    try {
        g_last_error.clear();
        return new ZL_ProtoSerializer();
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return nullptr;
    } catch (...) {
        g_last_error = "Unknown exception creating serializer";
        return nullptr;
    }
}

ZL_ProtoSerializer* ZL_ProtoSerializer_createWithCompressor(
    const void* compressor_bytes,
    size_t compressor_len)
{
    try {
        g_last_error.clear();
        auto* ps = new ZL_ProtoSerializer();
        openzl::Compressor compressor;
        compressor.deserialize(std::string_view(
            static_cast<const char*>(compressor_bytes), compressor_len));
        ps->serializer.setCompressor(std::move(compressor));
        return ps;
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return nullptr;
    } catch (...) {
        g_last_error = "Unknown exception creating serializer with compressor";
        return nullptr;
    }
}

void ZL_ProtoSerializer_free(ZL_ProtoSerializer* serializer)
{
    delete serializer;
}

size_t ZL_ProtoSerializer_compress(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len,
    ZL_ProtoSchema schema)
{
    try {
        g_last_error.clear();
        switch (schema) {
            case ZL_PROTO_SCHEMA_OTLP_METRICS:
                return compressImpl<OtlpMetrics>(serializer, dst, dst_capacity, src, src_len, "OtlpMetrics");
            case ZL_PROTO_SCHEMA_OTLP_TRACES:
                return compressImpl<OtlpTraces>(serializer, dst, dst_capacity, src, src_len, "OtlpTraces");
            case ZL_PROTO_SCHEMA_OTAP:
                return compressImpl<Otap>(serializer, dst, dst_capacity, src, src_len, "Otap");
            case ZL_PROTO_SCHEMA_TPCH:
                return compressImpl<TpchBatch>(serializer, dst, dst_capacity, src, src_len, "TpchBatch");
            case ZL_PROTO_SCHEMA_OTLP_METRICS_DICT:
                return compressImpl<OtlpMetricsDict>(serializer, dst, dst_capacity, src, src_len, "OtlpMetricsDict");
            case ZL_PROTO_SCHEMA_OTLP_TRACES_DICT:
                return compressImpl<OtlpTracesDict>(serializer, dst, dst_capacity, src, src_len, "OtlpTracesDict");
            default:
                g_last_error = "Unknown schema: " + std::to_string(static_cast<int>(schema));
                return 0;
        }
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception during compression";
        return 0;
    }
}

// ============ Deserializer ============

ZL_ProtoDeserializer* ZL_ProtoDeserializer_create()
{
    try {
        g_last_error.clear();
        return new ZL_ProtoDeserializer();
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return nullptr;
    } catch (...) {
        g_last_error = "Unknown exception creating deserializer";
        return nullptr;
    }
}

void ZL_ProtoDeserializer_free(ZL_ProtoDeserializer* deserializer)
{
    delete deserializer;
}

size_t ZL_ProtoDeserializer_decompress(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len,
    ZL_ProtoSchema schema)
{
    try {
        g_last_error.clear();
        switch (schema) {
            case ZL_PROTO_SCHEMA_OTLP_METRICS:
                return decompressImpl<OtlpMetrics>(deserializer, dst, dst_capacity, src, src_len, "OtlpMetrics");
            case ZL_PROTO_SCHEMA_OTLP_TRACES:
                return decompressImpl<OtlpTraces>(deserializer, dst, dst_capacity, src, src_len, "OtlpTraces");
            case ZL_PROTO_SCHEMA_OTAP:
                return decompressImpl<Otap>(deserializer, dst, dst_capacity, src, src_len, "Otap");
            case ZL_PROTO_SCHEMA_TPCH:
                return decompressImpl<TpchBatch>(deserializer, dst, dst_capacity, src, src_len, "TpchBatch");
            case ZL_PROTO_SCHEMA_OTLP_METRICS_DICT:
                return decompressImpl<OtlpMetricsDict>(deserializer, dst, dst_capacity, src, src_len, "OtlpMetricsDict");
            case ZL_PROTO_SCHEMA_OTLP_TRACES_DICT:
                return decompressImpl<OtlpTracesDict>(deserializer, dst, dst_capacity, src, src_len, "OtlpTracesDict");
            default:
                g_last_error = "Unknown schema: " + std::to_string(static_cast<int>(schema));
                return 0;
        }
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception during decompression";
        return 0;
    }
}

// ============ Message Comparison ============

int ZL_Proto_compare(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len,
    ZL_ProtoSchema schema)
{
    try {
        g_last_error.clear();
        switch (schema) {
            case ZL_PROTO_SCHEMA_OTLP_METRICS:
                return compareImpl<OtlpMetrics>(proto1, proto1_len, proto2, proto2_len, "OtlpMetrics");
            case ZL_PROTO_SCHEMA_OTLP_TRACES:
                return compareImpl<OtlpTraces>(proto1, proto1_len, proto2, proto2_len, "OtlpTraces");
            case ZL_PROTO_SCHEMA_OTAP:
                return compareImpl<Otap>(proto1, proto1_len, proto2, proto2_len, "Otap");
            case ZL_PROTO_SCHEMA_TPCH:
                return compareImpl<TpchBatch>(proto1, proto1_len, proto2, proto2_len, "TpchBatch");
            case ZL_PROTO_SCHEMA_OTLP_METRICS_DICT:
                return compareImpl<OtlpMetricsDict>(proto1, proto1_len, proto2, proto2_len, "OtlpMetricsDict");
            case ZL_PROTO_SCHEMA_OTLP_TRACES_DICT:
                return compareImpl<OtlpTracesDict>(proto1, proto1_len, proto2, proto2_len, "OtlpTracesDict");
            default:
                g_last_error = "Unknown schema: " + std::to_string(static_cast<int>(schema));
                return 0;
        }
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception during comparison";
        return 0;
    }
}

} // extern "C"
