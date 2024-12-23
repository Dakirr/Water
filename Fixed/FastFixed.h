#ifndef FAST_FIXED_H
#define FAST_FIXED_H
#include <cstdint>
#include <limits>
#include <array>
#include <iostream>
#include <utility>
#include <random>
#include "../LiquidSim/Thread.h"
extern int optimise;


using namespace std;
extern mt19937 rnd;
extern std::ifstream file_random;
extern size_t cur_r;


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

template <int N, int K> class Fixed;

template <int N, int K, bool is_fast, typename IntType, typename T>
static constexpr T from_raw(IntType x);

template <int N, int K>
class FastFixed {
    public:
    static constexpr std::size_t kNValue = N;
    static constexpr std::size_t kKValue = K;
    static constexpr bool kFast = true;
    static constexpr int128_t mask =  ((((int128_t) 1) << K) - 1); 
    using IntType = 
    std::conditional_t<
        (N <= 8), int_fast8_t,
        std::conditional_t<
            (N <= 16), int_fast16_t,
            std::conditional_t<
                (N <= 32), int_fast32_t,
                std::conditional_t<
                    (N <= 64), int_fast64_t,
                    int_fast64_t     
                >
            >
        >
    >;
    
    IntType v;
    constexpr FastFixed(int v): v(((int128_t) v) << K) {}
    constexpr FastFixed(float f): v(f * (((int128_t) 1) << K)) {}
    constexpr FastFixed(double f): v(f * (((int128_t) 1) << K)) {}
    
    template <int N2, int K2>
    constexpr FastFixed(FastFixed<N2, K2> other) {
        this->v = (((int128_t) other.v) << K) >> K2; 
    }

    template <int N2, int K2>
    constexpr FastFixed(Fixed<N2, K2> other) {
        this->v = (((int128_t) other.v) << K) >> K2; 
    }


    constexpr FastFixed(): v(0) {}

    // template <typename T>
    // std::strong_ordering operator<=>(const T& other) const {
    //     return v <=> FastFixed<N, K>(other);
    // };

    // template <int N2, int K2>
    // std::strong_ordering operator<=>(const FastFixed<N2, K2>& other) const {
    //     return v <=> other.v;
    // };

    bool operator>(const FastFixed<N, K> &other) const {
        return v > other.v;
    };

    template <typename T>
    bool operator>(const T& other) const {
        return v > FastFixed<N, K>(other).v;
    };

    bool operator<(const FastFixed<N, K> &other) const {
        return v < other.v;
    };

    template <typename T>
    bool operator<(const T& other) const {
        return v < FastFixed<N, K>(other).v;
    };

    bool operator>=(const FastFixed<N, K> &other) const {
        return v >= other.v;
    };

    template <typename T>
    bool operator>=(const T& other) const {
        return v >= FastFixed<N, K>(other).v;
    };

    bool operator<=(const FastFixed<N, K> &other) const {
        return v <= other.v;
    };

    template <typename T>
    bool operator<=(const T& other) const {
        return v <= FastFixed<N, K>(other).v;
    };

    bool operator!=(const FastFixed<N, K> &other) const {
        return v != other.v;
    };

    template <typename T>
    bool operator!=(const T& other) const {
        return v != FastFixed<N, K>(other).v;
    };

    bool operator==(const FastFixed<N, K>& other) const {
        return v == other.v;
    };


    template <typename T>
    bool operator==(const T& other) const {
        return v == FastFixed<N, K>(other).v;
    };

    std::string type_str() {
        return "FAST_FIXED(" + std::to_string(N) + "," + std::to_string(K) + ")"; 
    };

    constexpr operator float() const {
        return (float)v / (((int128_t) 1) << K);
    }

    constexpr explicit operator double() const {
        return (double)v / (((int128_t) 1) << K);
    }


    FastFixed<N, K> random01();
};

