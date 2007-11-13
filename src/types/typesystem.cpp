
#include <assert.h>
#include "zorba_api.h"
#include "store/naive/atomic_items.h"
#include "node_test.h"
#include "store/api/item_factory.h"
#include "store/api/store.h"
#include "typesystem.h"

using namespace xqp;

#define T true
#define F false

const bool TypeSystem::ATOMIC_SUBTYPE_MATRIX[45][45] = {
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* anyAtomicType */
  {T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* string */
  {T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* normalizedString */
  {T, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* token */
  {T, T, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* language */
  {T, T, T, T, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* NMTOKEN */
  {T, T, T, T, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* Name */
  {T, T, T, T, F, F, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* NCName */
  {T, T, T, T, F, F, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* ID */
  {T, T, T, T, F, F, T, T, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* IDREF */
  {T, T, T, T, F, F, T, T, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* ENTITY */
  {T, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* untypedAtomic */
  {T, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* dateTime */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* date */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* time */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* duration */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* yearMonthDuration */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* dayTimeDuration */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* float */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* double */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* decimal */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* integer */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* nonPositiveInteger */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* negativeInteger */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* long */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* int */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* short */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, T, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* byte */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* nonNegativeInteger */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* unsignedLong */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* unsignedInt */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, T, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F, F}, /* unsignedShort */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, T, T, T, T, T, F, F, F, F, F, F, F, F, F, F, F, F}, /* unsignedByte */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, T, F, F, F, F, F, F, T, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F, F}, /* positiveInteger */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F, F}, /* gYearMonth */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F, F}, /* gYear */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F, F}, /* gMonthDay */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F, F}, /* gDay */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F, F}, /* gMonth */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, F}, /* boolean */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F}, /* base64Binary */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F, F}, /* hexBinary */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F, F}, /* anyURI */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T, F}, /* QName */
  {T, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, T}, /* NOTATION */
};

const bool TypeSystem::QUANT_SUBTYPE_MATRIX[5][5] = {
  {T, F, T, T, F}, /* QUANT_ZERO */
  {F, T, T, T, T}, /* QUANT_ONE */
  {F, F, T, T, F}, /* QUANT_QUESTION */
  {F, F, F, T, F}, /* QUANT_STAR */
  {F, F, F, T, T}, /* QUANT_PLUS */
};

#undef T
#undef F

