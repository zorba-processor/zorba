/*
 * Copyright 2006-2016 zorba.io
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdafx.h"

#include <algorithm>
#include <string>

#include <zorba/config.h>
#include <zorba/security.h>

#include "util/security_check.h"

namespace zorba {

namespace {

const uint32_t ALL_CAPABILITIES =
  static_cast<uint32_t>(Capability::ExternalEntities) |
  static_cast<uint32_t>(Capability::FileRead) |
  static_cast<uint32_t>(Capability::FileWrite) |
  static_cast<uint32_t>(Capability::Network) |
  static_cast<uint32_t>(Capability::RemoteImport) |
  static_cast<uint32_t>(Capability::Environment) |
  static_cast<uint32_t>(Capability::NativeModules);

#ifndef ZORBA_WITH_FILE_ACCESS
const uint32_t FILE_CAPABILITIES =
  static_cast<uint32_t>(Capability::FileRead) |
  static_cast<uint32_t>(Capability::FileWrite);
#endif

// Same default as Properties::max_udf_call_depth.
const uint32_t DEFAULT_MAX_CALL_DEPTH = 1024;

typedef bool (*covers_fn)(const std::string&, const std::string&);

bool is_separator(char c)
{
  return c == '/' || c == '\\';
}

std::string strip_trailing_separators(const std::string& aPath)
{
  std::string::size_type lEnd = aPath.size();
  while (lEnd > 1 && is_separator(aPath[lEnd - 1]))
    --lEnd;
  return aPath.substr(0, lEnd);
}

// True if directory aInner equals directory aOuter or lies below it. Purely
// lexical: canonicalization happens where paths are checked.
bool dir_covers(const std::string& aOuter, const std::string& aInner)
{
  const std::string lOuter = strip_trailing_separators(aOuter);
  const std::string lInner = strip_trailing_separators(aInner);
  if (lOuter == lInner)
    return true;
  if (lInner.size() <= lOuter.size() ||
      lInner.compare(0, lOuter.size(), lOuter) != 0)
    return false;
  return is_separator(lOuter.back()) || is_separator(lInner[lOuter.size()]);
}

bool prefix_covers(const std::string& aOuter, const std::string& aInner)
{
  return aInner.size() >= aOuter.size() &&
         aInner.compare(0, aOuter.size(), aOuter) == 0;
}

// Sorted, without duplicates and without entries covered by another entry.
std::vector<std::string> minimize(
    const std::vector<String>& aList,
    covers_fn aCovers)
{
  std::vector<std::string> lIn;
  lIn.reserve(aList.size());
  for (const String& s : aList)
    lIn.push_back(s.str());
  std::sort(lIn.begin(), lIn.end());
  lIn.erase(std::unique(lIn.begin(), lIn.end()), lIn.end());

  std::vector<std::string> lOut;
  for (std::size_t i = 0; i < lIn.size(); ++i)
  {
    bool lCovered = false;
    for (std::size_t j = 0; j < lIn.size() && !lCovered; ++j)
      lCovered = (i != j && aCovers(lIn[j], lIn[i]));
    if (!lCovered)
      lOut.push_back(lIn[i]);
  }
  return lOut;
}

// Intersection of two allow-lists. Returns false if both lists are non-empty
// and no entry survives (the capability must then be denied).
bool intersect(
    const std::vector<String>& aA,
    const std::vector<String>& aB,
    covers_fn aCovers,
    std::vector<String>& aResult)
{
  if (aA.empty())
  {
    aResult = aB;
    return true;
  }
  if (aB.empty())
  {
    aResult = aA;
    return true;
  }

  std::vector<String> lResult;
  for (const String& a : aA)
  {
    for (const String& b : aB)
    {
      const std::string lA = a.str();
      const std::string lB = b.str();
      if (aCovers(lA, lB))
        lResult.push_back(b);
      else if (aCovers(lB, lA))
        lResult.push_back(a);
    }
  }

  aResult.clear();
  for (const std::string& s : minimize(lResult, aCovers))
    aResult.push_back(String(s));
  return !aResult.empty();
}

uint32_t min_limit(uint32_t aA, uint32_t aB)
{
  if (aA == 0) return aB;
  if (aB == 0) return aA;
  return std::min(aA, aB);
}

} // anonymous namespace


/*******************************************************************************

********************************************************************************/
class SecurityPolicy::Impl
{
public:
  uint32_t                  theCapabilities = 0;
  std::vector<String>       theReadPaths;
  std::vector<String>       theWritePaths;
  std::vector<String>       theNetworkPrefixes;
  bool                      theAllowPrivateAddresses = false;
  std::chrono::milliseconds theTimeout{0};
  uint32_t                  theMaxCallDepth = DEFAULT_MAX_CALL_DEPTH;
  uint32_t                  theMaxEntityAmplification = 0;
};