template <int N, int K>
FastFixed<N, K> FastFixed<N, K>::random01() {
    //return from_raw<N, K, true, typename FastFixed<N, K>::IntType, FastFixed<N, K>>((rnd() & ((((int128_t) 1) << K) - 1)));
    if (!optimise) {
        return from_raw<N, K, false, typename FastFixed<N, K>::IntType, FastFixed<N, K>>((rnd() & ((((int128_t) 1) << K) - 1)));
    } else {
        while (RandQueue.empty());
        return from_raw<N, K, false, typename FastFixed<N, K>::IntType, FastFixed<N, K>>((RandQueue.pop() & mask));
    }
};

template <int N, int K>
FastFixed<N, K> operator+(FastFixed<N, K> a, FastFixed<N, K> b) {
    return from_raw<N, K, true, typename FastFixed<N, K>::IntType, FastFixed<N, K>>(a.v + b.v);
};

template <int N, int K, typename T>
FastFixed<N, K> operator+(FastFixed<N, K> a, T b) {
    return a + FastFixed<N, K>(b);
};

template <int N, int K>
FastFixed<N, K> operator-(FastFixed<N, K> a, FastFixed<N, K> b) {
    return from_raw<N, K, true, typename FastFixed<N, K>::IntType, FastFixed<N, K>>(a.v - b.v);
};

template <int N, int K, typename T>
FastFixed<N, K> operator-(FastFixed<N, K> a, T b) {
    return a - FastFixed<N, K>(b);
};

template <int N, int K>
FastFixed<N, K> operator*(FastFixed<N, K> a, FastFixed<N, K> b) {
    return from_raw<N, K, true, typename FastFixed<N, K>::IntType, FastFixed<N, K>>(((int128_t) a.v * b.v) >> K);
};

template <int N, int K, typename T>
FastFixed<N, K> operator*(FastFixed<N, K> a, T b) {
    return a * FastFixed<N, K>(b);
};

template <int N, int K>
FastFixed<N, K> operator/(FastFixed<N, K> a, FastFixed<N, K> b) {
    return from_raw<N, K, true, typename FastFixed<N, K>::IntType, FastFixed<N, K>>(((int128_t) a.v << K) / b.v);
};

template <int N, int K, typename T>
FastFixed<N, K> operator/(FastFixed<N, K> a, T b) {
    return a / FastFixed<N, K>(b);
};

template <int N, int K>
FastFixed<N, K> &operator+=(FastFixed<N, K> &a, FastFixed<N, K> b) {
    return a = a + b;
};

template <int N, int K, typename T>
FastFixed<N, K> &operator+=(FastFixed<N, K> &a, T b) {
    return a = a + b;
};


template <int N, int K>
FastFixed<N, K> &operator-=(FastFixed<N, K> &a, FastFixed<N, K> b) {
    return a = a - b;
};

template <int N, int K, typename T>
FastFixed<N, K> &operator-=(FastFixed<N, K> &a, T b) {
    return a = a - b;
};

template <int N, int K>
FastFixed<N, K> &operator*=(FastFixed<N, K> &a, FastFixed<N, K> b) {
    return a = a * b;
};

template <int N, int K, typename T>
FastFixed<N, K> &operator*=(FastFixed<N, K> &a, T b) {
    return a = a * b;
};

template <int N, int K>
FastFixed<N, K> &operator/=(FastFixed<N, K> &a, FastFixed<N, K> b) {
    return a = a / b;
};

template <int N, int K, typename T>
FastFixed<N, K> &operator/=(FastFixed<N, K> &a, T b) {
    return a = a / b;
};

template <int N, int K>
FastFixed<N, K> operator-(FastFixed<N, K> x) {
    return from_raw<N, K, true>(-x.v);
};

template <int N, int K>
FastFixed<N, K> abs(FastFixed<N, K> x) {
    if (x.v < 0) {
        x.v = -x.v;
    }
    return x;
};

template <int N, int K>
ostream &operator<<(ostream &out, FastFixed<N, K> x)  {
    return out << double(x);
};

template <int N, int K>
istream &operator>>(istream &in, FastFixed<N, K> x)  {
    double a;
    in >> a;
    x = Fixed<N, K>(a);
    return in;
};


//constexpr static std::array<pair<int, int>, 4> deltas{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
//extern std::array<pair<int, int>, 4> deltas;
// extern static constexpr FastFixed inf;

//extern static constexpr FastFixed eps;



#endif
