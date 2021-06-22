// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <fuchsia/diagnostics/stream/cpp/fidl.h>
#include <fuchsia/logger/cpp/fidl.h>
#include <lib/fdio/directory.h>
#include <lib/fdio/fd.h>
#include <lib/fdio/fdio.h>
#include <lib/stdcompat/variant.h>
#include <lib/syslog/cpp/log_level.h>
#include <lib/syslog/cpp/logging_backend.h>
#include <lib/syslog/cpp/logging_backend_fuchsia_globals.h>
#include <lib/syslog/streams/cpp/encode.h>
#include <lib/zx/channel.h>
#include <lib/zx/process.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "lib/syslog/streams/cpp/fields.h"
#include "logging_backend_fuchsia_private.h"
#include "logging_backend_shared.h"
#include "macros.h"
namespace syslog_backend {

// Flushes a record to the legacy fx_logger, if available.
// Returns true on success.
bool fx_log_compat_flush_record(LogBuffer* buffer);

// Attempts to reconfigure the legacy fx_logger if available.
// Returns the file descriptor if one was configured.
// A return value of -1 indicates that no file was opened.
int fx_log_compat_reconfigure(syslog::LogSettings& settings,
                              const std::initializer_list<std::string>& tags);

bool HasStructuredBackend() { return true; }

using log_word_t = uint64_t;

zx_koid_t GetKoid(zx_handle_t handle) {
  zx_info_handle_basic_t info;
  // We need to use _zx_object_get_info to avoid breaking the driver ABI.
  // fake_ddk can fake out this method, which results in us deadlocking
  // when used in certain drivers because the fake doesn't properly pass-through
  // to the real syscall in this case.
  zx_status_t status =
      _zx_object_get_info(handle, ZX_INFO_HANDLE_BASIC, &info, sizeof(info), nullptr, nullptr);
  return status == ZX_OK ? info.koid : ZX_KOID_INVALID;
}

static zx_koid_t pid = GetKoid(zx_process_self());
static thread_local zx_koid_t tid = GetCurrentThreadKoid();

// Represents a slice of a buffer of type T.
template <typename T>
class DataSlice {
 public:
  DataSlice(T* ptr, WordOffset<T> slice) : ptr_(ptr), slice_(slice) {}

  T& operator[](WordOffset<T> offset) {
    offset.AssertValid();
    return ptr_[offset];
  }

  const T& operator[](WordOffset<T> offset) const { return ptr_[offset.unsafe_get()]; }

  WordOffset<T> slice() { return slice_; }

  T* data() { return ptr_; }

 private:
  T* ptr_;
  WordOffset<T> slice_;
};

static DataSlice<const char> SliceFromString(const std::string& string) {
  return DataSlice<const char>(
      string.data(), WordOffset<const char>::FromByteOffset(ByteOffset::Unbounded(string.size())));
}

template <typename T, size_t size>
static DataSlice<const T> SliceFromArray(const T (&array)[size]) {
  return DataSlice<const T>(array, size);
}

template <size_t size>
static DataSlice<const char> SliceFromArray(const char (&array)[size]) {
  return DataSlice<const char>(
      array, WordOffset<const char>::FromByteOffset(ByteOffset::Unbounded(size - 1)));
}

static constexpr int WORD_SIZE = sizeof(log_word_t);  // See sdk/lib/syslog/streams/cpp/encode.cc
class DataBuffer {
 public:
  static constexpr auto kBufferSize = (1 << 15) / WORD_SIZE;
  void Write(const log_word_t* data, WordOffset<log_word_t> length) {
    for (size_t i = 0; i < length.unsafe_get(); i++) {
      buffer_[cursor_.unsafe_get() + i] = data[i];
    }
    cursor_ = cursor_ + length;
  }

  WordOffset<log_word_t> WritePadded(const void* msg, const ByteOffset& length) {
    auto retval = WritePaddedInternal(&buffer_[cursor_.unsafe_get()], msg, length);
    cursor_ = cursor_ + retval;
    return retval;
  }

