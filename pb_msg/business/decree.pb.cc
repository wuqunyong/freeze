// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: decree.proto

#include "decree.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
namespace pb {
namespace decree {
class decree_buyDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<decree_buy> _instance;
} _decree_buy_default_instance_;
class decree_reflushDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<decree_reflush> _instance;
} _decree_reflush_default_instance_;
}  // namespace decree
}  // namespace pb
static void InitDefaultsscc_info_decree_buy_decree_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::pb::decree::_decree_buy_default_instance_;
    new (ptr) ::pb::decree::decree_buy();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
  ::pb::decree::decree_buy::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_decree_buy_decree_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, 0, InitDefaultsscc_info_decree_buy_decree_2eproto}, {}};

static void InitDefaultsscc_info_decree_reflush_decree_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::pb::decree::_decree_reflush_default_instance_;
    new (ptr) ::pb::decree::decree_reflush();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
  ::pb::decree::decree_reflush::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_decree_reflush_decree_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, 0, InitDefaultsscc_info_decree_reflush_decree_2eproto}, {}};

static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_decree_2eproto[2];
static const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* file_level_enum_descriptors_decree_2eproto[1];
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_decree_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_decree_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::pb::decree::decree_buy, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::pb::decree::decree_buy, error_code_),
  PROTOBUF_FIELD_OFFSET(::pb::decree::decree_buy, get_count_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::pb::decree::decree_reflush, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::pb::decree::decree_reflush, next_time_),
  PROTOBUF_FIELD_OFFSET(::pb::decree::decree_reflush, cur_count_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, sizeof(::pb::decree::decree_buy)},
  { 7, -1, sizeof(::pb::decree::decree_reflush)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::pb::decree::_decree_buy_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::pb::decree::_decree_reflush_default_instance_),
};

const char descriptor_table_protodef_decree_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\014decree.proto\022\tpb.decree\"3\n\ndecree_buy\022"
  "\022\n\nerror_code\030\001 \001(\r\022\021\n\tget_count\030\002 \001(\r\"6"
  "\n\016decree_reflush\022\021\n\tnext_time\030\001 \001(\004\022\021\n\tc"
  "ur_count\030\002 \001(\r*Z\n\014E_decree_cmd\022\025\n\021E_decr"
  "ee_cmd_None\020\000\022\024\n\020E_decree_cmd_Buy\020\001\022\035\n\031E"
  "_decree_cmd_reflush_time\020\002b\006proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_decree_2eproto_deps[1] = {
};
static ::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase*const descriptor_table_decree_2eproto_sccs[2] = {
  &scc_info_decree_buy_decree_2eproto.base,
  &scc_info_decree_reflush_decree_2eproto.base,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_decree_2eproto_once;
static bool descriptor_table_decree_2eproto_initialized = false;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_decree_2eproto = {
  &descriptor_table_decree_2eproto_initialized, descriptor_table_protodef_decree_2eproto, "decree.proto", 234,
  &descriptor_table_decree_2eproto_once, descriptor_table_decree_2eproto_sccs, descriptor_table_decree_2eproto_deps, 2, 0,
  schemas, file_default_instances, TableStruct_decree_2eproto::offsets,
  file_level_metadata_decree_2eproto, 2, file_level_enum_descriptors_decree_2eproto, file_level_service_descriptors_decree_2eproto,
};

// Force running AddDescriptors() at dynamic initialization time.
static bool dynamic_init_dummy_decree_2eproto = (  ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptors(&descriptor_table_decree_2eproto), true);
namespace pb {
namespace decree {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* E_decree_cmd_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_decree_2eproto);
  return file_level_enum_descriptors_decree_2eproto[0];
}
bool E_decree_cmd_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
      return true;
    default:
      return false;
  }
}


// ===================================================================

void decree_buy::InitAsDefaultInstance() {
}
class decree_buy::_Internal {
 public:
};

decree_buy::decree_buy()
  : ::PROTOBUF_NAMESPACE_ID::Message(), _internal_metadata_(nullptr) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:pb.decree.decree_buy)
}
decree_buy::decree_buy(const decree_buy& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      _internal_metadata_(nullptr) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::memcpy(&error_code_, &from.error_code_,
    static_cast<size_t>(reinterpret_cast<char*>(&get_count_) -
    reinterpret_cast<char*>(&error_code_)) + sizeof(get_count_));
  // @@protoc_insertion_point(copy_constructor:pb.decree.decree_buy)
}

void decree_buy::SharedCtor() {
  ::memset(&error_code_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&get_count_) -
      reinterpret_cast<char*>(&error_code_)) + sizeof(get_count_));
}

decree_buy::~decree_buy() {
  // @@protoc_insertion_point(destructor:pb.decree.decree_buy)
  SharedDtor();
}

void decree_buy::SharedDtor() {
}

void decree_buy::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const decree_buy& decree_buy::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_decree_buy_decree_2eproto.base);
  return *internal_default_instance();
}


void decree_buy::Clear() {
// @@protoc_insertion_point(message_clear_start:pb.decree.decree_buy)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::memset(&error_code_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&get_count_) -
      reinterpret_cast<char*>(&error_code_)) + sizeof(get_count_));
  _internal_metadata_.Clear();
}

