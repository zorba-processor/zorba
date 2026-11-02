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
#include <cstring>

#include "zorbatypes/decimal.h"
#include "zorbatypes/float.h"
#include "zorbatypes/integer.h"
#include "zorbatypes/schema_types.h"
#include "zorbautils/hashfun.h"

#include "numeric_key_hash.h"

namespace zorba {

///////////////////////////////////////////////////////////////////////////////

bool is_exact_numeric( store::SchemaTypeCode code ) {
  switch ( code ) {
    case store::XS_DECIMAL:
    case store::XS_INTEGER:
    case store::XS_NON_POSITIVE_INTEGER:
    case store::XS_NEGATIVE_INTEGER:
    case store::XS_LONG:
    case store::XS_INT:
    case store::XS_SHORT:
    case store::XS_BYTE:
    case store::XS_NON_NEGATIVE_INTEGER:
    case store::XS_POSITIVE_INTEGER:
    case store::XS_UNSIGNED_LONG:
    case store::XS_UNSIGNED_INT:
    case store::XS_UNSIGNED_SHORT:
    case store::XS_UNSIGNED_BYTE:
      return true;
    default:
      return false;
  }
}

namespace {

// The xs:float a key is cast to when compared with an xs:float (through its
// xs:string form, F&O 3.0 19.1.2.2), and whether rounding to xs:double first
// gives another one.
float exact_to_float( store::Item const *key, bool *double_rounding ) {
  xs_decimal const value( key->getDecimalValue() );
  float const f = xs_float( value ).getNumber();
  *double_rounding = static_cast<float>( xs_double( value ).getNumber() ) != f;
  return f;
}

} // namespace

uint32_t NumericKeyHash::hash_numeric( store::Item const *key, long timezone,
                                       XQPCollator const *collator ) const {
  if ( !key->isAtomic() )
    return key->hash( timezone, collator );

  store::SchemaTypeCode const code = key->getTypeCode();
  float f;
  if ( code == store::XS_FLOAT )
    f = key->getFloatValue().getNumber();
  else if ( code == store::XS_DOUBLE )
    f = static_cast<float>( key->getDoubleValue().getNumber() );
  else if ( is_exact_numeric( code ) ) {
    bool double_rounding;
    f = exact_to_float( key, &double_rounding );
    if ( double_rounding )
      saw_double_rounding_ = true;
  } else
    return key->hash( timezone, collator );

  if ( mode_ == linear )
    return 0x9E3779B9u;
  if ( f != f )
    return 0x7FC00000u;                 // all NaNs alike
  if ( f == 0 )
    f = 0;                              // +0 and -0 alike
  uint32_t bits;
  std::memcpy( &bits, &f, sizeof bits );
  return hashfun::h32( &bits, sizeof bits, FNV_32_INIT );
}

bool NumericKeyHash::needs_change( store::Item const *key ) {
  if ( !key->isAtomic() )
    return false;
  store::SchemaTypeCode const code = key->getTypeCode();
  if ( mode_ == exact ) {
    if ( code == store::XS_FLOAT ) {
      mode_ = via_float;
      return true;
    }
    return false;
  }
  if ( is_exact_numeric( code ) ) {
    bool double_rounding;
    (void)exact_to_float( key, &double_rounding );
    if ( double_rounding ) {
      mode_ = linear;
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace zorba
/* vim:set et sw=2 ts=2: */