  template <typename T>
  void Write(const T& data) {
    static_assert(sizeof(T) >= sizeof(log_word_t));
    static_assert(alignof(T) >= sizeof(log_word_t));
    Write(reinterpret_cast<const log_word_t*>(&data), cursor_ + (sizeof(T) / sizeof(log_word_t)));
  }

  DataSlice<log_word_t> GetSlice() { return DataSlice<log_word_t>(buffer_, cursor_); }

 private:
  WordOffset<log_word_t> cursor_;
  log_word_t buffer_[kBufferSize];
};

struct RecordState {
  RecordState()
      : arg_size(WordOffset<log_word_t>::FromByteOffset(
            ByteOffset::FromBuffer(0, sizeof(LogBuffer::data)))),
        current_key_size(ByteOffset::FromBuffer(0, sizeof(LogBuffer::data))),
        cursor(WordOffset<log_word_t>::FromByteOffset(
            ByteOffset::FromBuffer(0, sizeof(LogBuffer::data)))) {}
  // Header of the record itself
  uint64_t* header;
  syslog::LogSeverity log_severity;
  ::fuchsia::diagnostics::Severity severity;
  // arg_size in words
  WordOffset<log_word_t> arg_size;
  zx_handle_t socket = ZX_HANDLE_INVALID;
  // key_size in bytes
  ByteOffset current_key_size;
  // Header of the current argument being encoded
  uint64_t* current_header_position = 0;
  uint32_t dropped_count = 0;
  // Current position (in 64-bit words) into the buffer.
  WordOffset<log_word_t> cursor;
  // True if encoding was successful, false otherwise
  bool encode_success = true;
  // Message string -- valid if severity is FATAL. For FATAL
  // logs the caller is responsible for ensuring the string
  // is valid for the duration of the call (which our macros
  // will ensure for current users).
  const char* msg_string;
  static RecordState* CreatePtr(LogBuffer* buffer) {
    return reinterpret_cast<RecordState*>(&buffer->record_state);
  }
  size_t PtrToIndex(void* ptr) const {
    return reinterpret_cast<size_t>(static_cast<uint8_t*>(ptr)) - reinterpret_cast<size_t>(header);
  }
};
static_assert(sizeof(RecordState) <= sizeof(LogBuffer::record_state));
static_assert(std::alignment_of<RecordState>() == sizeof(uint64_t));

// Used for accessing external data buffers provided by clients.
// Used by the Encoder to do in-place encoding of data
class ExternalDataBuffer {
 public:
  explicit ExternalDataBuffer(LogBuffer* buffer)
      : buffer_(&buffer->data[0]), cursor_(RecordState::CreatePtr(buffer)->cursor) {}

  ExternalDataBuffer(log_word_t* data, size_t length, WordOffset<log_word_t>& cursor)
      : buffer_(data), cursor_(cursor) {}
  __WARN_UNUSED_RESULT bool Write(const log_word_t* data, WordOffset<log_word_t> length) {
    if (!cursor_.in_bounds(length)) {
      return false;
    }
    for (size_t i = 0; i < length.unsafe_get(); i++) {
      buffer_[(cursor_ + i).unsafe_get()] = data[i];
    }
    cursor_ = cursor_ + length;
    return true;
  }

  __WARN_UNUSED_RESULT bool WritePadded(const void* msg, const ByteOffset& byte_count,
                                        WordOffset<log_word_t>* written) {
    assert(written != nullptr);
    WordOffset<log_word_t> word_count = cursor_.begin().AddPadded(byte_count);
    if (!cursor_.in_bounds(word_count)) {
      return false;
    }
    auto retval = WritePaddedInternal(buffer_ + cursor_.unsafe_get(), msg, byte_count);
    cursor_ = cursor_ + retval;
    *written = retval;
    return true;
  }

