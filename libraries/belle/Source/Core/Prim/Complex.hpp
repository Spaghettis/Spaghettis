
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2018 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PRIM_COMPLEX_HPP
#define PRIM_COMPLEX_HPP

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace prim {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

template < class T > class Complex {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    Complex (T x = T(), T y = T()) : x_ (x), y_ (y)
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PRIM_CPP11

public:
    Complex (const Complex < T > &) = default;
    Complex (Complex < T > &&) = default;
    Complex < T > & operator = (const Complex < T > &) = default;
    Complex < T > & operator = (Complex < T > &&) = default;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    T& getX()
    { 
        return x_; 
    }
    
    const T& getX() const
    { 
        return x_; 
    }
    
    T& getY() 
    { 
        return y_; 
    }
    
    const T& getY() const
    { 
        return y_; 
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    const Complex < T > operator += (Complex < T > c)
    {
        x_ += c.x_;
        y_ += c.y_;
        
        return *this;
    }

    const Complex < T > operator -= (Complex < T > c)
    {
        x_ -= c.x_;
        y_ -= c.y_;
        
        return *this;
    }
    
    const Complex < T > operator *= (Complex < T > c)
    {
        T x = (x_ * c.x_ - y_ * c.y_);
        T y = (x_ * c.y_ + c.x_ * y_);
        
        x_ = x;
        y_ = y;
        
        return *this;
    }
    
    const Complex < T > operator /= (Complex < T > c)
    {
        double d = (c.x_ * c.x_) + (c.y_ * c.y_);
        T x = static_cast < T > ((x_ * c.x_ + y_ * c.y_) / d);
        T y = static_cast < T > ((c.x_ * y_ - x_ * c.y_) / d);
        
        x_ = x;
        y_ = y;
        
        return *this;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    double angle() const
    {
        double t = atan2 (y_, x_);
        if (t < 0.0) { t += kTwoPi; }
        return t;
    }
    
    double magnitude() const
    {
        double x = static_cast < double > (x_);
        double y = static_cast < double > (y_);
        return sqrt (x * x + y * y);
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    void setAngle (double a)
    {
        setPolar (a, magnitude());
    }
    
    void setMagnitude (double m)
    {
        setPolar (angle(), m);
    }

    void setPolar (double a, double m)
    {
        *this = Complex < T >::withPolar (a, m);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    const Complex < T > operator -()
    {
        return Complex (-x_, -y_);
    }
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    static Complex < T > withPolar (double a, double m)
    {
        return Complex < T > (T (std::cos (a) * m), T (std::sin (a) * m));
    }

private:
    T x_;
    T y_;

private:
    PRIM_LEAK_DETECTOR (Complex)
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend bool operator != (Complex < T > a, Complex < T > b)
{
    return !(a == b);
}

friend bool operator == (Complex < T > a, Complex < T > b) 
{
    return (a.x_ == b.x_) && (a.y_ == b.y_);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend const Complex < T > operator + (Complex < T > a, Complex < T > b) 
{
    return a += b;
}

friend const Complex < T > operator - (Complex < T > a, Complex < T > b)
{
    return a -= b;
}

friend const Complex < T > operator * (Complex < T > a, Complex < T > b)
{
    return a *= b;
}

friend const Complex < T > operator / (Complex < T > a, Complex < T > b)
{
    return a /= b;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.cs.princeton.edu/~rs/AlgsDS07/16Geometric.pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

friend int clockwiseOrder (Complex < T > a, Complex < T > b, Complex < T > c) 
{
    double t1 = (static_cast < double > (b.x_) - static_cast < double > (a.x_));
    double t2 = (static_cast < double > (c.y_) - static_cast < double > (a.y_));
    double t3 = (static_cast < double > (b.y_) - static_cast < double > (a.y_)); 
    double t4 = (static_cast < double > (c.x_) - static_cast < double > (a.x_)); 
    
    double d = (t1 * t2) - (t3 * t4);
     
    if (d < 0) { return 1; }            /* Clockwise. */
    else if (d > 0) { return -1; }      /* Counterclockwise. */
    else { 
        return 0;                       /* Collinear. */
    }
}
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace prim

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PRIM_COMPLEX_HPP
