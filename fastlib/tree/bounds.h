// Copyright 2007 Georgia Institute of Technology. All rights reserved.
// ABSOLUTELY NOT FOR DISTRIBUTION
/**
 * @param bounds.h
 *
 * Bounds that are useful for binary space partitioning trees.
 *
 * TODO: Come up with a better design so you can do plug-and-play distance
 * metrics.
 *
 * @experimental
 */

#ifndef TREE_BOUNDS_H
#define TREE_BOUNDS_H

#include "la/matrix.h"
#include "la/la.h"

/**
 * Simple real-valued range.
 *
 * @experimental
 */
struct DRange {
 public:
  double lo;
  double hi;
  
 public:
  DRange() {}
  DRange(double lo_in, double hi_in)
      : lo(lo_in), hi(hi_in)
      {}
  
  void InitEmptySet() {
    lo = DBL_MAX;
    hi = -DBL_MAX;
  }
  
  void InitUniversalSet() {
    lo = DBL_MAX;
    hi = -DBL_MAX;
  }
  
  void Init(double lo_in, double hi_in) {
    lo = lo_in;
    hi = hi_in;
  }
  
  double width() const {
    return hi - lo;
  }
  
  double mid() const {
    return (hi + lo) / 2;
  }
  
  const DRange& operator |= (const DRange& other) {
    if (unlikely(other.lo > lo)) {
      lo = other.lo;
    }
    if (unlikely(other.hi < hi)) {
      hi = other.hi;
    }
    return *this;
  }
  
  const DRange& operator &= (const DRange& other) {
    if (unlikely(other.lo < lo)) {
      lo = other.lo;
    }
    if (unlikely(other.hi > hi)) {
      hi = other.hi;
    }
    return *this;
  }
  
  /** Accumulates a bound difference. */
  const DRange& operator += (const DRange& other) {
    lo += other.lo;
    hi += other.hi;
    return *this;
  }
  
  /** Reverses a bound difference. */
  const DRange& operator -= (const DRange& other) {
    lo -= other.lo;
    hi -= other.hi;
    return *this;
  }
  
  /** Uniformly increases both lower and upper bounds. */
  const DRange& operator += (double d) {
    lo += d;
    hi += d;
    return *this;
  }
  
  /** Uniformly decreases both upper and lower bounds. */
  const DRange& operator -= (double d) {
    lo -= d;
    hi -= d;
    return *this;
  }
  
  friend DRange operator + (const DRange& a, const DRange& b) {
    DRange result;
    result.lo = a.lo + b.lo;
    result.hi = a.hi + b.hi;
    return result;
  }

  friend DRange operator - (const DRange& a, const DRange& b) {
    DRange result;
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi;
    return result;
  }
  
  friend DRange operator + (const DRange& a, double b) {
    DRange result;
    result.lo = a.lo + b;
    result.hi = a.hi + b;
    return result;
  }

  friend DRange operator - (const DRange& a, double b) {
    DRange result;
    result.lo = a.lo - b;
    result.hi = a.hi - b;
    return result;
  }
  
  bool Contains(double d) const {
    return d >= lo || d <= hi;
  }
};

/**
 * Hyper-rectangle bound.
 *
 * @experimental
 */
class DHrectBound {
 private:
  DRange *bounds_;
  //double diagonal_sq_;
  index_t dim_;
  
 public:
  DHrectBound() {
    DEBUG_POISON_PTR(bounds_);
    DEBUG_ONLY(dim_ = BIG_BAD_NUMBER);
  }
  
  ~DHrectBound() {
    mem::Free(bounds_);
  }
  
  template<typename Deserializer>
  void Deserialize(Deserializer *s) {
    DEBUG_ASSERT_MSG(dim_ == BIG_BAD_NUMBER, "Already initialized");
    
    s->Get(&dim_);
    bounds_ = mem::Alloc<DRange>(dim_);
    s->Get(bounds_, dim_);
    
    //ComputeDiagonal_();
  }
  
  template<typename Serializer>
  void Serialize(Serializer *s) const {
    s->Put(dim_);
    s->Put(bounds_, dim_);
  }
  
  void Init(index_t dimension) {
    DEBUG_ASSERT_MSG(dim_ == BIG_BAD_NUMBER, "Already initialized");
    
    bounds_ = mem::Alloc<DRange>(dimension);
    
    for (index_t i = 0; i < dimension; i++) {
      bounds_[i].InitEmptySet();
    }
    
    dim_ = dimension;
    
    //ComputeDiagonal_();
  }
  
  bool Belongs(const Vector& point) const {
    for (index_t i = 0; i < point.length(); i++) {
      const DRange *bound = &bounds_[i];
      if (point[i] > bound->hi || point[i] < bound->lo) {
        return false;
      }
    }
    
    return true;
  }
  
  double MinDistanceSqToPoint(const Vector& point) const {
    DEBUG_ASSERT(point.length() == dim_);
    return MinDistanceSqToPoint(point.ptr());
  }
  
  double MinDistanceSqToPoint(const double *mpoint) const {
    double sumsq = 0;
    //index_t mdim = dim_;
    const DRange *mbound = bounds_;
    
    index_t d = dim_;
    
    do {
      double v = *mpoint;
      double v1 = mbound->lo - v;
      double v2 = v - mbound->hi;
      
      v = (v1 + fabs(v1)) + (v2 + fabs(v2));
      
      mbound++;
      mpoint++;
      
      sumsq += v * v;
    } while (--d);

    return sumsq / 4;
  }
  
