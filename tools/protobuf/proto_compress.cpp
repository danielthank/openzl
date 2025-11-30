// Copyright (c) Meta Platforms, Inc. and affiliates.
#include "tools/protobuf/proto_compress.h"
#include "tools/protobuf/ProtoDeserializer.h"
#include "tools/protobuf/ProtoSerializer.h"
#include "tools/protobuf/schema_otlp_metrics.pb.h"
#include "tools/protobuf/schema_otlp_traces.pb.h"
#include "tools/protobuf/schema_otap.pb.h"
#include "tools/protobuf/schema_tpch.pb.h"
#include <cstring>
#include <string>

using OtlpMetrics =
    opentelemetry::proto::collector::metrics::v1::ExportMetricsServiceRequest;
using OtlpTraces =
    opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest;
using Otap =
    opentelemetry::proto::experimental::arrow::v1::BatchArrowRecords;
using TpchBatch = tpch::TpchBatch;

// Thread-local storage for last error message
static thread_local std::string g_last_error;

struct ZL_ProtoSerializer {
    openzl::protobuf::ProtoSerializer serializer;
};

struct ZL_ProtoDeserializer {
    openzl::protobuf::ProtoDeserializer deserializer;
};

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

size_t ZL_ProtoSerializer_compressOtlpMetrics(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        OtlpMetrics message;
        if (!message.ParseFromArray(src, src_len)) {
            g_last_error = "Failed to parse OtlpMetrics proto";
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
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception compressing OtlpMetrics";
        return 0;
    }
}

size_t ZL_ProtoSerializer_compressOtlpTraces(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        OtlpTraces message;
        if (!message.ParseFromArray(src, src_len)) {
            g_last_error = "Failed to parse OtlpTraces proto";
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
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception compressing OtlpTraces";
        return 0;
    }
}

size_t ZL_ProtoSerializer_compressOtap(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        Otap message;
        if (!message.ParseFromArray(src, src_len)) {
            g_last_error = "Failed to parse Otap proto";
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
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception compressing Otap";
        return 0;
    }
}

size_t ZL_ProtoSerializer_compressTpch(
    ZL_ProtoSerializer* serializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        TpchBatch message;
        if (!message.ParseFromArray(src, src_len)) {
            g_last_error = "Failed to parse TpchBatch proto";
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
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception compressing TpchBatch";
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

size_t ZL_ProtoDeserializer_decompressOtlpMetrics(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        std::string compressed(static_cast<const char*>(src), src_len);
        OtlpMetrics message;
        deserializer->deserializer.deserialize(compressed, message);

        std::string proto_bytes = message.SerializeAsString();
        if (proto_bytes.size() > dst_capacity) {
            g_last_error = "Buffer too small: need " + std::to_string(proto_bytes.size()) +
                           " bytes, have " + std::to_string(dst_capacity);
            return 0;
        }
        memcpy(dst, proto_bytes.data(), proto_bytes.size());
        return proto_bytes.size();
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception decompressing OtlpMetrics";
        return 0;
    }
}

size_t ZL_ProtoDeserializer_decompressOtlpTraces(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        std::string compressed(static_cast<const char*>(src), src_len);
        OtlpTraces message;
        deserializer->deserializer.deserialize(compressed, message);

        std::string proto_bytes = message.SerializeAsString();
        if (proto_bytes.size() > dst_capacity) {
            g_last_error = "Buffer too small: need " + std::to_string(proto_bytes.size()) +
                           " bytes, have " + std::to_string(dst_capacity);
            return 0;
        }
        memcpy(dst, proto_bytes.data(), proto_bytes.size());
        return proto_bytes.size();
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception decompressing OtlpTraces";
        return 0;
    }
}

