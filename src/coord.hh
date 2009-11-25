// coord.hh
//
// Copyright 2009 Martin Read

#ifndef LIBMRL_COORD_H
#define LIBMRL_COORD_H
#include "mathops.hh"

namespace libmrl
{
    template <typename T> class basic_coord
    {
    public:
        typedef basic_coord<T>& ref;
        typedef const basic_coord<T>& const_ref;
        typedef T& baseref;
        typedef const T& const_baseref;
        int y;
        int x;
        ref operator -=(const_ref right)
        {
            y -= right.y;
            x -= right.x;
            return *this;
        }
        ref operator +=(const_ref right)
        {
            y += right.y;
            x += right.x;
            return *this;
        }
        ref operator /=(const_baseref right)
        {
            y /= right;
            x /= right;
            return *this;
        }
        ref operator *=(const_baseref right)
        {
            y *= right;
            x *= right;
            return *this;
        }
        int distance(const_ref far) const
        {
            return libmrl::max(libmrl::abs(y - far.y), libmrl::abs(x - far.x));
        };
        bool operator < (const_ref right) const
        {
            return (y < right.y) || ((y == right.y) && (x < right.x));
        }
        bool operator > (const_ref right) const
        {
            return (y > right.y) || ((y == right.y) && (x > right.x));
        }
        bool operator == (const_ref right) const
        {
            return (y == right.y) && (x == right.x);
        }
        bool operator != (const_ref right) const
        {
            return (y != right.y) || (x != right.x);
        }
        bool cardinal() const
        {
            return (x == 0) || (y == 0) || (libmrl::abs(y) == libmrl::abs(x));
        }
        operator T() const
        {
            return libmrl::max(libmrl::abs(y), libmrl::abs(x));
        }
    };

    typedef basic_coord<int> Coord;

    template <typename T> inline basic_coord<T> operator -(const basic_coord<T>& left, const basic_coord<T>& right)
    {
        basic_coord<T> result = left;
        result -= right;
        return result;
    }
    template <typename T> inline basic_coord<T> operator +(const basic_coord<T>& left, const basic_coord<T>& right)
    {
        basic_coord<T> result = left;
        result += right;
        return result;
    }
    template <> inline Coord abs<Coord> (const Coord& orig)
    {
        Coord tmp = { abs(orig.y), abs(orig.x) };
        return tmp;
    }
    template <> inline Coord sign<Coord> (const Coord& orig)
    {
        Coord tmp = { sign(orig.y), sign(orig.x) };
        return tmp;
    }
    extern const Coord NOWHERE;
    extern const Coord NORTH;
    extern const Coord WEST;
    extern const Coord EAST;
    extern const Coord SOUTH;
    extern const Coord NORTHEAST;
    extern const Coord NORTHWEST;
    extern const Coord SOUTHEAST;
    extern const Coord SOUTHWEST;
    extern const Coord NOWHERE;
}

#endif

// coord.hh
