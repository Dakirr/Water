#ifndef FIXED_H
#define FIXED_H
#include <cstdint>
#include <limits>
#include <array>
#include <iostream>
#include <utility>
#include <random>
#include <fstream>
extern int optimise;


#include "FastFixed.h"

using namespace std;
extern mt19937 rnd;
// extern std::ifstream file_random;
// extern size_t cur_r;

#if (defined(__clang__) || defined(__GNUC__)) && defined(__SIZEOF_INT128__)

using uint128_t = __uint128_t;
using int128_t  = __int128_t;

#elif defined(_MSC_VER) && defined(__has_include)

#if __has_include(<__msvc_int128.hpp>)

#include <__msvc_int128.hpp>
using uint128_t = std::_Unsigned128;
using int128_t  = std::_Signed128;

#endif

#endif


template <int N, int K>
class Fixed {
    public:
    static constexpr std::size_t kNValue = N;
    static constexpr std::size_t kKValue = K;
    static constexpr bool kFast = false;
    static constexpr int128_t mask =  ((((int128_t) 1) << K) - 1); 
    using IntType = 
    std::conditional_t<
        (N <= 8), int8_t,
        std::conditional_t<
            (N <= 16), int16_t,
            std::conditional_t<
                (N <= 32), int32_t,
                std::conditional_t<
                    (N <= 64), int64_t,
                    std::conditional_t<
                        (N <= 128), int128_t,
                        void
                    >     
                >
            >
        >
    >;
    
    IntType v;
    constexpr Fixed(int v): v(((int128_t) v) << K) {}
    constexpr Fixed(float f): v(f * (((int128_t) 1) << K)) {}
    constexpr Fixed(double f): v(f * (((int128_t) 1) << K)) {}
    
    template <int N2, int K2>
    constexpr Fixed(Fixed<N2, K2> other) {
        this->v = (((int128_t) other.v) << K) >> K2; 
    }

    template <int N2, int K2>
    constexpr Fixed(FastFixed<N2, K2> other) {
        this->v = (((int128_t) other.v) << K) >> K2; 
    }

    constexpr Fixed(): v(0) {}

    // template <typename T>
    // std::strong_ordering operator<=>(const T& other) const {
    //     return v <=> Fixed<N, K>(other);
    // };

    // template <int N2, int K2>
    // std::strong_ordering operator<=>(const Fixed<N2, K2>& other) const {
    //     return v <=> other.v;
    // };

    bool operator>(const Fixed<N, K> &other) const {
        return v > other.v;
    };

    template <typename T>
    bool operator>(const T& other) const {
        return v > Fixed<N, K>(other).v;
    };

    bool operator<(const Fixed<N, K> &other) const {
        return v < other.v;
    };

    template <typename T>
    bool operator<(const T& other) const {
        return v < Fixed<N, K>(other).v;
    };

    bool operator>=(const Fixed<N, K> &other) const {
        return v >= other.v;
    };

    template <typename T>
    bool operator>=(const T& other) const {
        return v >= Fixed<N, K>(other).v;
    };

    bool operator<=(const Fixed<N, K> &other) const {
        return v <= other.v;
    };

    template <typename T>
    bool operator<=(const T& other) const {
        return v <= Fixed<N, K>(other).v;
    };

    bool operator!=(const Fixed<N, K> &other) const {
        return v != other.v;
    };

    template <typename T>
    bool operator!=(const T& other) const {
        return v != Fixed<N, K>(other).v;
    };

    bool operator==(const Fixed<N, K>& other) const {
        return v == other.v;
    };


    template <typename T>
    bool operator==(const T& other) const {
        return v == Fixed<N, K>(other).v;
    };

    std::string type_str() {
        return "FIXED(" + std::to_string(N) + "," + std::to_string(K) + ")"; 
    };

    constexpr operator float() const {
        return (float)v / (1 << K);
    }

    constexpr explicit operator double() const {
        return (double)v / (1 << K);
    }

    Fixed<N, K> random01();
};

template <int N, int K, bool is_fast, typename IntType, typename T>
static constexpr T from_raw(IntType x) {
    if (is_fast) {
        FastFixed<N, K> ret;
        ret.v = x;
        return ret;
    } else {
        Fixed<N, K> ret;
        ret.v = x;
        return ret;
    }
};



