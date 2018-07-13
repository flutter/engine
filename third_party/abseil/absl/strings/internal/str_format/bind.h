#ifndef ABSL_STRINGS_INTERNAL_STR_FORMAT_BIND_H_
#define ABSL_STRINGS_INTERNAL_STR_FORMAT_BIND_H_

#include <array>
#include <cstdio>
#include <sstream>
#include <string>

#include "absl/base/port.h"
#include "absl/container/inlined_vector.h"
#include "absl/strings/internal/str_format/arg.h"
#include "absl/strings/internal/str_format/checker.h"
#include "absl/strings/internal/str_format/parser.h"
#include "absl/types/span.h"

namespace absl {

class UntypedFormatSpec;

namespace str_format_internal {

class BoundConversion : public ConversionSpec {
 public:
  const FormatArgImpl* arg() const { return arg_; }
  void set_arg(const FormatArgImpl* a) { arg_ = a; }

 private:
  const FormatArgImpl* arg_;
};

// This is the type-erased class that the implementation uses.
class UntypedFormatSpecImpl {
 public:
  UntypedFormatSpecImpl() = delete;

  explicit UntypedFormatSpecImpl(string_view s) : str_(s), pc_() {}
  explicit UntypedFormatSpecImpl(
      const str_format_internal::ParsedFormatBase* pc)
      : pc_(pc) {}
  string_view str() const { return str_; }
  const str_format_internal::ParsedFormatBase* parsed_conversion() const {
    return pc_;
  }

  template <typename T>
  static const UntypedFormatSpecImpl& Extract(const T& s) {
    return s.spec_;
  }

 private:
  string_view str_;
  const str_format_internal::ParsedFormatBase* pc_;
};

template <typename T, typename...>
struct MakeDependent {
  using type = T;
};

// Implicitly convertible from `const char*`, `string_view`, and the
// `ExtendedParsedFormat` type. This abstraction allows all format functions to
// operate on any without providing too many overloads.
template <typename... Args>
class FormatSpecTemplate
    : public MakeDependent<UntypedFormatSpec, Args...>::type {
  using Base = typename MakeDependent<UntypedFormatSpec, Args...>::type;

 public:
#if ABSL_INTERNAL_ENABLE_FORMAT_CHECKER

  // Honeypot overload for when the std::string is not constexpr.
  // We use the 'unavailable' attribute to give a better compiler error than
  // just 'method is deleted'.
  FormatSpecTemplate(...)  // NOLINT
      __attribute__((unavailable("Format std::string is not constexpr.")));

  // Honeypot overload for when the format is constexpr and invalid.
  // We use the 'unavailable' attribute to give a better compiler error than
  // just 'method is deleted'.
  // To avoid checking the format twice, we just check that the format is
  // constexpr. If is it valid, then the overload below will kick in.
  // We add the template here to make this overload have lower priority.
  template <typename = void>
  FormatSpecTemplate(const char* s)  // NOLINT
      __attribute__((
          enable_if(str_format_internal::EnsureConstexpr(s), "constexpr trap"),
          unavailable(
              "Format specified does not match the arguments passed.")));

  template <typename T = void>
  FormatSpecTemplate(string_view s)  // NOLINT
      __attribute__((enable_if(str_format_internal::EnsureConstexpr(s),
                               "constexpr trap"))) {
    static_assert(sizeof(T*) == 0,
                  "Format specified does not match the arguments passed.");
  }

  // Good format overload.
  FormatSpecTemplate(const char* s)  // NOLINT
      __attribute__((enable_if(ValidFormatImpl<ArgumentToConv<Args>()...>(s),
                               "bad format trap")))
      : Base(s) {}

  FormatSpecTemplate(string_view s)  // NOLINT
      __attribute__((enable_if(ValidFormatImpl<ArgumentToConv<Args>()...>(s),
                               "bad format trap")))
      : Base(s) {}

#else  // ABSL_INTERNAL_ENABLE_FORMAT_CHECKER

  FormatSpecTemplate(const char* s) : Base(s) {}  // NOLINT
  FormatSpecTemplate(string_view s) : Base(s) {}  // NOLINT

#endif  // ABSL_INTERNAL_ENABLE_FORMAT_CHECKER

  template <Conv... C, typename = typename std::enable_if<
                           sizeof...(C) == sizeof...(Args) &&
                           AllOf(Contains(ArgumentToConv<Args>(),
                                          C)...)>::type>
  FormatSpecTemplate(const ExtendedParsedFormat<C...>& pc)  // NOLINT
      : Base(&pc) {}
};

template <typename... Args>
struct FormatSpecDeductionBarrier {
  using type = FormatSpecTemplate<Args...>;
};

class Streamable {
 public:
  Streamable(const UntypedFormatSpecImpl& format,
             absl::Span<const FormatArgImpl> args)
      : format_(format), args_(args.begin(), args.end()) {}

  std::ostream& Print(std::ostream& os) const;

  friend std::ostream& operator<<(std::ostream& os, const Streamable& l) {
    return l.Print(os);
  }

 private:
  const UntypedFormatSpecImpl& format_;
  absl::InlinedVector<FormatArgImpl, 4> args_;
};

// for testing
std::string Summarize(const UntypedFormatSpecImpl& format,
                 absl::Span<const FormatArgImpl> args);
bool BindWithPack(const UnboundConversion* props,
                  absl::Span<const FormatArgImpl> pack, BoundConversion* bound);

bool FormatUntyped(FormatRawSinkImpl raw_sink,
                   const UntypedFormatSpecImpl& format,
                   absl::Span<const FormatArgImpl> args);

std::string& AppendPack(std::string* out, const UntypedFormatSpecImpl& format,
                   absl::Span<const FormatArgImpl> args);

inline std::string FormatPack(const UntypedFormatSpecImpl& format,
                         absl::Span<const FormatArgImpl> args) {
  std::string out;
  AppendPack(&out, format, args);
  return out;
}

int FprintF(std::FILE* output, const UntypedFormatSpecImpl& format,
            absl::Span<const FormatArgImpl> args);
int SnprintF(char* output, size_t size, const UntypedFormatSpecImpl& format,
             absl::Span<const FormatArgImpl> args);

// Returned by Streamed(v). Converts via '%s' to the std::string created
// by std::ostream << v.
template <typename T>
class StreamedWrapper {
 public:
  explicit StreamedWrapper(const T& v) : v_(v) { }

 private:
  template <typename S>
  friend ConvertResult<Conv::s> FormatConvertImpl(const StreamedWrapper<S>& v,
                                                  const ConversionSpec& conv,
                                                  FormatSinkImpl* out);
  const T& v_;
};

}  // namespace str_format_internal
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_STR_FORMAT_BIND_H_