const char* decree_buy::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // uint32 error_code = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          error_code_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // uint32 get_count = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 16)) {
          get_count_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag, &_internal_metadata_, ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* decree_buy::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:pb.decree.decree_buy)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // uint32 error_code = 1;
  if (this->error_code() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt32ToArray(1, this->_internal_error_code(), target);
  }

  // uint32 get_count = 2;
  if (this->get_count() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt32ToArray(2, this->_internal_get_count(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:pb.decree.decree_buy)
  return target;
}

size_t decree_buy::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:pb.decree.decree_buy)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // uint32 error_code = 1;
  if (this->error_code() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt32Size(
        this->_internal_error_code());
  }

  // uint32 get_count = 2;
  if (this->get_count() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt32Size(
        this->_internal_get_count());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void decree_buy::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:pb.decree.decree_buy)
  GOOGLE_DCHECK_NE(&from, this);
  const decree_buy* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<decree_buy>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:pb.decree.decree_buy)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:pb.decree.decree_buy)
    MergeFrom(*source);
  }
}

void decree_buy::MergeFrom(const decree_buy& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:pb.decree.decree_buy)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.error_code() != 0) {
    _internal_set_error_code(from._internal_error_code());
  }
  if (from.get_count() != 0) {
    _internal_set_get_count(from._internal_get_count());
  }
}

void decree_buy::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:pb.decree.decree_buy)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void decree_buy::CopyFrom(const decree_buy& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:pb.decree.decree_buy)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool decree_buy::IsInitialized() const {
  return true;
}

void decree_buy::InternalSwap(decree_buy* other) {
  using std::swap;
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(error_code_, other->error_code_);
  swap(get_count_, other->get_count_);
}

::PROTOBUF_NAMESPACE_ID::Metadata decree_buy::GetMetadata() const {
  return GetMetadataStatic();
}


// ===================================================================

void decree_reflush::InitAsDefaultInstance() {
}
class decree_reflush::_Internal {
 public:
};

decree_reflush::decree_reflush()
  : ::PROTOBUF_NAMESPACE_ID::Message(), _internal_metadata_(nullptr) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:pb.decree.decree_reflush)
}
decree_reflush::decree_reflush(const decree_reflush& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      _internal_metadata_(nullptr) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::memcpy(&next_time_, &from.next_time_,
    static_cast<size_t>(reinterpret_cast<char*>(&cur_count_) -
    reinterpret_cast<char*>(&next_time_)) + sizeof(cur_count_));
  // @@protoc_insertion_point(copy_constructor:pb.decree.decree_reflush)
}

void decree_reflush::SharedCtor() {
  ::memset(&next_time_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&cur_count_) -
      reinterpret_cast<char*>(&next_time_)) + sizeof(cur_count_));
}

decree_reflush::~decree_reflush() {
  // @@protoc_insertion_point(destructor:pb.decree.decree_reflush)
  SharedDtor();
}

void decree_reflush::SharedDtor() {
}

void decree_reflush::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const decree_reflush& decree_reflush::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_decree_reflush_decree_2eproto.base);
  return *internal_default_instance();
}


void decree_reflush::Clear() {
// @@protoc_insertion_point(message_clear_start:pb.decree.decree_reflush)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::memset(&next_time_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&cur_count_) -
      reinterpret_cast<char*>(&next_time_)) + sizeof(cur_count_));
  _internal_metadata_.Clear();
}

const char* decree_reflush::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // uint64 next_time = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          next_time_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // uint32 cur_count = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 16)) {
          cur_count_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint(&ptr);
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag, &_internal_metadata_, ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* decree_reflush::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:pb.decree.decree_reflush)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // uint64 next_time = 1;
  if (this->next_time() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt64ToArray(1, this->_internal_next_time(), target);
  }

  // uint32 cur_count = 2;
  if (this->cur_count() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteUInt32ToArray(2, this->_internal_cur_count(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:pb.decree.decree_reflush)
  return target;
}

size_t decree_reflush::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:pb.decree.decree_reflush)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // uint64 next_time = 1;
  if (this->next_time() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt64Size(
        this->_internal_next_time());
  }

  // uint32 cur_count = 2;
  if (this->cur_count() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::UInt32Size(
        this->_internal_cur_count());
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(
        _internal_metadata_, total_size, &_cached_size_);
  }
  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void decree_reflush::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:pb.decree.decree_reflush)
  GOOGLE_DCHECK_NE(&from, this);
  const decree_reflush* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<decree_reflush>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:pb.decree.decree_reflush)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:pb.decree.decree_reflush)
    MergeFrom(*source);
  }
}

void decree_reflush::MergeFrom(const decree_reflush& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:pb.decree.decree_reflush)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.next_time() != 0) {
    _internal_set_next_time(from._internal_next_time());
  }
  if (from.cur_count() != 0) {
    _internal_set_cur_count(from._internal_cur_count());
  }
}

void decree_reflush::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:pb.decree.decree_reflush)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void decree_reflush::CopyFrom(const decree_reflush& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:pb.decree.decree_reflush)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool decree_reflush::IsInitialized() const {
  return true;
}

void decree_reflush::InternalSwap(decree_reflush* other) {
  using std::swap;
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(next_time_, other->next_time_);
  swap(cur_count_, other->cur_count_);
}

::PROTOBUF_NAMESPACE_ID::Metadata decree_reflush::GetMetadata() const {
  return GetMetadataStatic();
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace decree
}  // namespace pb
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::pb::decree::decree_buy* Arena::CreateMaybeMessage< ::pb::decree::decree_buy >(Arena* arena) {
  return Arena::CreateInternal< ::pb::decree::decree_buy >(arena);
}
template<> PROTOBUF_NOINLINE ::pb::decree::decree_reflush* Arena::CreateMaybeMessage< ::pb::decree::decree_reflush >(Arena* arena) {
  return Arena::CreateInternal< ::pb::decree::decree_reflush >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>