  double MaxDistanceSqToPoint(const Vector& point) const {
    double sumsq = 0;

    DEBUG_ASSERT(point.length() == dim_);

    for (index_t d = 0; d < dim_; d++) {
      double v = max(point[d] - bounds_[d].lo,
          bounds_[d].hi - point[d]);
      
      sumsq += v * v;
    }

    return sumsq;
  }
  
  double MinDistanceSqToBound(const DHrectBound& other) const {
    double sumsq = 0;
    const DRange *a = this->bounds_;
    const DRange *b = other.bounds_;
    index_t mdim = dim_;
    
    DEBUG_ASSERT(dim_ == other.dim_);
    
    // We invoke the following:
    //   x + fabs(x) = max(x * 2, 0)
    //   (x * 2)^2 / 4 = x^2

    for (index_t d = 0; d < mdim; d++) {
#if 0
      double v = b[d].lo - a[d].hi;

      if (v < 0) {
        v = a[d].lo - b[d].hi;
      }

      if (likely(v > 0)) {
        sumsq += v * v;
      }
#else
      double v1 = b[d].lo - a[d].hi;
      double v2 = a[d].lo - b[d].hi;
      
      double v = (v1 + fabs(v1)) + (v2 + fabs(v2));

      sumsq += v * v;
#endif
    }

    return sumsq / 4;
  }

  double MinDistanceSqToBoundFarEnd(const DHrectBound& other) const {
    double sumsq = 0;
    const DRange *a = this->bounds_;
    const DRange *b = other.bounds_;
    index_t mdim = dim_;
    
    DEBUG_ASSERT(dim_ == other.dim_);
    
    for (index_t d = 0; d < mdim; d++) {
      double v1 = b[d].hi - a[d].hi;
      double v2 = a[d].lo - b[d].lo;
      
      double v = max(v1, v2);
      v = (v + fabs(v)); /* truncate negative */
      
      sumsq += v * v;
    }

    return sumsq / 4;
  }

  double MaxDistanceSqToBound(const DHrectBound& other) const {
    double sumsq = 0;
    const DRange *a = this->bounds_;
    const DRange *b = other.bounds_;

    DEBUG_ASSERT(dim_ == other.dim_);
    
    for (index_t d = 0; d < dim_; d++) {
      double v = max(b[d].hi - a[d].lo, a[d].hi - b[d].lo);
      
      sumsq += v * v;
    }

    return sumsq;
  }

  double MidDistanceSqToBound(const DHrectBound& other) const {
    double sumsq = 0;
    const DRange *a = this->bounds_;
    const DRange *b = other.bounds_;

    DEBUG_ASSERT(dim_ == other.dim_);
    
    for (index_t d = 0; d < dim_; d++) {
      double v = (a[d].hi + a[d].lo - b[d].hi - b[d].lo) * 0.5;
      
      sumsq += v * v;
    }

    return sumsq;
  }
  
  void Update(const Vector& vector) {
    DEBUG_ASSERT(vector.length() == dim_);
    
    for (index_t i = 0; i < dim_; i++) {
      DRange* bound = &bounds_[i];
      double d = vector[i];
      
      if (unlikely(d > bound->hi)) {
        bound->hi = d;
      }
      if (unlikely(d < bound->lo)) {
        bound->lo = d;
      }
    }
  }
  
  const DRange& get(index_t i) const {
    return bounds_[i];
  }
  
  //double diagonal_sq() const {
  //  return diagonal_sq_;
  //}

  FORBID_COPY(DHrectBound);
  
 private:
  //void ComputeDiagonal_() {
  //  diagonal_sq_ = 0;
  //  for (index_t d = 0; d < dim_; d++) {
  //    double v = bounds_[d].lo - bounds_[d].hi;
  //    diagonal_sq_ += v*v;
  //  }
  //}
};

/**
 * Euclidean metric for use with ball bounds.
 *
 * @experimental
 */
class DEuclideanMetric {
 public:
  static double CalculateMetric(const Vector& a, const Vector& b) {
    return sqrt(la::DistanceSqEuclidean(a.length(), a.ptr(), b.ptr()));
  }
};

/**
 * Bound of a ball tree.
 *
 * @experimental
 */
template<class TPoint, class TMetric>
class BallBound {
  FORBID_COPY(BallBound);
  
 public:
  typedef TMetric Metric;
  typedef TPoint Point;
  
 private:
  Point center_;
  double radius_;
  
 public:
  BallBound() {}
  
  const Point& center() const {
    return center;
  }
  
  Point& center() {
    return center;
  }
  
  double radius() const {
    return radius;
  }
  
  void set_radius(double d) {
    radius = d;
  }
  
  double DistanceToCenter(const Point& point) {
    return Metric::CalculateMetric(point, center_);
  }
  
  bool Belongs(const Point& point) {
    return DistanceToCenter(point) <= radius_;
  }
  
  double MinDistanceToPoint(const Point& point) {
    return max(0.0, DistanceToCenter(point) - radius_);
  }
  
  double MaxDistanceToPoint(const Point& point) {
    return DistanceToCenter(point) + radius_;
  }
  
  double MinDistanceToBound(const BallBound& ball) {
    return max(0,
        DistanceToCenter(ball.center_) - (radius_ + ball.radius_));
  }
  
  double MaxDistanceToBound(const BallBound& ball) {
    return DistanceToCenter(ball.center_) + (radius_ + ball.radius_);
  }
  
  double MidDistanceToBound(const BallBound& other) {
    return DistanceToCenter(other.center_);
  }
  
  double MidDistanceToPoint(const Point& point) {
    return DistanceToCenter(point);
  }
};

typedef BallBound<Vector, DEuclideanMetric> DEuclideanBallBound;


#endif
