// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: fnf_friends.proto

#ifndef PROTOBUF_fnf_5ffriends_2eproto__INCLUDED
#define PROTOBUF_fnf_5ffriends_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace FNF {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_fnf_5ffriends_2eproto();
void protobuf_AssignDesc_fnf_5ffriends_2eproto();
void protobuf_ShutdownFile_fnf_5ffriends_2eproto();

class user_friends;
class friend_info;
class helpitem;

enum EM_FRIGIFT_STATUS {
  EM_FRI_GIFT_CANGOT = 0,
  EM_FRI_GIFT_GOT = 1,
  EM_FRI_GIFT_INVITED = 2,
  EM_FRI_GIFT_CANINVITE = 3
};
bool EM_FRIGIFT_STATUS_IsValid(int value);
const EM_FRIGIFT_STATUS EM_FRIGIFT_STATUS_MIN = EM_FRI_GIFT_CANGOT;
const EM_FRIGIFT_STATUS EM_FRIGIFT_STATUS_MAX = EM_FRI_GIFT_CANINVITE;
const int EM_FRIGIFT_STATUS_ARRAYSIZE = EM_FRIGIFT_STATUS_MAX + 1;

const ::google::protobuf::EnumDescriptor* EM_FRIGIFT_STATUS_descriptor();
inline const ::std::string& EM_FRIGIFT_STATUS_Name(EM_FRIGIFT_STATUS value) {
  return ::google::protobuf::internal::NameOfEnum(
    EM_FRIGIFT_STATUS_descriptor(), value);
}
inline bool EM_FRIGIFT_STATUS_Parse(
    const ::std::string& name, EM_FRIGIFT_STATUS* value) {
  return ::google::protobuf::internal::ParseNamedEnum<EM_FRIGIFT_STATUS>(
    EM_FRIGIFT_STATUS_descriptor(), name, value);
}
// ===================================================================

class user_friends : public ::google::protobuf::Message {
 public:
  user_friends();
  virtual ~user_friends();

  user_friends(const user_friends& from);

  inline user_friends& operator=(const user_friends& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const user_friends& default_instance();

  void Swap(user_friends* other);

  // implements Message ----------------------------------------------

  user_friends* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const user_friends& from);
  void MergeFrom(const user_friends& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required int64 uid = 1;
  inline bool has_uid() const;
  inline void clear_uid();
  static const int kUidFieldNumber = 1;
  inline ::google::protobuf::int64 uid() const;
  inline void set_uid(::google::protobuf::int64 value);

  // repeated .FNF.friend_info friends = 2;
  inline int friends_size() const;
  inline void clear_friends();
  static const int kFriendsFieldNumber = 2;
  inline const ::FNF::friend_info& friends(int index) const;
  inline ::FNF::friend_info* mutable_friends(int index);
  inline ::FNF::friend_info* add_friends();
  inline const ::google::protobuf::RepeatedPtrField< ::FNF::friend_info >&
      friends() const;
  inline ::google::protobuf::RepeatedPtrField< ::FNF::friend_info >*
      mutable_friends();

  // repeated .FNF.helpitem help_items = 3;
  inline int help_items_size() const;
  inline void clear_help_items();
  static const int kHelpItemsFieldNumber = 3;
  inline const ::FNF::helpitem& help_items(int index) const;
  inline ::FNF::helpitem* mutable_help_items(int index);
  inline ::FNF::helpitem* add_help_items();
  inline const ::google::protobuf::RepeatedPtrField< ::FNF::helpitem >&
      help_items() const;
  inline ::google::protobuf::RepeatedPtrField< ::FNF::helpitem >*
      mutable_help_items();

  // optional int32 help_gift_count = 4;
  inline bool has_help_gift_count() const;
  inline void clear_help_gift_count();
  static const int kHelpGiftCountFieldNumber = 4;
  inline ::google::protobuf::int32 help_gift_count() const;
  inline void set_help_gift_count(::google::protobuf::int32 value);

  // optional int32 help_gift_time = 5;
  inline bool has_help_gift_time() const;
  inline void clear_help_gift_time();
  static const int kHelpGiftTimeFieldNumber = 5;
  inline ::google::protobuf::int32 help_gift_time() const;
  inline void set_help_gift_time(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:FNF.user_friends)
 private:
  inline void set_has_uid();
  inline void clear_has_uid();
  inline void set_has_help_gift_count();
  inline void clear_has_help_gift_count();
  inline void set_has_help_gift_time();
  inline void clear_has_help_gift_time();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int64 uid_;
  ::google::protobuf::RepeatedPtrField< ::FNF::friend_info > friends_;
  ::google::protobuf::RepeatedPtrField< ::FNF::helpitem > help_items_;
  ::google::protobuf::int32 help_gift_count_;
  ::google::protobuf::int32 help_gift_time_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(5 + 31) / 32];

