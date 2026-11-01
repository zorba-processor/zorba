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
#pragma once
#ifndef ZORBA_XERCES_XMLCHARRAY_H
#define ZORBA_XERCES_XMLCHARRAY_H

#ifndef ZORBA_NO_XMLSCHEMA

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XercesDefs.hpp>

#include <cstddef>
#include <ostream>
#include <string>

#include <zorba/config.h>
#include <zorbatypes/zstring.h>

XERCES_CPP_NAMESPACE_USE

namespace zorba 
{

/**
 * Explicit UTF-8 <-> UTF-16 (XMLCh) conversion. XMLString::transcode() uses
 * the process locale, which mangles non-ASCII text (e.g. under LC_ALL=C), so
 * Zorba never calls it. The returned buffers are allocated by Xerces' memory
 * manager and must be freed with XMLString::release().
 */
namespace xerces_utf8
{

/** Decodes UTF-8 (invalid sequences become U+FFFD). Returns 0 for a null s. */
inline XMLCh* from_utf8(const char* s, std::size_t len)
{
  if (!s)
    return 0;
  std::basic_string<XMLCh> out;
  out.reserve(len);
  const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
  const unsigned char* end = p + len;
  while (p < end)
  {
    unsigned long cp = 0xFFFD;
    unsigned c = *p;
    int n = 0;
    unsigned long min = 0;
    if (c < 0x80)       { cp = c; n = 0; }
    else if (c >= 0xC2 && c <= 0xDF) { cp = c & 0x1F; n = 1; min = 0x80; }
    else if (c >= 0xE0 && c <= 0xEF) { cp = c & 0x0F; n = 2; min = 0x800; }
    else if (c >= 0xF0 && c <= 0xF4) { cp = c & 0x07; n = 3; min = 0x10000; }
    else n = -1;
    ++p;
    if (n > 0)
    {
      int i = 0;
      for (; i < n && p < end && (*p & 0xC0) == 0x80; ++i, ++p)
        cp = (cp << 6) | (*p & 0x3F);
      if (i != n || cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
        cp = 0xFFFD;
    }
    else if (n < 0)
      cp = 0xFFFD;
    if (cp >= 0x10000)
    {
      cp -= 0x10000;
      out.push_back(static_cast<XMLCh>(0xD800 + (cp >> 10)));
      out.push_back(static_cast<XMLCh>(0xDC00 + (cp & 0x3FF)));
    }
    else
      out.push_back(static_cast<XMLCh>(cp));
  }
  return XMLString::replicate(out.c_str());
}

inline XMLCh* from_utf8(const char* s)
{
  return s ? from_utf8(s, std::char_traits<char>::length(s)) : 0;
}

/** Encodes len XMLCh units as UTF-8 (lone surrogates become U+FFFD). */
inline char* to_utf8(const XMLCh* s, std::size_t len)
{
  if (!s)
    return 0;
  std::string out;
  out.reserve(len);
  for (std::size_t i = 0; i < len; ++i)
  {
    unsigned long cp = s[i];
    if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < len &&
        s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF)
    {
      cp = 0x10000 + ((cp - 0xD800) << 10) + (s[i + 1] - 0xDC00);
      ++i;
    }
    else if (cp >= 0xD800 && cp <= 0xDFFF)
      cp = 0xFFFD;
    if (cp < 0x80)
      out.push_back(static_cast<char>(cp));
    else if (cp < 0x800)
    {
      out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    else if (cp < 0x10000)
    {
      out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    else
    {
      out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
  }
  return XMLString::replicate(out.c_str());
}

inline char* to_utf8(const XMLCh* s)
{
  return s ? to_utf8(s, XMLString::stringLen(s)) : 0;
}

} // namespace xerces_utf8


class XMLChArray 
{
private:
  XMLCh * buf;
    
 public:
  XMLChArray(XMLCh* buf_) : buf(buf_) {}
    
  XMLChArray(const zstring& str) 
    :
    buf(xerces_utf8::from_utf8(str.c_str(), str.size()))
  {
  }

  ~XMLChArray() { XMLString::release(&buf); }

  XMLCh* get() { return buf; }

  const XMLCh* get () const { return buf; }

  operator XMLCh* () { return get(); }

  operator const XMLCh* () const { return get(); }

  friend std::ostream& operator<<(std::ostream& out, const XMLChArray& xmlChArray)
  {
    char* utf8 = xerces_utf8::to_utf8(xmlChArray.get());
    if (utf8)
      out << utf8;
    XMLString::release(&utf8);
    return out;
  }
};

}

#endif // ifndef ZORBA_NO_XMLSCHEMA
#endif // ifndef ZORBA_XERCES_XMLCHARRAY_H


/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
