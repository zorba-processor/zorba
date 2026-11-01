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

#ifndef ZORBA_BIN_DIRECTORY_CONFIG_H
#define ZORBA_BIN_DIRECTORY_CONFIG_H

#include <string>
#include <utility>
#include <vector>

#include <zorba/dynamic_context.h>
#include <zorba/security.h>
#include <zorba/zorba.h>

/**
 * How a tool confines a query to one directory of documents: shared by
 * "zorba --directory" and zorba-mcp so that both run queries under the same
 * policy, with the same variable binding and output format.
 */
namespace zorba {
namespace directory_config {

/** The namespace of the directory module. */
extern char const module_ns[];

/**
 * Makes \a dir absolute and normalized, without a trailing separator: the
 * same spelling is used as the policy's read path and as \c $dir:root.
 *
 * @return \c true if \a dir names a directory; otherwise \c false and
 * \a error (if given) says why. \a dir is only changed on success.
 */
bool normalize( std::string *dir, std::string *error = nullptr );

/**
 * Narrows \a policy to reading \a dir: allows file-read, restricted to
 * \a dir, and native modules, restricted to the file module (the directory
 * module reads through it). Start from SecurityPolicy::secure() so nothing
 * else is allowed.
 */
void apply_policy( SecurityPolicy &policy, std::string const &dir );

/**
 * Binds \c $dir:root to \a dir in \a dctx (a no-op if the query does not
 * import the directory module).
 */
void bind_root( Zorba *zorba, DynamicContext *dctx, std::string const &dir );

/**
 * The serialization parameters of a directory query's result: JSON, one
 * item per line (method=json, item-separator newline).
 */
std::vector<std::pair<std::string,std::string> > serialization_params();

} // namespace directory_config
} // namespace zorba

#endif /* ZORBA_BIN_DIRECTORY_CONFIG_H */
/* vim:set et sw=2 ts=2: */