  friend void  protobuf_AddDesc_fnf_5ffriends_2eproto();
  friend void protobuf_AssignDesc_fnf_5ffriends_2eproto();
  friend void protobuf_ShutdownFile_fnf_5ffriends_2eproto();

  void InitAsDefaultInstance();
  static user_friends* default_instance_;
};
// -------------------------------------------------------------------

class friend_info : public ::google::protobuf::Message {
 public:
  friend_info();
  virtual ~friend_info();

  friend_info(const friend_info& from);

  inline friend_info& operator=(const friend_info& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const friend_info& default_instance();

  void Swap(friend_info* other);

  // implements Message ----------------------------------------------

  friend_info* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const friend_info& from);
  void MergeFrom(const friend_info& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required string openid = 1;
  inline bool has_openid() const;
  inline void clear_openid();
  static const int kOpenidFieldNumber = 1;
  inline const ::std::string& openid() const;
  inline void set_openid(const ::std::string& value);
  inline void set_openid(const char* value);
  inline void set_openid(const char* value, size_t size);
  inline ::std::string* mutable_openid();
  inline ::std::string* release_openid();
  inline void set_allocated_openid(::std::string* openid);

  // optional string remark = 2;
  inline bool has_remark() const;
  inline void clear_remark();
  static const int kRemarkFieldNumber = 2;
  inline const ::std::string& remark() const;
  inline void set_remark(const ::std::string& value);
  inline void set_remark(const char* value);
  inline void set_remark(const char* value, size_t size);
  inline ::std::string* mutable_remark();
  inline ::std::string* release_remark();
  inline void set_allocated_remark(::std::string* remark);

  // optional int32 mail_time = 3;
  inline bool has_mail_time() const;
  inline void clear_mail_time();
  static const int kMailTimeFieldNumber = 3;
  inline ::google::protobuf::int32 mail_time() const;
  inline void set_mail_time(::google::protobuf::int32 value);

  // optional .FNF.EM_FRIGIFT_STATUS frigift_status = 4;
  inline bool has_frigift_status() const;
  inline void clear_frigift_status();
  static const int kFrigiftStatusFieldNumber = 4;
  inline ::FNF::EM_FRIGIFT_STATUS frigift_status() const;
  inline void set_frigift_status(::FNF::EM_FRIGIFT_STATUS value);

  // optional int32 frigift_time = 5;
  inline bool has_frigift_time() const;
  inline void clear_frigift_time();
  static const int kFrigiftTimeFieldNumber = 5;
  inline ::google::protobuf::int32 frigift_time() const;
  inline void set_frigift_time(::google::protobuf::int32 value);

  // optional int32 callhelp_time = 6;
  inline bool has_callhelp_time() const;
  inline void clear_callhelp_time();
  static const int kCallhelpTimeFieldNumber = 6;
  inline ::google::protobuf::int32 callhelp_time() const;
  inline void set_callhelp_time(::google::protobuf::int32 value);

  // optional int32 callhelp_count = 7;
  inline bool has_callhelp_count() const;
  inline void clear_callhelp_count();
  static const int kCallhelpCountFieldNumber = 7;
  inline ::google::protobuf::int32 callhelp_count() const;
  inline void set_callhelp_count(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:FNF.friend_info)
 private:
  inline void set_has_openid();
  inline void clear_has_openid();
  inline void set_has_remark();
  inline void clear_has_remark();
  inline void set_has_mail_time();
  inline void clear_has_mail_time();
  inline void set_has_frigift_status();
  inline void clear_has_frigift_status();
  inline void set_has_frigift_time();
  inline void clear_has_frigift_time();
  inline void set_has_callhelp_time();
  inline void clear_has_callhelp_time();
  inline void set_has_callhelp_count();
  inline void clear_has_callhelp_count();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::std::string* openid_;
  ::std::string* remark_;
  ::google::protobuf::int32 mail_time_;
  int frigift_status_;
  ::google::protobuf::int32 frigift_time_;
  ::google::protobuf::int32 callhelp_time_;
  ::google::protobuf::int32 callhelp_count_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(7 + 31) / 32];