size_t ZL_ProtoDeserializer_decompressOtap(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        std::string compressed(static_cast<const char*>(src), src_len);
        Otap message;
        deserializer->deserializer.deserialize(compressed, message);

        std::string proto_bytes = message.SerializeAsString();
        if (proto_bytes.size() > dst_capacity) {
            g_last_error = "Buffer too small: need " + std::to_string(proto_bytes.size()) +
                           " bytes, have " + std::to_string(dst_capacity);
            return 0;
        }
        memcpy(dst, proto_bytes.data(), proto_bytes.size());
        return proto_bytes.size();
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception decompressing Otap";
        return 0;
    }
}

size_t ZL_ProtoDeserializer_decompressTpch(
    ZL_ProtoDeserializer* deserializer,
    void* dst,
    size_t dst_capacity,
    const void* src,
    size_t src_len)
{
    try {
        g_last_error.clear();
        std::string compressed(static_cast<const char*>(src), src_len);
        TpchBatch message;
        deserializer->deserializer.deserialize(compressed, message);

        std::string proto_bytes = message.SerializeAsString();
        if (proto_bytes.size() > dst_capacity) {
            g_last_error = "Buffer too small: need " + std::to_string(proto_bytes.size()) +
                           " bytes, have " + std::to_string(dst_capacity);
            return 0;
        }
        memcpy(dst, proto_bytes.data(), proto_bytes.size());
        return proto_bytes.size();
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception decompressing TpchBatch";
        return 0;
    }
}

// ============ Message Comparison ============

int ZL_Proto_compareOtlpMetrics(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len)
{
    try {
        g_last_error.clear();
        OtlpMetrics msg1, msg2;
        if (!msg1.ParseFromArray(proto1, proto1_len)) {
            g_last_error = "Failed to parse first OtlpMetrics proto";
            return 0;
        }
        if (!msg2.ParseFromArray(proto2, proto2_len)) {
            g_last_error = "Failed to parse second OtlpMetrics proto";
            return 0;
        }
        // Use protobuf's built-in comparison
        return msg1.SerializeAsString() == msg2.SerializeAsString() ? 1 : 0;
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception comparing OtlpMetrics";
        return 0;
    }
}

int ZL_Proto_compareOtlpTraces(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len)
{
    try {
        g_last_error.clear();
        OtlpTraces msg1, msg2;
        if (!msg1.ParseFromArray(proto1, proto1_len)) {
            g_last_error = "Failed to parse first OtlpTraces proto";
            return 0;
        }
        if (!msg2.ParseFromArray(proto2, proto2_len)) {
            g_last_error = "Failed to parse second OtlpTraces proto";
            return 0;
        }
        return msg1.SerializeAsString() == msg2.SerializeAsString() ? 1 : 0;
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception comparing OtlpTraces";
        return 0;
    }
}

int ZL_Proto_compareOtap(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len)
{
    try {
        g_last_error.clear();
        Otap msg1, msg2;
        if (!msg1.ParseFromArray(proto1, proto1_len)) {
            g_last_error = "Failed to parse first Otap proto";
            return 0;
        }
        if (!msg2.ParseFromArray(proto2, proto2_len)) {
            g_last_error = "Failed to parse second Otap proto";
            return 0;
        }
        return msg1.SerializeAsString() == msg2.SerializeAsString() ? 1 : 0;
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception comparing Otap";
        return 0;
    }
}

int ZL_Proto_compareTpch(
    const void* proto1,
    size_t proto1_len,
    const void* proto2,
    size_t proto2_len)
{
    try {
        g_last_error.clear();
        TpchBatch msg1, msg2;
        if (!msg1.ParseFromArray(proto1, proto1_len)) {
            g_last_error = "Failed to parse first TpchBatch proto";
            return 0;
        }
        if (!msg2.ParseFromArray(proto2, proto2_len)) {
            g_last_error = "Failed to parse second TpchBatch proto";
            return 0;
        }
        return msg1.SerializeAsString() == msg2.SerializeAsString() ? 1 : 0;
    } catch (const std::exception& e) {
        g_last_error = e.what();
        return 0;
    } catch (...) {
        g_last_error = "Unknown exception comparing TpchBatch";
        return 0;
    }
}

} // extern "C"
