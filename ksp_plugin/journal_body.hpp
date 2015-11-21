#pragma once

#include "ksp_plugin/journal.hpp"

#include <list>

#include "glog/logging.h"

namespace principia {
namespace ksp_plugin {

template<typename Profile>
Journal::Method<Profile>::Method()
    : message_(std::make_unique<typename Profile::Message>()) {}

template<typename Profile>
template<typename P, typename>
Journal::Method<Profile>::Method(typename P::In const& in)
    : message_(std::make_unique<typename Profile::Message>()) {
  Profile::Fill(in, message_.get());
}

template<typename Profile>
template<typename P, typename, typename>
Journal::Method<Profile>::Method(typename P::In const& in,
                                 typename P::Out const& out)
    : message_(std::make_unique<typename Profile::Message>()),
      out_filler_([this, out]() { Profile::Fill(out, message_.get()); }) {
  Profile::Fill(in, message_.get());
}

template<typename Profile>
Journal::Method<Profile>::~Method() {
  CHECK(returned_);
  if (out_filler_ != nullptr) {
    out_filler_();
  }
  serialization::Method method;
  method.SetAllocatedExtension(Profile::Message::extension, message_.release());
  active_->Write(method);
}

template<typename Profile>
void Journal::Method<Profile>::Return() {
  CHECK(!returned_);
  returned_ = true;
}

template<typename Profile>
template<typename P, typename>
typename P::Return Journal::Method<Profile>::Return(
    typename P::Return const& result) {
  CHECK(!returned_);
  returned_ = true;
  Profile::Fill(result, message_.get());
  return result;
}

template<typename Message>
void Journal::AppendMessage(not_null<std::unique_ptr<Message>> message) {
  if (journal_ == nullptr) {
    journal_ = new std::list<serialization::Method>;
  }
  journal_->emplace_back();
  journal_->back().SetAllocatedExtension(Message::extension, message.release());
}

}  // namespace ksp_plugin
}  // namespace principia