  friend void  protobuf_AddDesc_fnf_5ffriends_2eproto();
  friend void protobuf_AssignDesc_fnf_5ffriends_2eproto();
  friend void protobuf_ShutdownFile_fnf_5ffriends_2eproto();

  void InitAsDefaultInstance();
  static friend_info* default_instance_;
};
// -------------------------------------------------------------------

class helpitem : public ::google::protobuf::Message {
 public:
  helpitem();
  virtual ~helpitem();

  helpitem(const helpitem& from);

  inline helpitem& operator=(const helpitem& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const helpitem& default_instance();

  void Swap(helpitem* other);

  // implements Message ----------------------------------------------

  helpitem* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const helpitem& from);
  void MergeFrom(const helpitem& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int64 fuid = 1;
  inline bool has_fuid() const;
  inline void clear_fuid();
  static const int kFuidFieldNumber = 1;
  inline ::google::protobuf::int64 fuid() const;
  inline void set_fuid(::google::protobuf::int64 value);

  // optional int32 help_starfruit = 2;
  inline bool has_help_starfruit() const;
  inline void clear_help_starfruit();
  static const int kHelpStarfruitFieldNumber = 2;
  inline ::google::protobuf::int32 help_starfruit() const;
  inline void set_help_starfruit(::google::protobuf::int32 value);

  // optional int32 help_time = 3;
  inline bool has_help_time() const;
  inline void clear_help_time();
  static const int kHelpTimeFieldNumber = 3;
  inline ::google::protobuf::int32 help_time() const;
  inline void set_help_time(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:FNF.helpitem)
 private:
  inline void set_has_fuid();
  inline void clear_has_fuid();
  inline void set_has_help_starfruit();
  inline void clear_has_help_starfruit();
  inline void set_has_help_time();
  inline void clear_has_help_time();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int64 fuid_;
  ::google::protobuf::int32 help_starfruit_;
  ::google::protobuf::int32 help_time_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(3 + 31) / 32];

  friend void  protobuf_AddDesc_fnf_5ffriends_2eproto();
  friend void protobuf_AssignDesc_fnf_5ffriends_2eproto();
  friend void protobuf_ShutdownFile_fnf_5ffriends_2eproto();

  void InitAsDefaultInstance();
  static helpitem* default_instance_;
};
// ===================================================================


// ===================================================================

// user_friends

// required int64 uid = 1;
inline bool user_friends::has_uid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void user_friends::set_has_uid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void user_friends::clear_has_uid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void user_friends::clear_uid() {
  uid_ = GOOGLE_LONGLONG(0);
  clear_has_uid();
}
inline ::google::protobuf::int64 user_friends::uid() const {
  return uid_;
}
inline void user_friends::set_uid(::google::protobuf::int64 value) {
  set_has_uid();
  uid_ = value;
}

