// mathops.hh
//
// Copyright 2009 Martin Read

#ifndef LIBMRL_MATHOPS_HH
#define LIBMRL_MATHOPS_HH

namespace libmrl
{
    template <typename T> inline T max(const T& left, const T& right)
    {
        return (left < right) ? right : left;
    }
    template <typename T> inline T min(const T& left, const T& right)
    {
        return (left < right) ? left : right;
    }
    template <typename T> inline T abs(const T& orig)
    {
        return (orig < 0) ? -orig : orig;
    }
    template <typename T> inline T sign(const T& orig)
    {
        return (orig != 0) ? ((orig < 0) ? T(-1) : T(1)) : T(0);
    }
}

#endif

// mathops.hh
