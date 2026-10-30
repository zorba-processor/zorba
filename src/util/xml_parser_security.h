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
#ifndef ZORBA_XML_PARSER_SECURITY_H
#define ZORBA_XML_PARSER_SECURITY_H

#include <string>

#include <libxml/parser.h>

#include <zorba/security.h>

namespace zorba {
namespace xml {

/**
 * The single place where libxml2 parser contexts get their security-relevant
 * configuration. Every libxml2 parse in Zorba goes through ParserSecurity:
 *
 *  - XML_PARSE_NONET is always set and XML_PARSE_HUGE never is, whatever the
 *    caller asked for. System catalogs are disabled (libxml2 >= 2.14).
 *  - Without Capability::ExternalEntities, XML_PARSE_DTDLOAD and
 *    XML_PARSE_XINCLUDE are removed. XML_PARSE_NOENT is kept: Zorba's SAX
 *    loaders need it to see internal entity declarations at all, and
 *    external entities are stopped by the entity loader below instead.
 *  - The entity loader (the resource loader on libxml2 >= 2.14, a
 *    process-wide external entity loader before) loads an external DTD
 *    subset, external entity or XInclude target only if the policy allows
 *    Capability::ExternalEntities and the URI passes the policy's file
 *    (Capability::FileRead, read paths) or network (Capability::Network,
 *    network prefixes) rules. A refusal is recorded, so that the caller can
 *    report it as zerr:ZXQP0029 instead of the parser silently dropping the
 *    entity.
 *  - The entity amplification limit comes from the policy (libxml2 >= 2.11;
 *    0 keeps libxml2's default). libxml2's amplification checks always apply.
 *
 * The entity loader finds the policy through a per-thread pointer that
 * ParserSecurity::Scope sets, so every libxml2 call that can load entities
 * (xmlParseChunk, xmlParseDocument, ...) must run inside a Scope.
 */
class ParserSecurity
{
public:
  /** Uses the given policy. */
  explicit ParserSecurity(const SecurityPolicy& aPolicy);

  /**
   * Uses the given policy or, if NULL, the policy of the Zorba instance (the
   * root static context).
   */
  explicit ParserSecurity(const SecurityPolicy* aPolicy = nullptr);

  ParserSecurity(const ParserSecurity&) = delete;
  ParserSecurity& operator=(const ParserSecurity&) = delete;

  const SecurityPolicy& policy() const { return thePolicy; }

  /** The libxml2 options to use for the requested ones under the policy. */
  int options(int aRequested) const;

  /**
   * Applies options(aRequested), the amplification limit and the entity
   * loader to a parser context.
   */
  void configure(xmlParserCtxtPtr aCtxt, int aRequested) const;

  /** True if the policy refused a resource during a parse. */
  bool denied() const { return theDenied; }

  /** The first resource refused, if denied(). */
  const std::string& deniedURI() const { return theDeniedURI; }

  /**
   * Makes a ParserSecurity the one the entity loader consults on the current
   * thread for as long as the Scope lives. Scopes nest.
   */
  class Scope
  {
  public:
    explicit Scope(ParserSecurity& aSecurity);
    ~Scope();
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

  private:
    ParserSecurity* thePrevious;
  };

  /**
   * Called by the entity loader: true if the resource may be loaded; records
   * the first refusal otherwise.
   */
  bool allowResource(const char* aURI);

private:
  SecurityPolicy thePolicy;
  bool           theDenied;
  std::string    theDeniedURI;
};

/**
 * True if the policy allows an external entity, DTD or XInclude target to be
 * loaded from the given (absolute or relative) URI.
 */
bool allows_external_resource(const SecurityPolicy& aPolicy, const char* aURI);

} // namespace xml
} // namespace zorba

#endif /* ZORBA_XML_PARSER_SECURITY_H */
/* vim:set et sw=2 ts=2: */