  template <typename T>
  __WARN_UNUSED_RESULT bool Write(const T& data) {
    static_assert(sizeof(T) >= sizeof(log_word_t));
    static_assert(alignof(T) >= sizeof(log_word_t));
    return Write(reinterpret_cast<const log_word_t*>(&data),
                 WordOffset<log_word_t>::FromByteOffset(
                     ByteOffset::Unbounded((sizeof(T) / sizeof(log_word_t)) * sizeof(log_word_t))));
  }

  uint64_t* data() { return buffer_ + cursor_.unsafe_get(); }

  DataSlice<log_word_t> GetSlice() { return DataSlice<log_word_t>(buffer_, cursor_); }

 private:
  // Start of buffer
  log_word_t* buffer_ = nullptr;
  // Current location in buffer (in words)
  WordOffset<log_word_t>& cursor_;
};

template <typename T>
class Encoder {
 public:
  explicit Encoder(T& buffer) { buffer_ = &buffer; }

  void Begin(RecordState& state, zx_time_t timestamp, ::fuchsia::diagnostics::Severity severity) {
    state.severity = severity;
    state.header = buffer_->data();
    log_word_t empty_header = 0;
    state.encode_success &= buffer_->Write(empty_header);
    state.encode_success &= buffer_->Write(timestamp);
  }

  void FlushPreviousArgument(RecordState& state) { state.arg_size.reset(); }

  void AppendArgumentKey(RecordState& state, DataSlice<const char> key) {
    FlushPreviousArgument(state);
    auto header_position = buffer_->data();
    log_word_t empty_header = 0;
    state.encode_success &= buffer_->Write(empty_header);
    WordOffset<log_word_t> s_size =
        WordOffset<log_word_t>::FromByteOffset(ByteOffset::Unbounded(0));
    state.encode_success &= buffer_->WritePadded(key.data(), key.slice().ToByteOffset(), &s_size);
    state.arg_size = s_size + 1;  // offset by 1 for the header
    state.current_key_size = key.slice().ToByteOffset();
    state.current_header_position = header_position;
  }

  uint64_t ComputeArgHeader(RecordState& state, int type, uint64_t value_ref = 0) {
    return ArgumentFields::Type::Make(type) |
           ArgumentFields::SizeWords::Make(state.arg_size.unsafe_get()) |
           ArgumentFields::NameRefVal::Make(state.current_key_size.unsafe_get()) |
           ArgumentFields::NameRefMSB::Make(state.current_key_size.unsafe_get() > 0 ? 1 : 0) |
           ArgumentFields::ValueRef::Make(value_ref) | ArgumentFields::Reserved::Make(0);
  }

  void AppendArgumentValue(RecordState& state, int64_t value) {
    int type = 3;
    state.encode_success &= buffer_->Write(value);
    state.arg_size = state.arg_size + 1;
    *state.current_header_position = ComputeArgHeader(state, type);
  }

  void AppendArgumentValue(RecordState& state, uint64_t value) {
    int type = 4;
    state.encode_success &= buffer_->Write(value);
    state.arg_size = state.arg_size + 1;
    *state.current_header_position = ComputeArgHeader(state, type);
  }

  void AppendArgumentValue(RecordState& state, double value) {
    int type = 5;
    state.encode_success &= buffer_->Write(value);
    state.arg_size = state.arg_size + 1;
    *state.current_header_position = ComputeArgHeader(state, type);
  }

  void AppendArgumentValue(RecordState& state, DataSlice<const char> string) {
    int type = 6;
    WordOffset<log_word_t> written =
        WordOffset<log_word_t>::FromByteOffset(ByteOffset::Unbounded(0));
    state.encode_success &=
        buffer_->WritePadded(string.data(), string.slice().ToByteOffset(), &written);
    state.arg_size = state.arg_size + written;
    *state.current_header_position = ComputeArgHeader(
        state, type, string.slice().unsafe_get() > 0 ? (1 << 15) | string.slice().unsafe_get() : 0);
  }

