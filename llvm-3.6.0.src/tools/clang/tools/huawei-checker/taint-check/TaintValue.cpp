
#include "taint-check/TaintValue.h"

namespace taint {

TaintValue operator+(const TaintValue &v1, const TaintValue &v2) {
  if (v1.getTaintKind() == TaintKind::Tainted ||
      v2.getTaintKind() == TaintKind::Tainted)
    return TaintValue(TaintKind::Tainted);
  return TaintValue(v1.value | v2.value);
}

bool operator==(const TaintValue &v1, const TaintValue &v2) {
  return v1.value == v2.value;
}

bool operator!=(const TaintValue &v1, const TaintValue &v2) {
  return v1.value != v2.value;
}

TaintValueV operator+(const TaintValueV &v1, const TaintValueV &v2) {

  TaintValueV result(ValueKind::Temp);
  result.value->update(*v1.value + *v2.value);

  return result;
}

bool operator==(const TaintValueV &v1, const TaintValueV &v2) {

  assert(v1.V == v2.V && v1.kind == v2.kind);
  return *v1.value == *v2.value;
}

bool operator!=(const TaintValueV &v1, const TaintValueV &v2) {
  return !(v1 == v2);
}

TaintValue combineValues(TaintValueV value, std::vector<TaintValueV> values) {
  if (value.getTaintKind() == TaintKind::Tainted)
    return TaintValue(TaintKind::Tainted);

  unsigned result = 0;
  unsigned v = value.getValue();
  unsigned i = 0;
  while (v != 0 && i < values.size()) {
    result = result | (values[i].getValue() & (v % 2));
    v = v / 2;
    i++;
  }
  return TaintValue(result);
}
}