// repeated .FNF.friend_info friends = 2;
inline int user_friends::friends_size() const {
  return friends_.size();
}
inline void user_friends::clear_friends() {
  friends_.Clear();
}
inline const ::FNF::friend_info& user_friends::friends(int index) const {
  return friends_.Get(index);
}
inline ::FNF::friend_info* user_friends::mutable_friends(int index) {
  return friends_.Mutable(index);
}
inline ::FNF::friend_info* user_friends::add_friends() {
  return friends_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::FNF::friend_info >&
user_friends::friends() const {
  return friends_;
}
inline ::google::protobuf::RepeatedPtrField< ::FNF::friend_info >*
user_friends::mutable_friends() {
  return &friends_;
}

// repeated .FNF.helpitem help_items = 3;
inline int user_friends::help_items_size() const {
  return help_items_.size();
}
inline void user_friends::clear_help_items() {
  help_items_.Clear();
}
inline const ::FNF::helpitem& user_friends::help_items(int index) const {
  return help_items_.Get(index);
}
inline ::FNF::helpitem* user_friends::mutable_help_items(int index) {
  return help_items_.Mutable(index);
}
inline ::FNF::helpitem* user_friends::add_help_items() {
  return help_items_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::FNF::helpitem >&
user_friends::help_items() const {
  return help_items_;
}
inline ::google::protobuf::RepeatedPtrField< ::FNF::helpitem >*
user_friends::mutable_help_items() {
  return &help_items_;
}

// optional int32 help_gift_count = 4;
inline bool user_friends::has_help_gift_count() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void user_friends::set_has_help_gift_count() {
  _has_bits_[0] |= 0x00000008u;
}
inline void user_friends::clear_has_help_gift_count() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void user_friends::clear_help_gift_count() {
  help_gift_count_ = 0;
  clear_has_help_gift_count();
}
inline ::google::protobuf::int32 user_friends::help_gift_count() const {
  return help_gift_count_;
}
inline void user_friends::set_help_gift_count(::google::protobuf::int32 value) {
  set_has_help_gift_count();
  help_gift_count_ = value;
}

// optional int32 help_gift_time = 5;
inline bool user_friends::has_help_gift_time() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void user_friends::set_has_help_gift_time() {
  _has_bits_[0] |= 0x00000010u;
}
inline void user_friends::clear_has_help_gift_time() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void user_friends::clear_help_gift_time() {
  help_gift_time_ = 0;
  clear_has_help_gift_time();
}
inline ::google::protobuf::int32 user_friends::help_gift_time() const {
  return help_gift_time_;
}
inline void user_friends::set_help_gift_time(::google::protobuf::int32 value) {
  set_has_help_gift_time();
  help_gift_time_ = value;
}

// -------------------------------------------------------------------

// friend_info

// required string openid = 1;
inline bool friend_info::has_openid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void friend_info::set_has_openid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void friend_info::clear_has_openid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void friend_info::clear_openid() {
  if (openid_ != &::google::protobuf::internal::kEmptyString) {
    openid_->clear();
  }
  clear_has_openid();
}
inline const ::std::string& friend_info::openid() const {
  return *openid_;
}
inline void friend_info::set_openid(const ::std::string& value) {
  set_has_openid();
  if (openid_ == &::google::protobuf::internal::kEmptyString) {
    openid_ = new ::std::string;
  }
  openid_->assign(value);
}
inline void friend_info::set_openid(const char* value) {
  set_has_openid();
  if (openid_ == &::google::protobuf::internal::kEmptyString) {
    openid_ = new ::std::string;
  }
  openid_->assign(value);
}
inline void friend_info::set_openid(const char* value, size_t size) {
  set_has_openid();
  if (openid_ == &::google::protobuf::internal::kEmptyString) {
    openid_ = new ::std::string;
  }
  openid_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* friend_info::mutable_openid() {
  set_has_openid();
  if (openid_ == &::google::protobuf::internal::kEmptyString) {
    openid_ = new ::std::string;
  }
  return openid_;
}
inline ::std::string* friend_info::release_openid() {
  clear_has_openid();
  if (openid_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = openid_;
    openid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void friend_info::set_allocated_openid(::std::string* openid) {
  if (openid_ != &::google::protobuf::internal::kEmptyString) {
    delete openid_;
  }
  if (openid) {
    set_has_openid();
    openid_ = openid;
  } else {
    clear_has_openid();
    openid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional string remark = 2;
inline bool friend_info::has_remark() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void friend_info::set_has_remark() {
  _has_bits_[0] |= 0x00000002u;
}
inline void friend_info::clear_has_remark() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void friend_info::clear_remark() {
  if (remark_ != &::google::protobuf::internal::kEmptyString) {
    remark_->clear();
  }
  clear_has_remark();
}
inline const ::std::string& friend_info::remark() const {
  return *remark_;
}
inline void friend_info::set_remark(const ::std::string& value) {
  set_has_remark();
  if (remark_ == &::google::protobuf::internal::kEmptyString) {
    remark_ = new ::std::string;
  }
  remark_->assign(value);
}
inline void friend_info::set_remark(const char* value) {
  set_has_remark();
  if (remark_ == &::google::protobuf::internal::kEmptyString) {
    remark_ = new ::std::string;
  }
  remark_->assign(value);
}
inline void friend_info::set_remark(const char* value, size_t size) {
  set_has_remark();
  if (remark_ == &::google::protobuf::internal::kEmptyString) {
    remark_ = new ::std::string;
  }
  remark_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* friend_info::mutable_remark() {
  set_has_remark();
  if (remark_ == &::google::protobuf::internal::kEmptyString) {
    remark_ = new ::std::string;
  }
  return remark_;
}
inline ::std::string* friend_info::release_remark() {
  clear_has_remark();
  if (remark_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = remark_;
    remark_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void friend_info::set_allocated_remark(::std::string* remark) {
  if (remark_ != &::google::protobuf::internal::kEmptyString) {
    delete remark_;
  }
  if (remark) {
    set_has_remark();
    remark_ = remark;
  } else {
    clear_has_remark();
    remark_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional int32 mail_time = 3;
inline bool friend_info::has_mail_time() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void friend_info::set_has_mail_time() {
  _has_bits_[0] |= 0x00000004u;
}
inline void friend_info::clear_has_mail_time() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void friend_info::clear_mail_time() {
  mail_time_ = 0;
  clear_has_mail_time();
}
inline ::google::protobuf::int32 friend_info::mail_time() const {
  return mail_time_;
}
inline void friend_info::set_mail_time(::google::protobuf::int32 value) {
  set_has_mail_time();
  mail_time_ = value;
}

// optional .FNF.EM_FRIGIFT_STATUS frigift_status = 4;
inline bool friend_info::has_frigift_status() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void friend_info::set_has_frigift_status() {
  _has_bits_[0] |= 0x00000008u;
}
inline void friend_info::clear_has_frigift_status() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void friend_info::clear_frigift_status() {
  frigift_status_ = 0;
  clear_has_frigift_status();
}
inline ::FNF::EM_FRIGIFT_STATUS friend_info::frigift_status() const {
  return static_cast< ::FNF::EM_FRIGIFT_STATUS >(frigift_status_);
}
inline void friend_info::set_frigift_status(::FNF::EM_FRIGIFT_STATUS value) {
  assert(::FNF::EM_FRIGIFT_STATUS_IsValid(value));
  set_has_frigift_status();
  frigift_status_ = value;
}

// optional int32 frigift_time = 5;
inline bool friend_info::has_frigift_time() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void friend_info::set_has_frigift_time() {
  _has_bits_[0] |= 0x00000010u;
}
inline void friend_info::clear_has_frigift_time() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void friend_info::clear_frigift_time() {
  frigift_time_ = 0;
  clear_has_frigift_time();
}
inline ::google::protobuf::int32 friend_info::frigift_time() const {
  return frigift_time_;
}
inline void friend_info::set_frigift_time(::google::protobuf::int32 value) {
  set_has_frigift_time();
  frigift_time_ = value;
}

// optional int32 callhelp_time = 6;
inline bool friend_info::has_callhelp_time() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void friend_info::set_has_callhelp_time() {
  _has_bits_[0] |= 0x00000020u;
}
inline void friend_info::clear_has_callhelp_time() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void friend_info::clear_callhelp_time() {
  callhelp_time_ = 0;
  clear_has_callhelp_time();
}
inline ::google::protobuf::int32 friend_info::callhelp_time() const {
  return callhelp_time_;
}
inline void friend_info::set_callhelp_time(::google::protobuf::int32 value) {
  set_has_callhelp_time();
  callhelp_time_ = value;
}

// optional int32 callhelp_count = 7;
inline bool friend_info::has_callhelp_count() const {
  return (_has_bits_[0] & 0x00000040u) != 0;
}
inline void friend_info::set_has_callhelp_count() {
  _has_bits_[0] |= 0x00000040u;
}
inline void friend_info::clear_has_callhelp_count() {
  _has_bits_[0] &= ~0x00000040u;
}
inline void friend_info::clear_callhelp_count() {
  callhelp_count_ = 0;
  clear_has_callhelp_count();
}
inline ::google::protobuf::int32 friend_info::callhelp_count() const {
  return callhelp_count_;
}
inline void friend_info::set_callhelp_count(::google::protobuf::int32 value) {
  set_has_callhelp_count();
  callhelp_count_ = value;
}

// -------------------------------------------------------------------

// helpitem

// optional int64 fuid = 1;
inline bool helpitem::has_fuid() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void helpitem::set_has_fuid() {
  _has_bits_[0] |= 0x00000001u;
}
inline void helpitem::clear_has_fuid() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void helpitem::clear_fuid() {
  fuid_ = GOOGLE_LONGLONG(0);
  clear_has_fuid();
}
inline ::google::protobuf::int64 helpitem::fuid() const {
  return fuid_;
}
inline void helpitem::set_fuid(::google::protobuf::int64 value) {
  set_has_fuid();
  fuid_ = value;
}

// optional int32 help_starfruit = 2;
inline bool helpitem::has_help_starfruit() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void helpitem::set_has_help_starfruit() {
  _has_bits_[0] |= 0x00000002u;
}
inline void helpitem::clear_has_help_starfruit() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void helpitem::clear_help_starfruit() {
  help_starfruit_ = 0;
  clear_has_help_starfruit();
}
inline ::google::protobuf::int32 helpitem::help_starfruit() const {
  return help_starfruit_;
}
inline void helpitem::set_help_starfruit(::google::protobuf::int32 value) {
  set_has_help_starfruit();
  help_starfruit_ = value;
}

// optional int32 help_time = 3;
inline bool helpitem::has_help_time() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void helpitem::set_has_help_time() {
  _has_bits_[0] |= 0x00000004u;
}
inline void helpitem::clear_has_help_time() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void helpitem::clear_help_time() {
  help_time_ = 0;
  clear_has_help_time();
}
inline ::google::protobuf::int32 helpitem::help_time() const {
  return help_time_;
}
inline void helpitem::set_help_time(::google::protobuf::int32 value) {
  set_has_help_time();
  help_time_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace FNF

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< ::FNF::EM_FRIGIFT_STATUS>() {
  return ::FNF::EM_FRIGIFT_STATUS_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_fnf_5ffriends_2eproto__INCLUDED
