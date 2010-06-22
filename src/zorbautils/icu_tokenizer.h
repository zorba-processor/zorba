/*
 * Copyright 2006-2008 The FLWOR Foundation.
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

#ifndef ZORBA_FULL_TEXT_ICU_TOKENIZER_H
#define ZORBA_FULL_TEXT_ICU_TOKENIZER_H

#include <memory>                       /* for auto_ptr */
#include <unicode/rbbi.h>               /* for RuleBasedBreakIterator */

#include "zorbautils/tokenizer.h"

namespace zorba {

/**
 * An ICU_Tokenizer is-a Tokenizer that uses the ICU (International Components
 * for Unicode) library.
 */
class ICU_Tokenizer : public Tokenizer {
public:

  /**
   * Constructs an ICU_Tokenizer.
   *
   * @param wildcards If <code>true</code>, allows XQuery wildcard syntax
   * characters to be part of tokens.
   */
  ICU_Tokenizer( bool wildcards = false );

  void tokenize( char const*, int, locale::iso639_1::type, Callback& );

private:
  /**
   * Use an auto_ptr to guarantee destruction in case the constructor throws an
   * exception.
   *
   * See the C++ FAQ 17.4: How should I handle resources if my constructors may
   * throw exceptions?
   */ 
  typedef std::auto_ptr<U_NAMESPACE_QUALIFIER RuleBasedBreakIterator>
          BreakIterator_ptr;

  bool const wildcards_;
  BreakIterator_ptr word_it_;
  BreakIterator_ptr sent_it_;
};

} // namespace zorba
#endif  /* ZORBA_FULL_TEXT_ICU_TOKENIZER_H */
/* vim:set et sw=2 ts=2: */