  void End(RecordState& state) {
    // See src/lib/diagnostics/stream/rust/src/lib.rs
    constexpr auto kTracingFormatLogRecordType = 9;
    FlushPreviousArgument(state);
    uint64_t header =
        HeaderFields::Type::Make(kTracingFormatLogRecordType) |
        HeaderFields::SizeWords::Make(static_cast<size_t>(buffer_->data() - state.header)) |
        HeaderFields::Reserved::Make(0) | HeaderFields::Severity::Make(state.severity);
    *state.header = header;
  }

 private:
  T* buffer_;
};

const size_t kMaxTags = 4;  // Legacy from ulib/syslog. Might be worth rethinking.
// Compiler thinks this is unused even though WriteLogToSocket uses it.
__UNUSED const char kMessageFieldName[] = "message";
const char kPrintfFieldName[] = "printf";
const char kVerbosityFieldName[] = "verbosity";
const char kPidFieldName[] = "pid";
const char kTidFieldName[] = "tid";
const char kDroppedLogsFieldName[] = "dropped_logs";
const char kTagFieldName[] = "tag";
const char kFileFieldName[] = "file";
const char kLineFieldName[] = "line";

class LogState {
 public:
  static void Set(const syslog::LogSettings& settings);
  static void Set(const syslog::LogSettings& settings,
                  const std::initializer_list<std::string>& tags);
  static const LogState& Get();

  syslog::LogSeverity min_severity() const { return min_severity_; }

  template <typename T>
  void WriteLog(syslog::LogSeverity severity, const char* file_name, unsigned int line,
                const char* msg, const char* condition, const T& value) const;
  const std::string* tags() const { return tags_; }
  size_t tag_count() const { return num_tags_; }
  // Allowed to be const because descriptor_ is mutable
  cpp17::variant<zx::socket, std::ofstream>& descriptor() const { return descriptor_; }

  cpp17::optional<int> fd() const { return fd_; }

 private:
  LogState(const syslog::LogSettings& settings, const std::initializer_list<std::string>& tags);
  bool WriteLogToFile(std::ofstream* file_ptr, zx_time_t time, zx_koid_t pid, zx_koid_t tid,
                      syslog::LogSeverity severity, const char* file_name, unsigned int line,
                      const char* tag, const char* condition, const std::string& msg) const;

