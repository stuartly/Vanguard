#ifndef _BIMAP_H
#define _BIMAP_H

#include <map>

template <class X, class Y>
class Bimap {
private:
  std::map<X, Y> x_y;
  std::map<Y, X> y_x;

public:
  void add(X x, Y y);
  void remove(X x, Y y);
  X getLeft(Y y);
  Y getRight(X x);
};

template <class X, class Y>
void Bimap<X, Y>::add(X x, Y y) {
  x_y.insert(std::pair<X, Y>(x, y));
  y_x.insert(std::pair<Y, X>(y, x));
}

template <class X, class Y>
void Bimap<X, Y>::remove(X x, Y y) {
  x_y.erase(x);
  y_x.erase(x);
}

template <class X, class Y>
X Bimap<X, Y>::getLeft(Y y) {
  return y_x.at(y);
}

template <class X, class Y>
Y Bimap<X, Y>::getRight(X x) {
  return x_y.at(x);
}

#endif