SecurityPolicy SecurityPolicy::secure()
{
  return SecurityPolicy();
}


SecurityPolicy SecurityPolicy::unrestricted()
{
  SecurityPolicy lPolicy;
  uint32_t lCaps = ALL_CAPABILITIES;
#ifndef ZORBA_WITH_FILE_ACCESS
  lCaps &= ~FILE_CAPABILITIES;
#endif
  lPolicy.theImpl->theCapabilities = lCaps;
  lPolicy.theImpl->theAllowPrivateAddresses = true;
  return lPolicy;
}


SecurityPolicy::SecurityPolicy()
  :
  theImpl(new Impl())
{
}


SecurityPolicy::SecurityPolicy(const SecurityPolicy& aOther)
  :
  theImpl(new Impl(*aOther.theImpl))
{
}


SecurityPolicy& SecurityPolicy::operator=(const SecurityPolicy& aOther)
{
  if (this != &aOther)
    *theImpl = *aOther.theImpl;
  return *this;
}


SecurityPolicy::~SecurityPolicy()
{
}


SecurityPolicy& SecurityPolicy::allow(Capability aCap)
{
  uint32_t lBit = static_cast<uint32_t>(aCap) & ALL_CAPABILITIES;
#ifndef ZORBA_WITH_FILE_ACCESS
  lBit &= ~FILE_CAPABILITIES;
#endif
  theImpl->theCapabilities |= lBit;
  return *this;
}


SecurityPolicy& SecurityPolicy::deny(Capability aCap)
{
  theImpl->theCapabilities &= ~static_cast<uint32_t>(aCap);
  return *this;
}


bool SecurityPolicy::allows(Capability aCap) const
{
  const uint32_t lBit = static_cast<uint32_t>(aCap);
  return lBit != 0 && (theImpl->theCapabilities & lBit) == lBit;
}


SecurityPolicy& SecurityPolicy::addReadPath(const String& aDir)
{
  theImpl->theReadPaths.push_back(aDir);
  return *this;
}


SecurityPolicy& SecurityPolicy::addWritePath(const String& aDir)
{
  theImpl->theWritePaths.push_back(aDir);
  return *this;
}


SecurityPolicy& SecurityPolicy::addNetworkPrefix(const String& aUrlPrefix)
{
  theImpl->theNetworkPrefixes.push_back(aUrlPrefix);
  return *this;
}


const std::vector<String>& SecurityPolicy::getReadPaths() const
{
  return theImpl->theReadPaths;
}


const std::vector<String>& SecurityPolicy::getWritePaths() const
{
  return theImpl->theWritePaths;
}


const std::vector<String>& SecurityPolicy::getNetworkPrefixes() const
{
  return theImpl->theNetworkPrefixes;
}


bool SecurityPolicy::checkURI(
    const String& aURI,
    EntityData::Kind aKind,
    bool aWrite) const
{
  return checkURI(aURI, aKind, aWrite, nullptr);
}


bool SecurityPolicy::checkURI(
    const String& aURI,
    EntityData::Kind aKind,
    bool aWrite,
    String* aReason) const
{
  security::Use lUse;
  switch (aKind)
  {
    case EntityData::SCHEMA:
    case EntityData::MODULE:
      lUse = security::Use::Import;
      break;
    case EntityData::THESAURUS:
    case EntityData::STOP_WORDS:
      lUse = security::Use::Data;
      break;
    case EntityData::COLLECTION:
      lUse = security::Use::Collection;
      break;
    default:
      lUse = security::Use::Document;
      break;
  }
  security::Verdict const lVerdict =
    security::check(*this, aURI.str(), lUse, aWrite);
  if (!lVerdict.allowed && aReason)
    *aReason = lVerdict.reason;
  return lVerdict.allowed;
}


bool SecurityPolicy::checkConnection(
    const String& aURL,
    const String& aAddress,
    String* aReason) const
{
  security::Verdict const lVerdict =
    security::check_connection(*this, aURL.str(), aAddress.str());
  if (!lVerdict.allowed && aReason)
    *aReason = lVerdict.reason;
  return lVerdict.allowed;
}