TypeSystem::TypeSystem()
{
#define XS_URI "http://www.w3.org/2001/XMLSchema"
#define XS_PREFIX "xs"

#define XSQNDECL(var, local)                         \
        QNameItem_t var =                    \
            Store::getInstance().getItemFactory().   \
            createQName(XS_URI, XS_PREFIX, local)

  XSQNDECL(XS_ANY_ATOMIC_QNAME, "anyAtomicType");
  XSQNDECL(XS_STRING_QNAME, "string");
  XSQNDECL(XS_NORMALIZED_STRING_QNAME, "normalizedString");
  XSQNDECL(XS_TOKEN_QNAME, "token");
  XSQNDECL(XS_LANGUAGE_QNAME, "language");
  XSQNDECL(XS_NMTOKEN_QNAME, "NMTOKEN");
  XSQNDECL(XS_NAME_QNAME, "Name");
  XSQNDECL(XS_NCNAME_QNAME, "NCName");
  XSQNDECL(XS_ID_QNAME, "ID");
  XSQNDECL(XS_IDREF_QNAME, "IDREF");
  XSQNDECL(XS_ENTITY_QNAME, "ENTITY");
  XSQNDECL(XS_UNTYPED_ATOMIC_QNAME, "untypedAtomic");
  XSQNDECL(XS_DATETIME_QNAME, "dateTime");
  XSQNDECL(XS_DATE_QNAME, "date");
  XSQNDECL(XS_TIME_QNAME, "time");
  XSQNDECL(XS_DURATION_QNAME, "duration");
  XSQNDECL(XS_DT_DURATION_QNAME, "dayTimeDuration");
  XSQNDECL(XS_YM_DURATION_QNAME, "yearMonthDuration");
  XSQNDECL(XS_FLOAT_QNAME, "float");
  XSQNDECL(XS_DOUBLE_QNAME, "double");
  XSQNDECL(XS_DECIMAL_QNAME, "decimal");
  XSQNDECL(XS_INTEGER_QNAME, "integer");
  XSQNDECL(XS_NON_POSITIVE_INTEGER_QNAME, "nonPositiveInteger");
  XSQNDECL(XS_NEGATIVE_INTEGER_QNAME, "negativeInteger");
  XSQNDECL(XS_LONG_QNAME, "long");
  XSQNDECL(XS_INT_QNAME, "int");
  XSQNDECL(XS_SHORT_QNAME, "short");
  XSQNDECL(XS_BYTE_QNAME, "byte");
  XSQNDECL(XS_NON_NEGATIVE_INTEGER_QNAME, "nonNegativeInteger");
  XSQNDECL(XS_UNSIGNED_LONG_QNAME, "unsignedLong");
  XSQNDECL(XS_UNSIGNED_INT_QNAME, "unsignedInt");
  XSQNDECL(XS_UNSIGNED_SHORT_QNAME, "unsignedShort");
  XSQNDECL(XS_UNSIGNED_BYTE_QNAME, "unsignedByte");
  XSQNDECL(XS_POSITIVE_INTEGER_QNAME, "positiveInteger");
  XSQNDECL(XS_GYEAR_MONTH_QNAME, "gYearMonth");
  XSQNDECL(XS_GYEAR_QNAME, "gYear");
  XSQNDECL(XS_GMONTH_DAY_QNAME, "gMonthDay");
  XSQNDECL(XS_GDAY_QNAME, "gDay");
  XSQNDECL(XS_GMONTH_QNAME, "gMonth");
  XSQNDECL(XS_BOOLEAN_QNAME, "boolean");
  XSQNDECL(XS_BASE64BINARY_QNAME, "base64Binary");
  XSQNDECL(XS_HEXBINARY_QNAME, "hexBinary");
  XSQNDECL(XS_ANY_URI_QNAME, "anyURI");
  XSQNDECL(XS_QNAME_QNAME, "QName");
  XSQNDECL(XS_NOTATION_QNAME, "notation");

#define ATOMIC_TYPE_DEFN(basename) \
  basename##_TYPE_ONE = new AtomicXQType(XS_##basename, QUANT_ONE); \
  basename##_TYPE_QUESTION = new AtomicXQType(XS_##basename, QUANT_QUESTION); \
  basename##_TYPE_STAR = new AtomicXQType(XS_##basename, QUANT_STAR); \
  basename##_TYPE_PLUS = new AtomicXQType(XS_##basename, QUANT_PLUS); \
  m_atomic_qnametype_map[XS_##basename##_QNAME] = XS_##basename; \
  m_atomic_typecode_map[XS_##basename][QUANT_ONE] = &basename##_TYPE_ONE; \
  m_atomic_typecode_map[XS_##basename][QUANT_QUESTION] = &basename##_TYPE_QUESTION; \
  m_atomic_typecode_map[XS_##basename][QUANT_STAR] = &basename##_TYPE_STAR; \
  m_atomic_typecode_map[XS_##basename][QUANT_PLUS] = &basename##_TYPE_PLUS;

  ATOMIC_TYPE_DEFN(ANY_ATOMIC)
  ATOMIC_TYPE_DEFN(STRING)
  ATOMIC_TYPE_DEFN(NORMALIZED_STRING)
  ATOMIC_TYPE_DEFN(TOKEN)
  ATOMIC_TYPE_DEFN(LANGUAGE)
  ATOMIC_TYPE_DEFN(NMTOKEN)
  ATOMIC_TYPE_DEFN(NAME)
  ATOMIC_TYPE_DEFN(NCNAME)
  ATOMIC_TYPE_DEFN(ID)
  ATOMIC_TYPE_DEFN(IDREF)
  ATOMIC_TYPE_DEFN(ENTITY)
  ATOMIC_TYPE_DEFN(UNTYPED_ATOMIC)
  ATOMIC_TYPE_DEFN(DATETIME)
  ATOMIC_TYPE_DEFN(DATE)
  ATOMIC_TYPE_DEFN(TIME)
  ATOMIC_TYPE_DEFN(DURATION)
  ATOMIC_TYPE_DEFN(DT_DURATION)
  ATOMIC_TYPE_DEFN(YM_DURATION)
  ATOMIC_TYPE_DEFN(FLOAT)
  ATOMIC_TYPE_DEFN(DOUBLE)
  ATOMIC_TYPE_DEFN(DECIMAL)
  ATOMIC_TYPE_DEFN(INTEGER)
  ATOMIC_TYPE_DEFN(NON_POSITIVE_INTEGER)
  ATOMIC_TYPE_DEFN(NEGATIVE_INTEGER)
  ATOMIC_TYPE_DEFN(LONG)
  ATOMIC_TYPE_DEFN(INT)
  ATOMIC_TYPE_DEFN(SHORT)
  ATOMIC_TYPE_DEFN(BYTE)
  ATOMIC_TYPE_DEFN(NON_NEGATIVE_INTEGER)
  ATOMIC_TYPE_DEFN(UNSIGNED_LONG)
  ATOMIC_TYPE_DEFN(UNSIGNED_INT)
  ATOMIC_TYPE_DEFN(UNSIGNED_SHORT)
  ATOMIC_TYPE_DEFN(UNSIGNED_BYTE)
  ATOMIC_TYPE_DEFN(POSITIVE_INTEGER)
  ATOMIC_TYPE_DEFN(GYEAR_MONTH)
  ATOMIC_TYPE_DEFN(GYEAR)
  ATOMIC_TYPE_DEFN(GMONTH_DAY)
  ATOMIC_TYPE_DEFN(GDAY)
  ATOMIC_TYPE_DEFN(GMONTH)
  ATOMIC_TYPE_DEFN(BOOLEAN)
  ATOMIC_TYPE_DEFN(BASE64BINARY)
  ATOMIC_TYPE_DEFN(HEXBINARY)
  ATOMIC_TYPE_DEFN(ANY_URI)
  ATOMIC_TYPE_DEFN(QNAME)
  ATOMIC_TYPE_DEFN(NOTATION)
#undef ATOMIC_TYPE_DEFN
}

TypeSystem::~TypeSystem()
{
}

TypeSystem::quantifier_t TypeSystem::quantifier(const XQType &type) const
{
  return type.m_quantifier;
}

bool TypeSystem::is_equal(const XQType& type1, const XQType& type2) const
{
  if (type1.m_quantifier != type2.m_quantifier) {
    return false;
  }
  if (type1.type_kind() != type2.type_kind()) {
    return false;
  }
  switch(type1.type_kind()) {
    case XQType::ATOMIC_TYPE_KIND:
    {
      const AtomicXQType& a1 = static_cast<const AtomicXQType&>(type1);
      const AtomicXQType& a2 = static_cast<const AtomicXQType&>(type2);
      return a1.m_type_code == a2.m_type_code;
    }
    case XQType::NODE_TYPE_KIND:
    {
      const NodeXQType& n1 = static_cast<const NodeXQType&>(type1);
      const NodeXQType& n2 = static_cast<const NodeXQType&>(type2);
      return *n1.m_nodetest == *n2.m_nodetest;
    }
    default:
      break;
  }
  return true;
}

bool TypeSystem::is_subtype(const XQType& subtype, const XQType& supertype) const
{
  if (!QUANT_SUBTYPE_MATRIX[subtype.m_quantifier][supertype.m_quantifier]) {
    return false;
  }
  switch(supertype.type_kind()) {
    case XQType::ATOMIC_TYPE_KIND:
      switch(subtype.type_kind()) {
        case XQType::ATOMIC_TYPE_KIND:
        {
          const AtomicXQType& a1 = static_cast<const AtomicXQType&>(subtype);
          const AtomicXQType& a2 = static_cast<const AtomicXQType&>(supertype);
          return ATOMIC_SUBTYPE_MATRIX[a1.m_type_code][a2.m_type_code];
        }
        case XQType::NODE_TYPE_KIND:
        case XQType::ANY_TYPE_KIND:
        case XQType::ITEM_KIND:
        case XQType::ANY_SIMPLE_TYPE_KIND:
        case XQType::UNTYPED_KIND:
          return false;
      }
      break;

    case XQType::NODE_TYPE_KIND:
      switch(subtype.type_kind()) {
        case XQType::NODE_TYPE_KIND:
        {
          const NodeXQType& n1 = static_cast<const NodeXQType&>(subtype);
          const NodeXQType& n2 = static_cast<const NodeXQType&>(supertype);
          return n1.m_nodetest->is_sub_nodetest_of(*n2.m_nodetest);
        }
        case XQType::ATOMIC_TYPE_KIND:
        case XQType::ANY_TYPE_KIND:
        case XQType::ITEM_KIND:
        case XQType::ANY_SIMPLE_TYPE_KIND:
        case XQType::UNTYPED_KIND:
          return false;
      }
      break;

    case XQType::ANY_TYPE_KIND:
      switch(subtype.type_kind()) {
        case XQType::ATOMIC_TYPE_KIND:
        case XQType::NODE_TYPE_KIND:
        case XQType::ANY_TYPE_KIND:
        case XQType::ANY_SIMPLE_TYPE_KIND:
        case XQType::UNTYPED_KIND:
          return true;

        case XQType::ITEM_KIND:
          return false;
      }
      break;

    case XQType::ITEM_KIND:
      switch(subtype.type_kind()) {
        case XQType::ATOMIC_TYPE_KIND:
        case XQType::NODE_TYPE_KIND:
        case XQType::ITEM_KIND:
          return true;

        case XQType::ANY_TYPE_KIND:
        case XQType::ANY_SIMPLE_TYPE_KIND:
        case XQType::UNTYPED_KIND:
          return false;
      }
      break;

    case XQType::ANY_SIMPLE_TYPE_KIND:
      switch(subtype.type_kind()) {
        case XQType::ATOMIC_TYPE_KIND:
        case XQType::ANY_SIMPLE_TYPE_KIND:
          return true;

        case XQType::NODE_TYPE_KIND:
        case XQType::ANY_TYPE_KIND:
        case XQType::ITEM_KIND:
        case XQType::UNTYPED_KIND:
          return false;
      }
      break;

    case XQType::UNTYPED_KIND:
      switch(subtype.type_kind()) {
        case XQType::UNTYPED_KIND:
          return true;

        case XQType::ATOMIC_TYPE_KIND:
        case XQType::NODE_TYPE_KIND:
        case XQType::ANY_TYPE_KIND:
        case XQType::ITEM_KIND:
        case XQType::ANY_SIMPLE_TYPE_KIND:
          return false;
      }
      break;
  }
  return false;
}

bool TypeSystem::is_promotable(const XQType& srctype, const XQType& targettype) const
{
  return false;
}

bool TypeSystem::is_atomic(const XQType& type) const
{
  return type.m_quantifier == QUANT_ONE && type.type_kind() == XQType::ATOMIC_TYPE_KIND;
}

bool TypeSystem::is_simple(const XQType& type) const
{
  return type.type_kind() == XQType::ATOMIC_TYPE_KIND;
}

bool TypeSystem::is_numeric(const XQType& type) const
{
  return is_subtype(*DOUBLE_TYPE_ONE, type)
    || is_subtype(*FLOAT_TYPE_ONE, type)
    || is_subtype(*DECIMAL_TYPE_ONE, type);
}

TypeSystem::atomic_type_code_t TypeSystem::get_atomic_type_code(const XQType& type) const
{
  assert(type.type_kind() == XQType::ATOMIC_TYPE_KIND);
  return (static_cast<const AtomicXQType&>(type)).m_type_code;
}

TypeSystem::xqtref_t TypeSystem::union_type(const XQType& type1, const XQType& type2) const
{
  return TypeSystem::xqtref_t(0);
}

TypeSystem::xqtref_t TypeSystem::intersect_type(const XQType& type1, const XQType& type2) const
{
  return TypeSystem::xqtref_t(0);
}

TypeSystem::xqtref_t TypeSystem::prime_type(const XQType& type) const
{
  return TypeSystem::xqtref_t(0);
}

TypeSystem::xqtref_t TypeSystem::arithmetic_type(const XQType& type1, const XQType& type2) const
{
  if (is_subtype(*UNTYPED_ATOMIC_TYPE_ONE, type1)
    || is_subtype(*UNTYPED_ATOMIC_TYPE_ONE, type2)
    || is_subtype(*DOUBLE_TYPE_ONE, type1)
    || is_subtype(*DOUBLE_TYPE_ONE, type2)) {
    return DOUBLE_TYPE_ONE;
  }
  if (is_subtype(*FLOAT_TYPE_ONE, type1)
    || is_subtype(*FLOAT_TYPE_ONE, type2)) {
    return FLOAT_TYPE_ONE;
  }
  if ((is_subtype(*DECIMAL_TYPE_ONE, type1)
    && !is_subtype(*INTEGER_TYPE_ONE, type1))
    || (is_subtype(*DECIMAL_TYPE_ONE, type2))
    && !is_subtype(*INTEGER_TYPE_ONE, type2)) {
    return DECIMAL_TYPE_ONE;
  }

  return INTEGER_TYPE_ONE;
}

rchandle<NodeNameTest> TypeSystem::get_nametest(const XQType& type) const
{
  return rchandle<NodeNameTest>(0);
}

NodeXQType::NodeXQType(rchandle<NodeTest> nodetest, TypeSystem::quantifier_t quantifier) : XQType(quantifier), m_nodetest(nodetest)
{
}

TypeSystem::xqtref_t TypeSystem::create_node_type(rchandle<NodeTest> nodetest, TypeSystem::quantifier_t quantifier) const
{
  return new NodeXQType(nodetest, quantifier);
}

/* vim:set ts=2 sw=2: */
