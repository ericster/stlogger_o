// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_DMESGINFO_COM_EOLWRAL_OSMONITOR_CORE_H_
#define FLATBUFFERS_GENERATED_DMESGINFO_COM_EOLWRAL_OSMONITOR_CORE_H_

#include "flatbuffers/flatbuffers.h"


namespace com {
namespace eolwral {
namespace osmonitor {
namespace core {

struct dmesgInfo;
struct dmesgInfoList;

enum dmesgLevel {
  /// emergency 
  dmesgLevel_EMERGENCY = 0,
  /// alert 
  dmesgLevel_ALERT = 1,
  /// critical 
  dmesgLevel_CRITICAL = 2,
  /// error 
  dmesgLevel_ERROR = 3,
  /// warning 
  dmesgLevel_WARNING = 4,
  /// notice 
  dmesgLevel_NOTICE = 5,
  /// information 
  dmesgLevel_INFORMATION = 6,
  /// debug 
  dmesgLevel_DEBUG = 7
};

inline const char **EnumNamesdmesgLevel() {
  static const char *names[] = { "EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFORMATION", "DEBUG", nullptr };
  return names;
}

inline const char *EnumNamedmesgLevel(dmesgLevel e) { return EnumNamesdmesgLevel()[e]; }

struct dmesgInfo : private flatbuffers::Table {
  /// message level 
  dmesgLevel level() const { return static_cast<dmesgLevel>(GetField<int8_t>(4, 6)); }
  /// passed seconds since boot 
  uint64_t seconds() const { return GetField<uint64_t>(6, 0); }
  /// message 
  const flatbuffers::String *message() const { return GetPointer<const flatbuffers::String *>(8); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, 4 /* level */) &&
           VerifyField<uint64_t>(verifier, 6 /* seconds */) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 8 /* message */) &&
           verifier.Verify(message()) &&
           verifier.EndTable();
  }
};

struct dmesgInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_level(dmesgLevel level) { fbb_.AddElement<int8_t>(4, static_cast<int8_t>(level), 6); }
  void add_seconds(uint64_t seconds) { fbb_.AddElement<uint64_t>(6, seconds, 0); }
  void add_message(flatbuffers::Offset<flatbuffers::String> message) { fbb_.AddOffset(8, message); }
  dmesgInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  dmesgInfoBuilder &operator=(const dmesgInfoBuilder &);
  flatbuffers::Offset<dmesgInfo> Finish() {
    auto o = flatbuffers::Offset<dmesgInfo>(fbb_.EndTable(start_, 3));
    return o;
  }
};

inline flatbuffers::Offset<dmesgInfo> CreatedmesgInfo(flatbuffers::FlatBufferBuilder &_fbb,
   dmesgLevel level = dmesgLevel_INFORMATION,
   uint64_t seconds = 0,
   flatbuffers::Offset<flatbuffers::String> message = 0) {
  dmesgInfoBuilder builder_(_fbb);
  builder_.add_seconds(seconds);
  builder_.add_message(message);
  builder_.add_level(level);
  return builder_.Finish();
}

struct dmesgInfoList : private flatbuffers::Table {
  const flatbuffers::Vector<flatbuffers::Offset<dmesgInfo>> *list() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<dmesgInfo>> *>(4); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 4 /* list */) &&
           verifier.Verify(list()) &&
           verifier.VerifyVectorOfTables(list()) &&
           verifier.EndTable();
  }
};

struct dmesgInfoListBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_list(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<dmesgInfo>>> list) { fbb_.AddOffset(4, list); }
  dmesgInfoListBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  dmesgInfoListBuilder &operator=(const dmesgInfoListBuilder &);
  flatbuffers::Offset<dmesgInfoList> Finish() {
    auto o = flatbuffers::Offset<dmesgInfoList>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<dmesgInfoList> CreatedmesgInfoList(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<dmesgInfo>>> list = 0) {
  dmesgInfoListBuilder builder_(_fbb);
  builder_.add_list(list);
  return builder_.Finish();
}

inline const dmesgInfoList *GetdmesgInfoList(const void *buf) { return flatbuffers::GetRoot<dmesgInfoList>(buf); }

inline bool VerifydmesgInfoListBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<dmesgInfoList>(); }

inline void FinishdmesgInfoListBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<dmesgInfoList> root) { fbb.Finish(root); }

}  // namespace core
}  // namespace osmonitor
}  // namespace st
}  // namespace com

#endif  // FLATBUFFERS_GENERATED_DMESGINFO_COM_EOLWRAL_OSMONITOR_CORE_H_