std::unique_ptr<std::istream> SecurityPolicy::openFile(
    const String& aPath,
    bool aBinary,
    String* aReason) const
{
  security::Verdict lVerdict;
  std::unique_ptr<std::istream> lStream = security::open_file(
      *this, aPath.str(), security::Use::Document, nullptr, aBinary, lVerdict);
  if (!lVerdict.allowed && aReason)
    *aReason = lVerdict.reason;
  return lStream;
}


SecurityPolicy& SecurityPolicy::setAllowPrivateAddresses(bool aAllow)
{
  theImpl->theAllowPrivateAddresses = aAllow;
  return *this;
}


bool SecurityPolicy::allowsPrivateAddresses() const
{
  return theImpl->theAllowPrivateAddresses;
}


SecurityPolicy& SecurityPolicy::setTimeout(std::chrono::milliseconds aTimeout)
{
  theImpl->theTimeout =
    aTimeout.count() < 0 ? std::chrono::milliseconds(0) : aTimeout;
  return *this;
}


std::chrono::milliseconds SecurityPolicy::getTimeout() const
{
  return theImpl->theTimeout;
}


SecurityPolicy& SecurityPolicy::setMaxCallDepth(uint32_t aDepth)
{
  theImpl->theMaxCallDepth = aDepth;
  return *this;
}


uint32_t SecurityPolicy::getMaxCallDepth() const
{
  return theImpl->theMaxCallDepth;
}


SecurityPolicy& SecurityPolicy::setMaxEntityAmplification(uint32_t aFactor)
{
  theImpl->theMaxEntityAmplification = aFactor;
  return *this;
}


uint32_t SecurityPolicy::getMaxEntityAmplification() const
{
  return theImpl->theMaxEntityAmplification;
}


SecurityPolicy SecurityPolicy::narrowedBy(const SecurityPolicy& aOther) const
{
  const Impl& a = *theImpl;
  const Impl& b = *aOther.theImpl;

  SecurityPolicy lResult;
  Impl& r = *lResult.theImpl;

  r.theCapabilities = a.theCapabilities & b.theCapabilities;
  r.theAllowPrivateAddresses =
    a.theAllowPrivateAddresses && b.theAllowPrivateAddresses;

  if (a.theTimeout.count() == 0)
    r.theTimeout = b.theTimeout;
  else if (b.theTimeout.count() == 0)
    r.theTimeout = a.theTimeout;
  else
    r.theTimeout = std::min(a.theTimeout, b.theTimeout);

  r.theMaxCallDepth = min_limit(a.theMaxCallDepth, b.theMaxCallDepth);
  r.theMaxEntityAmplification =
    min_limit(a.theMaxEntityAmplification, b.theMaxEntityAmplification);

  if (!intersect(a.theReadPaths, b.theReadPaths, dir_covers, r.theReadPaths))
    r.theCapabilities &= ~static_cast<uint32_t>(Capability::FileRead);
  if (!intersect(a.theWritePaths, b.theWritePaths, dir_covers, r.theWritePaths))
    r.theCapabilities &= ~static_cast<uint32_t>(Capability::FileWrite);
  if (!intersect(a.theNetworkPrefixes, b.theNetworkPrefixes, prefix_covers,
                 r.theNetworkPrefixes))
    r.theCapabilities &= ~static_cast<uint32_t>(Capability::Network);

  return lResult;
}


bool SecurityPolicy::isWithin(const SecurityPolicy& aOther) const
{
  return narrowedBy(aOther) == *this;
}


bool SecurityPolicy::operator==(const SecurityPolicy& aOther) const
{
  const Impl& a = *theImpl;
  const Impl& b = *aOther.theImpl;
  return a.theCapabilities == b.theCapabilities &&
         a.theAllowPrivateAddresses == b.theAllowPrivateAddresses &&
         a.theTimeout == b.theTimeout &&
         a.theMaxCallDepth == b.theMaxCallDepth &&
         a.theMaxEntityAmplification == b.theMaxEntityAmplification &&
         minimize(a.theReadPaths, dir_covers) ==
           minimize(b.theReadPaths, dir_covers) &&
         minimize(a.theWritePaths, dir_covers) ==
           minimize(b.theWritePaths, dir_covers) &&
         minimize(a.theNetworkPrefixes, prefix_covers) ==
           minimize(b.theNetworkPrefixes, prefix_covers);
}

} // namespace zorba
/* vim:set et sw=2 ts=2: */
