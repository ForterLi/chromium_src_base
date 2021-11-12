// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MAC_SCOPED_AUTHORIZATIONREF_H_
#define BASE_MAC_SCOPED_AUTHORIZATIONREF_H_

#include <Security/Authorization.h>

#include "base/base_export.h"
#include "base/compiler_specific.h"

// ScopedAuthorizationRef maintains ownership of an AuthorizationRef.  It is
// patterned after the unique_ptr interface.

namespace base {
namespace mac {

class BASE_EXPORT ScopedAuthorizationRef {
 public:
  explicit ScopedAuthorizationRef(AuthorizationRef authorization = NULL)
      : authorization_(authorization) {
  }

  ScopedAuthorizationRef(const ScopedAuthorizationRef&) = delete;
  ScopedAuthorizationRef& operator=(const ScopedAuthorizationRef&) = delete;

  ~ScopedAuthorizationRef() {
    if (authorization_) {
      FreeInternal();
    }
  }

  void reset(AuthorizationRef authorization = NULL) {
    if (authorization_ != authorization) {
      if (authorization_) {
        FreeInternal();
      }
      authorization_ = authorization;
    }
  }

  bool operator==(AuthorizationRef that) const {
    return authorization_ == that;
  }

  bool operator!=(AuthorizationRef that) const {
    return authorization_ != that;
  }

  operator AuthorizationRef() const {
    return authorization_;
  }

  AuthorizationRef* get_pointer() { return &authorization_; }

  AuthorizationRef get() const {
    return authorization_;
  }

  void swap(ScopedAuthorizationRef& that) {
    AuthorizationRef temp = that.authorization_;
    that.authorization_ = authorization_;
    authorization_ = temp;
  }

  // ScopedAuthorizationRef::release() is like std::unique_ptr<>::release. It is
  // NOT a wrapper for AuthorizationFree(). To force a ScopedAuthorizationRef
  // object to call AuthorizationFree(), use ScopedAuthorizationRef::reset().
  AuthorizationRef release() WARN_UNUSED_RESULT {
    AuthorizationRef temp = authorization_;
    authorization_ = NULL;
    return temp;
  }

 private:
  // Calling AuthorizationFree, defined in Security.framework, from an inline
  // function, results in link errors when linking dynamically with
  // libbase.dylib. So wrap the call in an un-inlined method. This method
  // doesn't check if |authorization_| is null; that check should be in the
  // inlined callers.
  void FreeInternal();

  AuthorizationRef authorization_;
};

}  // namespace mac
}  // namespace base

#endif  // BASE_MAC_SCOPED_AUTHORIZATIONREF_H_
