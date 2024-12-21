#ifndef LIQUIDSIM_H
#define LIQUIDSIM_H
#include "../Fixed/Fixed.h"
#include "../VectorField/VectorField.h"
#include <random>
#include <algorithm>
#include <ranges>
#include <cassert>
#include <fstream>
#include <cstring>
#include <thread>
#include <execution>
// constexpr size_t N = 14, M = 5;


template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
class LiquidSim {
    public:
    // static constexpr size_t N = 36, M = 84;
    // static constexpr size_t T = 1'000'000;
    template <typename A>
    class VectorField {
        public:
        array<A, deltas.size()> v[N][M];
        A &add(int x, int y, int dx, int dy, A dv) {
            return get(x, y, dx, dy) += dv;
        };
        A &get(int x, int y, int dx, int dy){
            size_t i = ranges::find(deltas, pair(dx, dy)) - deltas.begin();
            //assert(i < deltas.size());
            return v[x][y][i];
        };

        void print() {
        }
    };
    void start();

    P rho[256];
    P g;
    P p[N][M]{}, old_p[N][M];

    VectorField<V> velocity{}; 
    VectorField<VF> velocity_flow{};

    int last_use[N][M]{};
    int UT = 0;
    P cur_p;
    char type;
    int dirs[N][M]{};

    LiquidSim (std::vector<vector<char>> field_in = {}, P g_in = 0.1, P* rho_in = nullptr) {
        cerr << "Entered LiquidSim()\n";
        cerr << "copying rho: ";

        if (field_in != vector<vector<char>>{}) {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    field[i][j] = field_in[i][j];
                }
            }
        }

        if (rho_in == nullptr) {
            P rho_z [256] = {0};
            rho_in = rho_z;
            //cerr << "filled the rho with 0 ";
        }
        for (int i = 0; i < 256; i++) {
            rho[i] = rho_in[i];
            cerr << i << " ";
        }
    
        cerr << "\n";
        //std::copy(rho_in, rho_in + 256, this->rho);
        cerr << "Rho copied\n";
        this->g = g_in;
    };


    tuple<V, bool, pair<int, int>> propagate_flow(int x, int y, V lim);
    void propagate_stop(int x, int y, bool force = false);
    V move_prob(int x, int y);

    array<V, deltas.size()> v;
    void swap_with(int x, int y);

    bool propagate_move(int x, int y, bool is_first);

    void simulate();

    void save(string name);
    
    char field[N][M + 1] = {
    "####################################################################################",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                       .........                                  #",
    "#..............#            #           .........                                  #",
    "#..............#            #           .........                                  #",
    "#..............#            #           .........                                  #",
    "#..............#            #                                                      #",
    "#..............#            #                                                      #",
    "#..............#            #                                                      #",
    "#..............#            #                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............#                                                      #",
    "#..............#............################                     #                 #",
    "#...........................#....................................#                 #",
    "#...........................#....................................#                 #",
    "#...........................#....................................#                 #",
    "##################################################################                 #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "#                                                                                  #",
    "####################################################################################",
    };

};



template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
tuple<V, bool, pair<int, int>> LiquidSim<N, M, T, P, V, VF>::propagate_flow(int x, int y, V lim) {
    last_use[x][y] = UT - 1;
    V ret = 0;
    for (auto [dx, dy] : deltas) {
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
            V cap = velocity.get(x, y, dx, dy);
            V flow = velocity_flow.get(x, y, dx, dy);
            if (flow == cap) {
                continue;
            }
            // assert(v >= velocity_flow.get(x, y, dx, dy));
            V vp = min(lim, cap - flow);
            if (last_use[nx][ny] == UT - 1) {
                velocity_flow.add(x, y, dx, dy, vp);
                last_use[x][y] = UT;
                // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                return {vp, 1, {nx, ny}};
            }
            auto [t, prop, end] = propagate_flow(nx, ny, vp);
            ret += t;
            if (prop) {
                velocity_flow.add(x, y, dx, dy, t);
                last_use[x][y] = UT;
                // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                return {t, prop && end != pair(x, y), end};
            }
        }
    }
    last_use[x][y] = UT;
    return {ret, 0, {0, 0}};
}

template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
void LiquidSim<N, M, T, P, V, VF>::propagate_stop(int x, int y, bool force) {
    if (!force) {
        bool stop = true;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > 0) {
                stop = false;
                break;
            }
        }
        if (!stop) {
            return;
        }
    }
    last_use[x][y] = UT;
    for (auto [dx, dy] : deltas) {
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > 0) {
            continue;
        }
        propagate_stop(nx, ny);
    }
}


template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
V LiquidSim<N, M, T, P, V, VF>::move_prob(int x, int y) {
    V sum = 0;
    for (size_t i = 0; i < deltas.size(); ++i) {
        auto [dx, dy] = deltas[i];
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
            continue;
        }
        V v = velocity.get(x, y, dx, dy);
        if (v < 0) {
            continue;
        }
        sum += v;
    }
    return sum;
}

