
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2019 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Operations with undefined rationals are not supported. */

// -- FIXME: Throw in that case?

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > class Rational {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Rational (T numerator, T denominator) : n_ (numerator), d_ (denominator)
    {
        if (d_ < 0)  { n_ = -n_; d_ = -d_; }
        if (d_ == 0) { n_ = 0; }
        else if (n_ == 0) { d_ = 1; }
        else {
        //
        T g = Math::GCD (n_, d_);
        
        if (g < 1) { n_ = d_ = 0; }
        else {
            n_ = n_ / g;
            d_ = d_ / g;
        }
        //
        }
    }
    
    Rational (int n = 0) : n_ (n), d_ (1)
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Rational (const Rational < T > &) = default;
    Rational (Rational < T > &&) = default;
    Rational < T > & operator = (const Rational < T > &) = default;
    Rational < T > & operator = (Rational < T > &&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    bool isNegative() const
    {
        return (n_ < 0) && !isUndefined();
    }
    
    bool isZero() const
    {
        return (n_ == 0) && !isUndefined();
    }
    
    bool isUndefined() const
    {
        return (d_ <= 0);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    T getNumerator() const
    {
        return n_;
    }

    T getDenominator() const
    {
        return d_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    double asDouble() const
    {
        if (isUndefined()) { return std::numeric_limits < double >::quiet_NaN(); }
        else {
            return static_cast < double > (getNumerator()) / static_cast < double > (getDenominator());
        }
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Rational < T > operator += (Rational < T > r)
    {
        PRIM_ASSERT (!(*this).isUndefined() && !r.isUndefined());
        
        T lcm = Math::LCM (d_, r.d_);
        T d = lcm;
        T n = n_ * (lcm / d_) + r.n_ * (lcm / r.d_);
        
        *this = Rational < T > (n, d);
        
        return *this;
    }

    Rational < T > operator -= (Rational < T > r)
    {
        PRIM_ASSERT (!(*this).isUndefined() && !r.isUndefined());
        
        T lcm = Math::LCM (d_, r.d_);
        T d = lcm;
        T n = n_ * (lcm / d_) - r.n_ * (lcm / r.d_);
        
        *this = Rational < T > (n, d);
        
        return *this;
    }
    
    Rational < T > operator *= (Rational < T > r)
    {
        PRIM_ASSERT (!(*this).isUndefined() && !r.isUndefined());
        
        T n = n_ * r.n_;
        T d = d_ * r.d_;
        
        *this = Rational < T > (n, d);
        
        return *this;
    }
    
    Rational < T > operator /= (Rational < T > r)
    {
        PRIM_ASSERT (!(*this).isUndefined() && !r.isUndefined());
        
        T n = n_ * r.d_;
        T d = d_ * r.n_;
        
        *this = Rational < T > (n, d);
        
        return *this;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static const Rational < T > undefined()
    {
        return Rational < T > (0, 0);
    }
    
private:
    T n_;
    T d_;

private:
    PRIM_LEAK_DETECTOR (Rational)
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend bool operator != (Rational < T > a, Rational < T > b)
{
    PRIM_ASSERT (!a.isUndefined() && !b.isUndefined());
    
    return !(a == b);
}

friend bool operator == (Rational < T > a, Rational < T > b)
{
    PRIM_ASSERT (!a.isUndefined() && !b.isUndefined());
    
    return (a.n_ == b.n_ && a.d_ == b.d_);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend bool operator > (Rational < T > a, Rational < T > b)
{
    return ((a - b).n_ > 0);
}

friend bool operator >= (Rational < T > a, Rational < T > b)
{
    return ((a - b).n_ >= 0);
}
    
friend bool operator < (Rational < T > a, Rational < T > b)
{
    return ((a - b).n_ < 0);
}

friend bool operator <= (Rational < T > a, Rational < T > b)
{
    return ((a - b).n_ <= 0);
}
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend const Rational < T > operator + (Rational < T > a, Rational < T > b) 
{
    return a += b;
}

friend const Rational < T > operator - (Rational < T > a, Rational < T > b)
{
    return a -= b;
}

friend const Rational < T > operator * (Rational < T > a, Rational < T > b)
{
    return a *= b;
}

friend const Rational < T > operator / (Rational < T > a, Rational < T > b)
{
    return a /= b;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef Rational < int64 > Ratio;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