  syslog::LogSeverity min_severity_;
  cpp17::optional<int> fd_;
  zx_koid_t pid_;
  mutable cpp17::variant<zx::socket, std::ofstream> descriptor_ = zx::socket();
  std::string tags_[kMaxTags];
  std::string tag_str_;
  size_t num_tags_ = 0;
};

void BeginRecordInternal(LogBuffer* buffer, syslog::LogSeverity severity, const char* file_name,
                         unsigned int line, const char* msg, const char* condition, bool is_printf,
                         zx_handle_t socket) {
  // Ensure we have log state
  auto& log_state = LogState::Get();
  cpp17::optional<int8_t> raw_severity;
  if (log_state.fd() != -1) {
    BeginRecordLegacy(buffer, severity, file_name, line, msg, condition);
    return;
  }
  // Validate that the severity matches the FIDL definition in
  // sdk/fidl/fuchsia.diagnostics/severity.fidl.
  if ((severity % 0x10) || (severity > 0x60) || (severity < 0x10)) {
    raw_severity = severity;
    severity = syslog::LOG_DEBUG;
  }
  zx_time_t time = zx_clock_get_monotonic();
  auto* state = RecordState::CreatePtr(buffer);
  RecordState& record = *state;
  // Invoke the constructor of RecordState to construct a valid RecordState
  // inside the LogBuffer.
  new (state) RecordState;
  if (socket != ZX_HANDLE_INVALID) {
    state->socket = socket;
  } else {
    state->socket = cpp17::get<zx::socket>(log_state.descriptor()).get();
  }
  if (severity == syslog::LOG_FATAL) {
    state->msg_string = msg;
  }
  state->log_severity = severity;
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  encoder.Begin(*state, time, ::fuchsia::diagnostics::Severity(severity));
  if (is_printf) {
    encoder.AppendArgumentKey(record, SliceFromArray(kPrintfFieldName));
    encoder.AppendArgumentValue(record, static_cast<uint64_t>(0));
  }
  encoder.AppendArgumentKey(record, SliceFromArray(kPidFieldName));
  encoder.AppendArgumentValue(record, static_cast<uint64_t>(pid));
  encoder.AppendArgumentKey(record, SliceFromArray(kTidFieldName));
  encoder.AppendArgumentValue(record, static_cast<uint64_t>(tid));

  auto dropped_count = GetAndResetDropped();
  record.dropped_count = dropped_count;
  if (raw_severity) {
    encoder.AppendArgumentKey(record, SliceFromString(kVerbosityFieldName));
    encoder.AppendArgumentValue(record, static_cast<int64_t>(raw_severity.value()));
  }
  if (dropped_count) {
    encoder.AppendArgumentKey(record, SliceFromString(kDroppedLogsFieldName));
    encoder.AppendArgumentValue(record, static_cast<uint64_t>(dropped_count));
  }
  for (size_t i = 0; i < GetState()->tag_count(); i++) {
    encoder.AppendArgumentKey(record, SliceFromString(kTagFieldName));
    encoder.AppendArgumentValue(record, SliceFromString(GetState()->tags()[i]));
  }
  if (msg) {
    encoder.AppendArgumentKey(record, SliceFromString(kMessageFieldName));
    encoder.AppendArgumentValue(record, SliceFromString(msg));
  }
  if (file_name) {
    encoder.AppendArgumentKey(record, SliceFromString(kFileFieldName));
    encoder.AppendArgumentValue(record, SliceFromString(file_name));
  }
  encoder.AppendArgumentKey(record, SliceFromString(kLineFieldName));
  encoder.AppendArgumentValue(record, static_cast<uint64_t>(line));
}

void BeginRecordPrintf(LogBuffer* buffer, syslog::LogSeverity severity, const char* file_name,
                       unsigned int line, const char* msg) {
  BeginRecordInternal(buffer, severity, file_name, line, msg, nullptr, true /* is_printf */,
                      ZX_HANDLE_INVALID);
}

void BeginRecord(LogBuffer* buffer, syslog::LogSeverity severity, const char* file_name,
                 unsigned int line, const char* msg, const char* condition) {
  BeginRecordInternal(buffer, severity, file_name, line, msg, condition, false /* is_printf */,
                      ZX_HANDLE_INVALID);
}

void BeginRecordWithSocket(LogBuffer* buffer, syslog::LogSeverity severity, const char* file_name,
                           unsigned int line, const char* msg, const char* condition,
                           zx_handle_t socket) {
  BeginRecordInternal(buffer, severity, file_name, line, msg, condition, false /* is_printf */,
                      socket);
}

void WriteKeyValue(LogBuffer* buffer, const char* key, const char* value) {
  auto& log_state = LogState::Get();
  if (log_state.fd() != -1) {
    WriteKeyValueLegacy(buffer, key, value);
    return;
  }
  auto* state = RecordState::CreatePtr(buffer);
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  encoder.AppendArgumentKey(
      *state, DataSlice<const char>(
                  key, WordOffset<const char>::FromByteOffset(ByteOffset::Unbounded(strlen(key)))));
  encoder.AppendArgumentValue(
      *state, DataSlice<const char>(value, WordOffset<const char>::FromByteOffset(
                                               ByteOffset::Unbounded(strlen(value)))));
}

void WriteKeyValue(LogBuffer* buffer, const char* key, int64_t value) {
  auto& log_state = LogState::Get();
  if (log_state.fd() != -1) {
    WriteKeyValueLegacy(buffer, key, value);
    return;
  }
  auto* state = RecordState::CreatePtr(buffer);
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  encoder.AppendArgumentKey(
      *state, DataSlice<const char>(
                  key, WordOffset<const char>::FromByteOffset(ByteOffset::Unbounded(strlen(key)))));
  encoder.AppendArgumentValue(*state, value);
}

void WriteKeyValue(LogBuffer* buffer, const char* key, uint64_t value) {
  auto& log_state = LogState::Get();
  if (log_state.fd() != -1) {
    WriteKeyValueLegacy(buffer, key, value);
    return;
  }
  auto* state = RecordState::CreatePtr(buffer);
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  encoder.AppendArgumentKey(
      *state, DataSlice<const char>(
                  key, WordOffset<const char>::FromByteOffset(ByteOffset::Unbounded(strlen(key)))));
  encoder.AppendArgumentValue(*state, value);
}

void WriteKeyValue(LogBuffer* buffer, const char* key, double value) {
  auto& log_state = LogState::Get();
  if (log_state.fd() != -1) {
    WriteKeyValueLegacy(buffer, key, value);
    return;
  }
  auto* state = RecordState::CreatePtr(buffer);
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  encoder.AppendArgumentKey(
      *state, DataSlice<const char>(
                  key, WordOffset<const char>::FromByteOffset(ByteOffset::Unbounded(strlen(key)))));
  encoder.AppendArgumentValue(*state, value);
}

void EndRecord(LogBuffer* buffer) {
  auto& log_state = LogState::Get();
  if (log_state.fd() != -1) {
    EndRecordLegacy(buffer);
    return;
  }
  auto* state = RecordState::CreatePtr(buffer);
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  encoder.End(*state);
}

bool FlushRecord(LogBuffer* buffer) {
  auto& log_state = LogState::Get();
  if (log_state.fd() != -1) {
    return fx_log_compat_flush_record(buffer);
  }
  auto* state = RecordState::CreatePtr(buffer);
  if (!state->encode_success) {
    return false;
  }
  ExternalDataBuffer external_buffer(buffer);
  Encoder<ExternalDataBuffer> encoder(external_buffer);
  auto slice = external_buffer.GetSlice();
  auto status = zx_socket_write(state->socket, 0, slice.data(),
                                slice.slice().ToByteOffset().unsafe_get(), nullptr);
  if (status != ZX_OK) {
    AddDropped(state->dropped_count + 1);
  }
  if (state->log_severity == syslog::LOG_FATAL) {
    std::cerr << state->msg_string << std::endl;
    abort();
  }
  return status != ZX_ERR_BAD_STATE && status != ZX_ERR_PEER_CLOSED &&
         state->log_severity != syslog::LOG_FATAL;
}

bool LogState::WriteLogToFile(std::ofstream* file_ptr, zx_time_t time, zx_koid_t pid, zx_koid_t tid,
                              syslog::LogSeverity severity, const char* file_name,
                              unsigned int line, const char* tag, const char* condition,
                              const std::string& msg) const {
  auto& file = *file_ptr;
  file << "[" << std::setw(5) << std::setfill('0') << time / 1000000000UL << "." << std::setw(6)
       << (time / 1000UL) % 1000000UL << std::setw(0) << "][" << pid << "][" << tid << "][";

  auto& tag_str = tag_str_;
  file << tag_str;

  if (tag) {
    if (!tag_str.empty()) {
      file << ", ";
    }

    file << tag;
  }

  file << "] ";

  switch (severity) {
    case syslog::LOG_TRACE:
      file << "TRACE";
      break;
    case syslog::LOG_DEBUG:
      file << "DEBUG";
      break;
    case syslog::LOG_INFO:
      file << "INFO";
      break;
    case syslog::LOG_WARNING:
      file << "WARNING";
      break;
    case syslog::LOG_ERROR:
      file << "ERROR";
      break;
    case syslog::LOG_FATAL:
      file << "FATAL";
      break;
    default:
      file << "VLOG(" << (syslog::LOG_INFO - severity) << ")";
  }

  file << ": [" << file_name << "(" << line << ")] " << msg << std::endl;

  return severity != syslog::LOG_FATAL;
}

const LogState& LogState::Get() {
  auto state = GetState();

  if (!state) {
    Set(syslog::LogSettings());
    state = GetState();
  }

  return *state;
}

void LogState::Set(const syslog::LogSettings& settings) { Set(settings, {}); }

void LogState::Set(const syslog::LogSettings& settings,
                   const std::initializer_list<std::string>& tags) {
  if (auto old = SetState(new LogState(settings, tags))) {
    delete old;
  }
}

LogState::LogState(const syslog::LogSettings& in_settings,
                   const std::initializer_list<std::string>& tags)
    : min_severity_(in_settings.min_log_level), fd_(in_settings.log_fd), pid_(pid) {
  syslog::LogSettings settings = in_settings;
  min_severity_ = in_settings.min_log_level;

  std::ostringstream tag_str;

  for (auto& tag : tags) {
    if (num_tags_) {
      tag_str << ", ";
    }
    tag_str << tag;
    tags_[num_tags_++] = tag;
    if (num_tags_ >= kMaxTags)
      break;
  }

  tag_str_ = tag_str.str();

  std::ofstream file;
  if (!settings.log_file.empty()) {
    fd_ = fx_log_compat_reconfigure(settings, tags);
  }
  if (fd_ != -1) {
    return;
  }
  if (file.is_open()) {
    descriptor_ = std::move(file);
  } else {
    zx::channel logger, logger_request;
    if (zx::channel::create(0, &logger, &logger_request) != ZX_OK) {
      return;
    }
    ::fuchsia::logger::LogSink_SyncProxy logger_client(std::move(logger));
    if (fdio_service_connect("/svc/fuchsia.logger.LogSink", logger_request.release()) != ZX_OK) {
      return;
    }
    zx::socket local, remote;
    if (zx::socket::create(ZX_SOCKET_DATAGRAM, &local, &remote) != ZX_OK) {
      return;
    }

    auto result = logger_client.ConnectStructured(std::move(remote));
    if (result != ZX_OK) {
      return;
    }

    descriptor_ = std::move(local);
  }
}

template <typename T>
void LogState::WriteLog(syslog::LogSeverity severity, const char* file_name, unsigned int line,
                        const char* msg, const char* condition, const T& value) const {
  zx_time_t time = zx_clock_get_monotonic();

  // Cached getter for a stringified version of the log message, so we stringify at most once.
  auto msg_str = [as_str = std::string(), condition, msg, &value]() mutable -> const std::string& {
    if (as_str.empty()) {
      as_str = FullMessageString(condition, msg, value);
    }

    return as_str;
  };

  if (cpp17::holds_alternative<std::ofstream>(descriptor_)) {
    auto& file = cpp17::get<std::ofstream>(descriptor_);
    if (WriteLogToFile(&file, time, pid_, tid, severity, file_name, line, nullptr, condition,
                       msg_str())) {
      return;
    }
  } else if (cpp17::holds_alternative<zx::socket>(descriptor_)) {
    auto& socket = cpp17::get<zx::socket>(descriptor_);
    std::string message;
    if (msg) {
      message.assign(msg);
    }
    if (WriteLogToSocket(&socket, time, pid_, tid, severity, file_name, line, message, condition,
                         value)) {
      return;
    }
  }

  std::cerr << msg_str() << std::endl;
}

void SetLogSettings(const syslog::LogSettings& settings) { LogState::Set(settings); }

void SetLogSettings(const syslog::LogSettings& settings,
                    const std::initializer_list<std::string>& tags) {
  LogState::Set(settings, tags);
}

syslog::LogSeverity GetMinLogLevel() { return LogState::Get().min_severity(); }

}  // namespace syslog_backend