template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
void LiquidSim<N, M, T, P, V, VF>::swap_with(int x, int y) {
    swap(field[x][y], this->type);
    swap(this->p[x][y], this->cur_p);
    swap(this->velocity.v[x][y], v);
}

template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
bool LiquidSim<N, M, T, P, V, VF>::propagate_move(int x, int y, bool is_first) {
    last_use[x][y] = UT - is_first;
    bool ret = false;
    int nx = -1, ny = -1;
    do {
        std::array<V, deltas.size()> tres;
        V sum = 0;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
                tres[i] = sum;
                continue;
            }
            V v = velocity.get(x, y, dx, dy);
            if (v < 0) {
                tres[i] = sum;
                continue;
            }
            sum += v;
            tres[i] = sum;
        }

        if (sum == 0) {
            break;
        }

        P p = random01<P>() * sum;
        size_t d = std::ranges::upper_bound(tres, V(p)) - tres.begin();

        auto [dx, dy] = deltas[d];
        nx = x + dx;
        ny = y + dy;
        assert(velocity.get(x, y, dx, dy) > 0 && field[nx][ny] != '#' && last_use[nx][ny] < UT);

        ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
    } while (!ret);
    last_use[x][y] = UT;
    for (size_t i = 0; i < deltas.size(); ++i) {
        auto [dx, dy] = deltas[i];
        int nx = x + dx, ny = y + dy;
        if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < 0) {
            propagate_stop(nx, ny);
        }
    }
    if (ret) {
        if (!is_first) {
            swap_with(x, y);
            swap_with(nx, ny);
            swap_with(x, y);
        }
    }
    return ret;
}


template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
void LiquidSim<N, M, T, P, V, VF>::simulate () {
    cerr << "Created\n";
    for (size_t x = 0; x < N; ++x) {
        for (size_t y = 0; y < M; ++y) {
            if (field[x][y] == '#')
                continue;
            for (auto [dx, dy] : deltas) {
                this->dirs[x][y] += (field[x + dx][y + dy] != '#');
            }
        }
    }

    for (size_t i = 0; i < T; ++i) {
        P total_delta_p = 0;
        // Apply external forces
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < M; ++y) {
                if (field[x][y] == '#')
                    continue;
                if (field[x + 1][y] != '#')
                    this->velocity.add(x, y, 1, 0, this->g);
            }
        }
        // Apply forces from p
        memcpy(this->old_p, this->p, sizeof(this->p));
        for (size_t x = 0; x < N; ++x) {
            std::for_each(std::execution::par_unseq, std::begin(this->p[x]), std::end(this->p[x]), [&](auto& py){
                size_t y = &py - &this->p[x][0];
                if (field[x][y] == '#')
                    return;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && this->old_p[nx][ny] < this->old_p[x][y]) {
                        auto delta_p = this->old_p[x][y] - this->old_p[nx][ny];
                        auto force = delta_p;
                        auto &contr = this->velocity.get(nx, ny, -dx, -dy);
                        if (contr * this->rho[(int) field[nx][ny]] >= force) {
                            contr -= force / this->rho[(int) field[nx][ny]];
                            continue;
                        }
                        force -= contr * this->rho[(int) field[nx][ny]];
                        contr = 0;
                        this->velocity.add(x, y, dx, dy, force / this->rho[(int) field[x][y]]);
                        this->p[x][y] -= force / this->dirs[x][y];
                        total_delta_p -= force / this->dirs[x][y];
                    }
                }
            });
        }
        // Make flow from velocities
        this->velocity_flow = {};
        bool prop = false;
        do {
            this->UT += 2;
            prop = 0;
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] != '#' && this->last_use[x][y] != this->UT) {
                        auto [t, local_prop, _] = this->propagate_flow(x, y, 1);
                        if (t > 0) {
                            prop = 1;
                        }
                    }
                }
            }
        } while (prop);
        // Recalculate p with kinetic energy
        for (size_t x = 0; x < N; ++x) {
            std::for_each(std::execution::par_unseq, std::begin(this->p[x]), std::end(this->p[x]), [&](auto& py){
                size_t y = &py - &this->p[x][0];
                if (field[x][y] == '#')
                    return;
                for (auto [dx, dy] : deltas) {
                    auto old_v = this->velocity.get(x, y, dx, dy);
                    auto new_v = this->velocity_flow.get(x, y, dx, dy);
                    if (old_v > 0) {
                        //assert(new_v <= old_v);
                        this->velocity.get(x, y, dx, dy) = new_v;
                        auto force = (old_v - new_v) * this->rho[(int) field[x][y]];
                        if (field[x][y] == '.')
                            force *= 0.8;
                        if (field[x + dx][y + dy] == '#') {
                            this->p[x][y] += force / this->dirs[x][y];
                            total_delta_p += force / this->dirs[x][y];
                        } else {
                            this->p[x + dx][y + dy] += force / this->dirs[x + dx][y + dy];
                            total_delta_p += force / this->dirs[x + dx][y + dy];
                        }
                    }
                }
            });
        }
        this->UT += 2;
        prop = false;
        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < M; ++y) {
                if (field[x][y] != '#' && this->last_use[x][y] != this->UT) {
                    if (random01<P>() < this->move_prob(x, y)) {
                        prop = true;
                        this->propagate_move(x, y, true);
                    } else {
                        this->propagate_stop(x, y, true);
                    }
                }
            }
        }
        if (prop || 1) {
            cout << "Tick " << i << ":\n";
            for (size_t x = 0; x < N; ++x) {
                cout << field[x] << "\n";
                
                if (i % 30 == 0) {
                    this->save("Test_" + to_string(i));
                }
            }
        }
        //exit(0);
    }
}

