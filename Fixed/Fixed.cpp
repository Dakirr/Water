#include "Fixed.h"
#include <fstream>
int optimise = 0;

// size_t cur_r = 0;
// bool x = false;
// std::ifstream file_random("Random.txt");


// void rnd_init() {
//     if (!x) {
//         if (!(file_random.is_open())) {
//             throw std::runtime_error("Could not open file");
//         }
//     }
//     x = true;
// }

// std::ofstream file("Random.txt");
    // if (!file.is_open()) {
    //     throw std::runtime_error("Could not open file");
    // }
    // for (size_t i = 0; i <= 100000000; i++) {
    //     file << double(rnd())/rnd.max() << " ";
    // }
    // file.close();
    // exit(0);

// constexpr Fixed from_raw(int32_t x) {
//     Fixed ret;
//     ret.v = x;
//     return ret;
// } 


// bool Fixed::operator==(const Fixed &) const = default;

// Fixed operator+(Fixed a, Fixed b) {
//     return from_raw(a.v + b.v);
// }

// Fixed operator-(Fixed a, Fixed b) {
//     return from_raw(a.v - b.v);
// }

// Fixed operator*(Fixed a, Fixed b) {
//     return from_raw(((int64_t) a.v * b.v) >> 16);
// }

// Fixed operator/(Fixed a, Fixed b) {
//     return from_raw(((int64_t) a.v << 16) / b.v);
// }

// Fixed &operator+=(Fixed &a, Fixed b) {
//     return a = a + b;
// }

// Fixed &operator-=(Fixed &a, Fixed b) {
//     return a = a - b;
// }

// Fixed &operator*=(Fixed &a, Fixed b) {
//     return a = a * b;
// }

// Fixed &operator/=(Fixed &a, Fixed b) {
//     return a = a / b;
// }

// Fixed operator-(Fixed x) {
//     return from_raw(-x.v);
// }

// Fixed abs(Fixed x) {
//     if (x.v < 0) {
//         x.v = -x.v;
//     }
//     return x;
// }

// ostream &operator<<(ostream &out, Fixed x) {
//     return out << x.v / (double) (1 << 16);
// }


// Fixed random01() {
//     return from_raw((rnd() & ((1 << 16) - 1)));
//}

static constexpr Fixed<64, 8> inf = from_raw<64, 8, false, int64_t, Fixed<64, 8>>(std::numeric_limits<int64_t>::max());
static constexpr Fixed<64, 8> eps = from_raw<64, 8, false, int64_t, Fixed<64, 8>>(4);