template <int N, int K>
Fixed<N, K> Fixed<N, K>::random01() {
    //return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>((rnd() & ((((int128_t) 1) << K) - 1)));
    if (!optimise) {
        return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>((rnd() & ((((int128_t) 1) << K) - 1)));
    } else {
        while (RandQueue.empty());
        return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>((RandQueue.pop() & mask));
    }
};

template <typename Fixed_or_Float> 
Fixed_or_Float random01 () {
    if constexpr (std::is_floating_point<Fixed_or_Float>::value) {
        return Fixed_or_Float(rnd()/double(rnd.max()));
    } else {
        return Fixed_or_Float().random01();
    }
}



template <int N, int K>
Fixed<N, K> operator+(Fixed<N, K> a, Fixed<N, K> b) {
    return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>(a.v + b.v);
};

template <int N, int K, typename T>
Fixed<N, K> operator+(Fixed<N, K> a, T b) {
    return a + Fixed<N, K>(b);
};

template <int N, int K>
Fixed<N, K> operator-(Fixed<N, K> a, Fixed<N, K> b) {
    return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>(a.v - b.v);
};

template <int N, int K, typename T>
Fixed<N, K> operator-(Fixed<N, K> a, T b) {
    return a - Fixed<N, K>(b);
};

template <int N, int K>
Fixed<N, K> operator*(Fixed<N, K> a, Fixed<N, K> b) {
    return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>(((int128_t) a.v * b.v) >> K);
};

template <int N, int K, typename T>
Fixed<N, K> operator*(Fixed<N, K> a, T b) {
    return a * Fixed<N, K>(b);
};

template <int N, int K>
Fixed<N, K> operator/(Fixed<N, K> a, Fixed<N, K> b) {
    return from_raw<N, K, false, typename Fixed<N, K>::IntType, Fixed<N, K>>(((int128_t) a.v << K) / b.v);
};

template <int N, int K, typename T>
Fixed<N, K> operator/(Fixed<N, K> a, T b) {
    return a / Fixed<N, K>(b);
};

template <int N, int K>
Fixed<N, K> &operator+=(Fixed<N, K> &a, Fixed<N, K> b) {
    return a = a + b;
};

template <int N, int K, typename T>
Fixed<N, K> &operator+=(Fixed<N, K> &a, T b) {
    return a = a + b;
};


template <int N, int K>
Fixed<N, K> &operator-=(Fixed<N, K> &a, Fixed<N, K> b) {
    return a = a - b;
};

template <int N, int K, typename T>
Fixed<N, K> &operator-=(Fixed<N, K> &a, T b) {
    return a = a - b;
};

template <int N, int K>
Fixed<N, K> &operator*=(Fixed<N, K> &a, Fixed<N, K> b) {
    return a = a * b;
};

template <int N, int K, typename T>
Fixed<N, K> &operator*=(Fixed<N, K> &a, T b) {
    return a = a * b;
};

template <int N, int K>
Fixed<N, K> &operator/=(Fixed<N, K> &a, Fixed<N, K> b) {
    return a = a / b;
};

template <int N, int K, typename T>
Fixed<N, K> &operator/=(Fixed<N, K> &a, T b) {
    return a = a / b;
};

template <int N, int K>
Fixed<N, K> operator-(Fixed<N, K> x) {
    return from_raw<N, K, false>(-x.v);
};

template <int N, int K>
Fixed<N, K> abs(Fixed<N, K> x) {
    if (x.v < 0) {
        x.v = -x.v;
    }
    return x;
};

template <int N, int K>
ostream &operator<<(ostream &out, Fixed<N, K> x)  {
    return out << double(x);
};

template <int N, int K>
istream &operator>>(istream &in, Fixed<N, K> &x)  {
    double a;
    in >> a;
    x = Fixed<N, K>(a);
    return in;
}; 

constexpr static std::array<pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
//extern std::array<pair<int, int>, 4> deltas;
// extern static constexpr Fixed inf;

//extern static constexpr Fixed eps;



#endif