template <typename Fixed_or_Float> 
std::string get_type_string (Fixed_or_Float) {
    if constexpr (std::is_floating_point<Fixed_or_Float>::value) {
        return "DOUBLE";
    } else {
        return Fixed_or_Float().type_str();
    }
}

template <typename Fixed_or_Float> 
std::string get_value_string (Fixed_or_Float a) {
    if constexpr (std::is_floating_point<Fixed_or_Float>::value) {
        return to_string(a);
    } else {
        return to_string(a.v);
    }
}

template <size_t N, size_t M, size_t T, typename P, typename V, typename VF>
void LiquidSim<N, M, T, P, V, VF>::save(std::string name) {
    std::ofstream file("./Saves/" + name + ".txt");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    file << N << " " << M << "\n\n"; 

    // file << get_type_string(P()) << "\n";
    // file << get_type_string(V()) << "\n";
    // file << get_type_string(VF()) << "\n\n";

    for (int i = 0; i < 256; i++) {
        file << rho[i] << " ";
    }
    file << "\n\n";

    file << g << "\n";

    file << "\n";
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            file << p[i][j] << " ";
        }
        file << "\n";
    }

    file << "\n";
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            file << old_p[i][j] << " ";
        }
        file << "\n";
    }

    file << "\n";
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < deltas.size(); k++){ 
                file << velocity.v[i][j][k] << " ";
            }
            file << "\t";
        }
        file << "\n";
    }
    file << "\n";
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < deltas.size(); k++){ 
                file << velocity_flow.v[i][j][k] << " ";
            }
            file << "\t";
        }
        file << "\n";
    }
    file << "\n";
    
    // int last_use[N][M]{};
    // int UT = 0;
    // P cur_p;
    // char type;
    // int dirs[N][M]{};

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            file << last_use[i][j] << " ";
        }
        file << "\n";
    }
    file << "\n";
    file << UT << "\n";
    file << cur_p << "\n";
    file << (int) type << "\n";
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            file << dirs[i][j] << " ";
        }
        file << "\n";
    }
    file << "\n";
    
    for (size_t x = 0; x < N; ++x) {
        for (size_t y = 0; y < M; ++y) {
            file << (int) field[x][y] << " ";
        }
        file << "\n";
    }
    //file << "\n";
    file.close();
}


// template < size_t N, size_t M, size_t T, typename P, typename V, typename VF>
// LiquidSim<N, M, T, P, V, VF> load(std::string name) {
//     std::ifstream file("./Saves/" + name + ".txt");
//     if (!file.is_open()) {
//         throw std::runtime_error("Could not open file");
//     }
    
//     int N1, M1;
//     file >> N1;
//     file >> M1; 
//     LiquidSim<N, M, T, P, V, VF> ret = LiquidSim<N, M, T, P, V, VF>();
    


//     for (int i = 0; i < 256; i++) {
//         file >> ret.rho[i];
//     }

//     file >> ret.g;

//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < M; j++) {
//             file >> ret.p[i][j];
//         }
//     }
    
//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < M; j++) {
//             file >> ret.old_p[i][j];
//         }
//     }

//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < M; j++) {
//             for (int k = 0; k < deltas.size(); k++){ 
//                 file >> (ret.velocity.v[i][j][k]);
//             }
//         }
//     }

//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < M; j++) {
//             for (int k = 0; k < deltas.size(); k++){ 
//                 file >> (ret.velocity_flow.v[i][j][k]);
//             }
//         }
//     }
    
//     // int last_use[N][M]{};
//     // int UT = 0;
//     // P cur_p;
//     // char type;
//     // int dirs[N][M]{};

//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < M; j++) {
//             file >> ret.last_use[i][j];
//         }
//     }
//     file >> ret.UT;
//     file >> ret.cur_p;
//     int type;
//     file >> type;
//     ret.type = (char) type;
//     for (int i = 0; i < N; i++) {
//         for (int j = 0; j < M; j++) {
//             file >> ret.dirs[i][j];
//         }
//     }
    
//     int n;
//     for (size_t x = 0; x < N; ++x) {
//         for (size_t y = 0; y < M; ++y) {
//             file >> n;
//             ret.field[x][y] = (char) n;
//         }
//     }

//     file.close();
//     return ret;
// }



#endif