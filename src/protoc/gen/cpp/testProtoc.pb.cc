// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: testProtoc.proto

#include "testProtoc.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace protobuf {
PROTOBUF_CONSTEXPR DemoPb::DemoPb(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_._has_bits_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_.a_)*/0u} {}
struct DemoPbDefaultTypeInternal {
  PROTOBUF_CONSTEXPR DemoPbDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~DemoPbDefaultTypeInternal() {}
  union {
    DemoPb _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 DemoPbDefaultTypeInternal _DemoPb_default_instance_;
}  // namespace protobuf
namespace protobuf {

// ===================================================================

class DemoPb::_Internal {
 public:
  using HasBits = decltype(std::declval<DemoPb>()._impl_._has_bits_);
  static void set_has_a(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
};

DemoPb::DemoPb(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::MessageLite(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:protobuf.DemoPb)
}
DemoPb::DemoPb(const DemoPb& from)
  : ::PROTOBUF_NAMESPACE_ID::MessageLite() {
  DemoPb* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){from._impl_._has_bits_}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.a_){}};

  _internal_metadata_.MergeFrom<std::string>(from._internal_metadata_);
  _this->_impl_.a_ = from._impl_.a_;
  // @@protoc_insertion_point(copy_constructor:protobuf.DemoPb)
}

inline void DemoPb::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.a_){0u}
  };
}

DemoPb::~DemoPb() {
  // @@protoc_insertion_point(destructor:protobuf.DemoPb)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<std::string>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void DemoPb::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
}

void DemoPb::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void DemoPb::Clear() {
// @@protoc_insertion_point(message_clear_start:protobuf.DemoPb)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.a_ = 0u;
  _impl_._has_bits_.Clear();
  _internal_metadata_.Clear<std::string>();
}

const char* DemoPb::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // optional uint32 a = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _Internal::set_has_a(&has_bits);
          _impl_.a_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<std::string>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  _impl_._has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* DemoPb::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:protobuf.DemoPb)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // optional uint32 a = 1;
  if (_internal_has_a()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(1, this->_internal_a(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = stream->WriteRaw(_internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString).data(),
        static_cast<int>(_internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString).size()), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:protobuf.DemoPb)
  return target;
}

size_t DemoPb::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:protobuf.DemoPb)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // optional uint32 a = 1;
  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000001u) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_a());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    total_size += _internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString).size();
  }
  int cached_size = ::_pbi::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void DemoPb::CheckTypeAndMergeFrom(
    const ::PROTOBUF_NAMESPACE_ID::MessageLite& from) {
  MergeFrom(*::_pbi::DownCast<const DemoPb*>(
      &from));
}

void DemoPb::MergeFrom(const DemoPb& from) {
  DemoPb* const _this = this;
  // @@protoc_insertion_point(class_specific_merge_from_start:protobuf.DemoPb)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_has_a()) {
    _this->_internal_set_a(from._internal_a());
  }
  _this->_internal_metadata_.MergeFrom<std::string>(from._internal_metadata_);
}

void DemoPb::CopyFrom(const DemoPb& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:protobuf.DemoPb)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool DemoPb::IsInitialized() const {
  return true;
}

void DemoPb::InternalSwap(DemoPb* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_._has_bits_[0], other->_impl_._has_bits_[0]);
  swap(_impl_.a_, other->_impl_.a_);
}

std::string DemoPb::GetTypeName() const {
  return "protobuf.DemoPb";
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::protobuf::DemoPb*
Arena::CreateMaybeMessage< ::protobuf::DemoPb >(Arena* arena) {
  return Arena::CreateMessageInternal< ::protobuf::DemoPb >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
