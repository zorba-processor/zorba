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

#include <cstdio>
#include <charconv>
#include <cstdlib>
#include <limits>
#include <string>
#include <stdlib.h>

#include "common/common.h"
#include "util/ascii_util.h"

#include "decimal.h"
#include "float.h"
#include "integer.h"

#include "zorbaserialization/serialize_zorba_types.h"
#include "zorbaserialization/serialize_template_types.h"

///////////////////////////////////////////////////////////////////////////////

namespace zorba {

zstring const& nan_str() {
  static zstring const value( "NaN" );
  return value;
}

zstring const& pos_inf_str() {
  static zstring const value( "INF" );
  return value;
}

zstring const& neg_inf_str() {
  static zstring const value( "-INF" );
  return value;
}

////////////////////////////////////////////////////////////////////////////////

static void count_significant_digits( char digit, int *significant_digits,
                                      int *trailing_zeros ) {
  if ( digit == '0' )
    ++*trailing_zeros;
  else {
    if ( (*significant_digits)++ )
      *significant_digits += *trailing_zeros;
    *trailing_zeros = 0;
  }
}

template<typename F>
void FloatImpl<F>::parse( char const *s ) {
  if ( !*s )
    throw std::invalid_argument( "empty string" );

  int significant_digits = 0;

  s = ascii::trim_start_space( s );

  if ( !parse_etc( s ) ) {
    char const *const first_non_ws = s;
    int trailing_zeros = 0;
    //
    // We need got_digit to know that we're potentially parsing a floating
    // point value comprised of at least one digit -- which means the value
    // can't be one of the special values of INF, -INF, or NaN.
    //
    // We need to know this to prevent aton() from parsing the special values
    // since it (via strtod() that it uses) allows them regardless of case
    // whereas XQuery insists on a specific case.
    //
    bool got_digit = false;

    if ( *s == '+' || *s == '-' )
      ++s;
    if ( ascii::is_digit( *s ) ) {
      got_digit = true;
      do {
        count_significant_digits( *s, &significant_digits, &trailing_zeros );
      } while ( ascii::is_digit( *++s ) );
    }
    if ( *s == '.' && ascii::is_digit( *++s ) ) {
      got_digit = true;
      do {
        count_significant_digits( *s, &significant_digits, &trailing_zeros );
      } while ( ascii::is_digit( *++s ) );
    }
    if ( got_digit && (*s == 'e' || *s == 'E') ) {
      ++s;
      if ( *s == '+' || *s == '-' )
        ++s;
      if ( !ascii::is_digit( *s ) )     // an exponent needs a digit
        got_digit = false;
      while ( ascii::is_digit( *s ) )
        ++s;
    }
    //
    // XSD 1.1 Part 2, 3.3.4-3.3.5: only trailing whitespace may follow; check
    // it here since aton() (via strtod()) would also accept forms like "0x1A"
    // or "1.5x" by parsing a prefix.
    //
    if ( !got_digit || *ascii::trim_start_space( s ) )
      throw std::invalid_argument(
        BUILD_STRING( '"', first_non_ws, "\": invalid floating-point literal" )
      );
    value_ = ztd::aton<value_type>( first_non_ws );
  }

  precision_ = significant_digits < max_precision() ?
    significant_digits : max_precision();
}

template<typename F>
bool FloatImpl<F>::parse_etc( char const *s ) {
  if ( strncmp( s, "INF", 3 ) == 0 ) {
    value_ = FloatImpl<F>::pos_inf().value_;
    s += 3;
  } else if ( strncmp( s, "-INF", 4 ) == 0 ) {
    value_ = FloatImpl<F>::neg_inf().value_;
    s += 4;
  } else if ( strncmp( s, "NaN", 3 ) == 0 ) {
    value_ = FloatImpl<F>::nan().value_;
    s += 3;
  } else if ( strncmp( s, "+INF", 4 ) == 0 ) {  // allowed by XSD 1.1
    value_ = FloatImpl<F>::pos_inf().value_;
    s += 4;
  } else
    return false;

  return !*ascii::trim_start_space( s );
}

////////// constructors ///////////////////////////////////////////////////////

template<typename F>
FloatImpl<F>::FloatImpl( Decimal const &d ) {
  zstring const temp( d.toString() );
  parse( temp.c_str() );
}

template<typename F>
template<class T>
FloatImpl<F>::FloatImpl( IntegerImpl<T> const &i ) {
  zstring const temp( i.toString() );
  parse( temp.c_str() );
}

template FloatImpl<float>::FloatImpl( Integer const& );
template FloatImpl<float>::FloatImpl( NegativeInteger const& );
template FloatImpl<float>::FloatImpl( NonNegativeInteger const& );
template FloatImpl<float>::FloatImpl( NonPositiveInteger const& );
template FloatImpl<float>::FloatImpl( PositiveInteger const& );

template FloatImpl<double>::FloatImpl( Integer const& );
template FloatImpl<double>::FloatImpl( NegativeInteger const& );
template FloatImpl<double>::FloatImpl( NonNegativeInteger const& );
template FloatImpl<double>::FloatImpl( NonPositiveInteger const& );
template FloatImpl<double>::FloatImpl( PositiveInteger const& );

////////// assignment operators ///////////////////////////////////////////////

template<typename F>
FloatImpl<F>& FloatImpl<F>::operator=( Decimal const &d ) {
  zstring const temp( d.toString() );
  parse( temp.c_str() );
  return *this;
}

template<typename F>
template<class T>
FloatImpl<F>& FloatImpl<F>::operator=( IntegerImpl<T> const &i ) {
  zstring const temp( i.toString() );
  parse( temp.c_str() );
  return *this;
}

#define ZORBA_INSTANTIATE(F,I) \
  template FloatImpl<F>& FloatImpl<F>::operator=( I const& )

ZORBA_INSTANTIATE(float,Integer);
ZORBA_INSTANTIATE(float,NegativeInteger);
ZORBA_INSTANTIATE(float,NonNegativeInteger);
ZORBA_INSTANTIATE(float,NonPositiveInteger);
ZORBA_INSTANTIATE(float,PositiveInteger);

ZORBA_INSTANTIATE(double,Integer);
ZORBA_INSTANTIATE(double,NegativeInteger);
ZORBA_INSTANTIATE(double,NonNegativeInteger);
ZORBA_INSTANTIATE(double,NonPositiveInteger);
ZORBA_INSTANTIATE(double,PositiveInteger);
#undef ZORBA_INSTANTIATE

////////// math functions /////////////////////////////////////////////////////

template<typename F>
FloatImpl<F> FloatImpl<F>::acos() const {
  if ( *this < numeric_consts<FloatImpl>::neg_one() ||
       *this > numeric_consts<FloatImpl>::one() )
    return nan();
  // acos(+0) = acos(-0) = +pi/2 (F&O 3.0 4.8.10): the sign of zero does not
  // carry over.
  return FloatImpl<F>( std::acos( value_ ) );
}

template<typename F>
FloatImpl<F> FloatImpl<F>::asin() const {
  if ( *this < numeric_consts<FloatImpl>::neg_one() ||
       *this > numeric_consts<FloatImpl>::one() )
    return nan();
  return FloatImpl<F>( std::asin( value_ ) );
}

template<typename F>
void FloatImpl<F>::frexp( FloatImpl<F> &out_mantissa,
                          Integer &out_exponent ) const {
  int expint;
  out_mantissa = FloatImpl( ::frexp( value_, &expint ) );
  out_exponent = Integer( expint );
}

template<>
void FloatImpl<double>::modf( FloatImpl<double> &out_fraction,
                              FloatImpl<double> &out_integer ) const {
  double int_part;
  out_fraction = std::modf( value_, &int_part );
  out_integer = int_part;
}

template<>
void FloatImpl<float>::modf( FloatImpl<float> &out_fraction,
                             FloatImpl<float> &out_integer ) const {
  float int_part;
  out_fraction = ::modff( value_, &int_part );
  out_integer = int_part;
}

template<typename F>
FloatImpl<F> FloatImpl<F>::round() const {
  return round( numeric_consts<xs_integer>::zero() );
}

template<typename F>
FloatImpl<F> FloatImpl<F>::round( Integer const &precision ) const {
  FloatImpl result;
  if ( isFinite() && !isZero() ) {
    MAPM m(
      Decimal::round2(
        Decimal::value_type( value_ ),
        Decimal::value_type( precision.itod() )
      )
    );
    if ( value_ < 0 && m.sign() == 0 )
      result = neg_zero();
    else {
      char buf[200];
      m.toString( buf, ZORBA_FLOAT_POINT_PRECISION );
      result.parse( buf );
    }
  } else
    result.value_ = value_;

  return result;
}

template<typename F>
FloatImpl<F> FloatImpl<F>::roundHalfToEven( Integer const &precision ) const {
  FloatImpl result;
  //
  // A value has at most max_digits10 significant digits, so rounding to a
  // precision at or beyond its last one leaves it unchanged. Short-cut that
  // case: the decimal rounding below is limited to
  // Decimal::round_precision_limit fractional digits and would flush a value
  // like 4.9e-324 to zero (F&O 3.0 4.4.5).
  //
  if ( isFinite() && !isZero() ) {
    MAPM const last_digit(
      std::numeric_limits<F>::max_digits10 -
      std::floor( std::log10( std::fabs( static_cast<double>( value_ ) ) ) )
    );
    if ( precision.itod() >= last_digit ) {
      result.value_ = value_;
      return result;
    }
  }
  if ( isFinite() && !isZero() ) {
    MAPM m(
      Decimal::roundHalfToEven2(
        Decimal::value_type( value_ ),
        Decimal::value_type( precision.itod() )
      )
    );
    if ( value_ < 0 && m.sign() == 0 )
      result = neg_zero();
    else {
      char buf[200];
      m.toString( buf, ZORBA_FLOAT_POINT_PRECISION );
      result.parse( buf );
    }
  } else
    result.value_ = value_;
  return result;
}

////////// miscellaneous //////////////////////////////////////////////////////

template<typename F>
FloatImpl<F> const& FloatImpl<F>::nan() {
  static FloatImpl<F> const value( std::sqrt( -1.0 ) );
  return value;
}

template<typename F>
FloatImpl<F> const& FloatImpl<F>::neg_inf() {
  static FloatImpl<F> const value( -std::numeric_limits<F>::infinity() );
  return value;
}

template<typename F>
FloatImpl<F> const& FloatImpl<F>::neg_zero() {
  static FloatImpl<F> const value( -0.0 );
  return value;
}

template<typename F>
FloatImpl<F> const& FloatImpl<F>::pos_inf() {
  static FloatImpl<F> const value( std::numeric_limits<F>::infinity() );
  return value;
}

template<typename F>
zstring FloatImpl<F>::toIntegerString() const {
  if ( isNaN() )
    return nan_str();
  if ( isPosInf() )
    return pos_inf_str();
  if ( isNegInf() )
    return neg_inf_str();
  if ( isPosZero() )
    return "0";
  if ( isNegZero() )
    return "-0";
  return ztd::to_string( static_cast<long long>( value_ ) );
}

template<typename F>
zstring FloatImpl<F>::toString( bool no_scientific_format ) const {
  if ( isNaN() )
    return nan_str();
  if ( isPosInf() )
    return pos_inf_str();
  if ( isNegInf() )
    return neg_inf_str();
  if ( isPosZero() )
    return "0";
  if ( isNegZero() )
    return "-0";

  //
  // F&O 3.1 19.1.2.2: the result uses the shortest sequence of significant
  // digits that converts back to the same xs:float or xs:double value.
  // std::to_chars with a format but no precision produces exactly that.
  //
  char sci[64];
  std::to_chars_result const r =
    std::to_chars( sci, sci + sizeof sci, value_, std::chars_format::scientific );
  *r.ptr = '\0';

  // sci is: [-]d[.ddd]e(+|-)xx
  char const *p = sci;
  bool const negative = *p == '-';
  if ( negative )
    ++p;
  std::string digits;
  for ( ; *p != 'e'; ++p )
    if ( *p != '.' )
      digits += *p;
  int const exponent = std::atoi( p + 1 );
  while ( digits.size() > 1 && digits.back() == '0' )
    digits.pop_back();

  zstring result;
  if ( negative )
    result += '-';

  value_type const abs_val = fabs( value_ );
  value_type const lower = 0.000001f, upper = 1000000.0f;

  if ( no_scientific_format || (abs_val >= lower && abs_val < upper) ) {
    //
    // Converted to xs:decimal, then to xs:string: no exponent, and no
    // fractional part if it is zero.
    //
    int const ndigits = static_cast<int>( digits.size() );
    if ( exponent < 0 ) {
      result += "0.";
      result.append( static_cast<zstring::size_type>( -exponent - 1 ), '0' );
      result += digits.c_str();
    } else if ( exponent + 1 >= ndigits ) {
      result += digits.c_str();
      result.append( static_cast<zstring::size_type>( exponent + 1 - ndigits ),
                     '0' );
    } else {
      result.append( digits, 0, exponent + 1 );
      result += '.';
      result.append( digits, exponent + 1, std::string::npos );
    }
  } else {
    //
    // Mantissa with one digit before the point and at least one after it,
    // then "E" and the exponent without "+" or leading zeros, e.g., 1.0E7.
    //
    result += digits[0];
    result += '.';
    if ( digits.size() > 1 )
      result.append( digits, 1, std::string::npos );
    else
      result += '0';
    result += 'E';
    ascii::itoa_buf_type buf;
    result += ascii::itoa( exponent, buf );
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////

template class FloatImpl<double>;
template class FloatImpl<float>;

#ifdef WIN32
// exported for testing only
template class ZORBA_DLL_PUBLIC FloatImpl<double>;
template class ZORBA_DLL_PUBLIC FloatImpl<float>;
#endif /* WIN32 */

} // namespace zorba
/* vim:set et sw=2 ts=2: */
