#ifndef LIQUIDSIMDYNAMIC_H
#define LIQUIDSIMDYNAMIC_H
#include "../Fixed/Fixed.h"
#include <random>
#include <algorithm>
#include <ranges>
#include <cassert>
#include <fstream>
#include <cstring>
#include <thread>
#include <execution>
// constexpr size_t N = 14, M = 5;
#include "LiquidSim.h"


template <size_t T, typename P, typename V, typename VF>
class LiquidSimDynamic {
    public:
    string name = "Simulator_№" + to_string(((unsigned int) (rnd() * 100000)));
    // static constexpr size_t N = 36, M = 84;
    // static constexpr size_t T = 1'000'000;
    size_t N;
    size_t M;
    template <typename A>
    class VectorField {
        public:
        std::vector<std::vector<array<A, deltas.size()>>> v;
        VectorField(size_t N, size_t M) {
            v.resize(N);
            for (int i = 0; i < N; i++) {
                v[i].resize(M);
            }
        };
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
    std::vector<std::vector<P>> p;
    std::vector<std::vector<P>> old_p;
    VectorField<V> velocity; 
    VectorField<VF> velocity_flow;
    vector<vector<char>> field;
    vector<vector<int>> last_use;
    int UT = 0;
    P cur_p;
    char type;
    vector<vector<int>> dirs;
   
    LiquidSimDynamic (size_t N_in,\
                      size_t M_in,\
                      std::vector<vector<char>> field_in, P g_in = 0.1,\
                      P* rho_in = nullptr): velocity(N_in, M_in), velocity_flow(N_in, M_in) {
        N = N_in;
        M = M_in;
        //p.resize(N, std::vector<P>(M));
        p.resize(N);
        old_p.resize(N);
        last_use.resize(N);
        dirs.resize(N);
        field.resize(N);
        for (int i = 0; i < N; i++) {
            p[i].resize(M);
            old_p[i].resize(M);
            last_use[i].resize(M);
            dirs[i].resize(M);
            field[i].resize(M + 1);
        }
        

        if (rho_in == nullptr) {
            P rho_z [256] = {0};
            rho_z ['.'] = 1000;
            rho_z [' '] = 0.01;
            rho_in = rho_z;
            //cerr << "filled the rho with 0 ";
        }
        for (int i = 0; i < 256; i++) {
            rho[i] = rho_in[i];
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M + 1; j++) {
                field[i][j] = field_in[i][j];
            }
        }

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
    
   
};



template <size_t T, typename P, typename V, typename VF>
tuple<V, bool, pair<int, int>> LiquidSimDynamic<T, P, V, VF>::propagate_flow(int x, int y, V lim) {
    last_use[x][y] = UT - 1;
    V ret = 0;
    for (auto [dx, dy] : deltas) {
        int nx = x + dx, ny = y + dy;
        if (nx >= N || ny >= M || nx < 0 || ny < 0) continue;
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

template <size_t T, typename P, typename V, typename VF>
void LiquidSimDynamic<T, P, V, VF>::propagate_stop(int x, int y, bool force) {
    if (!force) {
        bool stop = true;
        for (auto [dx, dy] : deltas) {
            int nx = x + dx, ny = y + dy;
            if (nx >= N || ny >= M || nx < 0 || ny < 0) continue;
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
        if (nx >= N || ny >= M || nx < 0 || ny < 0) continue;
        if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > 0) {
            continue;
        }
        propagate_stop(nx, ny);
    }
}


template <size_t T, typename P, typename V, typename VF>
V LiquidSimDynamic<T, P, V, VF>::move_prob(int x, int y) {
    V sum = 0;
    for (size_t i = 0; i < deltas.size(); ++i) {
        auto [dx, dy] = deltas[i];
        int nx = x + dx, ny = y + dy;
        if (nx >= N || ny >= M || nx < 0 || ny < 0) continue;
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

template <size_t T, typename P, typename V, typename VF>
void LiquidSimDynamic<T, P, V, VF>::swap_with(int x, int y) {
    //swap(field[x][y], this->type);
    auto tmp1 = field[x][y];
    field[x][y] = this->type;
    this->type = tmp1;
    //swap(this->p[x][y], this->cur_p);
    auto tmp2 = this->p[x][y];
    this->p[x][y] = this->cur_p;
    this->cur_p = tmp2;
    //swap(this->velocity.v[x][y], v);
    auto tmp3 = this->velocity.v[x][y];
    this->velocity.v[x][y] = v;
    v = tmp3;
}

template <size_t T, typename P, typename V, typename VF>
bool LiquidSimDynamic<T, P, V, VF>::propagate_move(int x, int y, bool is_first) {
    last_use[x][y] = UT - is_first;
    bool ret = false;
    int nx = -1, ny = -1;
    do {
        std::array<V, deltas.size()> tres;
        V sum = 0;
        for (size_t i = 0; i < deltas.size(); ++i) {
            auto [dx, dy] = deltas[i];
            int nx = x + dx, ny = y + dy;
            if (nx >= N || ny >= M || nx < 0 || ny < 0) continue;
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


template <size_t T, typename P, typename V, typename VF>
void LiquidSimDynamic<T, P, V, VF>::simulate () {
    //cerr << "Created\n" << N << " " << M << " " << field.size() << " " << field.back().size() << " " << dirs.size() << " " << dirs.back().size() << "\n";
    for (size_t x = 0; x < N; x++) {
        for (size_t y = 0; y < M; y++) {
            if (field[x][y] == '#')
                continue;
                for (auto [dx, dy] : deltas) {
                    if (x + dx >= N || y + dy >= M + 1 || x + dx < 0 || y + dy < 0) continue;
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
                if (x + 1 >= N) continue;
                if (field[x + 1][y] != '#')
                    this->velocity.add(x, y, 1, 0, this->g);
            }
        }
        // Apply forces from p
        //memcpy(this->old_p, this->p, sizeof(this->p));
        for (int i= 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                this->old_p[i][j] = this->p[i][j];
            }
        }


        for (size_t x = 0; x < N; ++x) {
            for (size_t y = 0; y < M; ++y) {
                if (field[x][y] == '#')
                    continue;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (nx < 0 || nx >= N || ny < 0 || ny >= M + 1) continue;
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
            };
        }
        // Make flow from velocities
        this->velocity_flow = VectorField<VF>(N, M);
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
            for (size_t y = 0; y < M; ++y) {
                if (field[x][y] == '#')
                    continue;;
                for (auto [dx, dy] : deltas) {
                    if (x >= N || y + dy >= M || x + dx < 0 || y + dy < 0) continue;
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
            };
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
                for (size_t y = 0; y < M; ++y) {
                    cout << field[x][y];
                }
                cout << "\n";
            }
            if (i % 100 == 0) {
                this->save(this->name + "_Tick_№" + to_string(i));
            }
        }
        //exit(0);
    }
}



template <size_t T, typename P, typename V, typename VF>
void LiquidSimDynamic<T, P, V, VF>::save(std::string name) {
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
    // cerr << "save" << "\n";
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            //cerr << i << " " << j << "\t";
            file << p[i][j] << " ";
        }
        //cerr << "\n";
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
    
    //cerr << N << " " << M << " " << field.size() << " " << field.back().size() << "\n"; 
    for (size_t x = 0; x < N; ++x) {
        for (size_t y = 0; y < M; ++y) {
            file << (int) field[x][y] << " ";
        }
        file << "\n";
    }
    //file << "\n";
    file.close();
}


template <size_t T, typename P, typename V, typename VF>
LiquidSimDynamic<T, P, V, VF> load(std::string name) {
    std::ifstream file("./Saves/" + name + ".txt");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    
    size_t N1, M1;
    file >> N1;
    file >> M1; 
    std::vector<vector<char>> field(N1, std::vector<char>(M1 + 1));
    LiquidSimDynamic<T, P, V, VF> ret = LiquidSimDynamic<T, P, V, VF>(N1, M1, field);
    


    for (int i = 0; i < 256; i++) {
        file >> ret.rho[i];
    }

    file >> ret.g;

    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < M1; j++) {
            file >> ret.p[i][j];
        }
    }
    
    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < M1; j++) {
            file >> ret.old_p[i][j];
        }
    }

    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < M1; j++) {
            for (int k = 0; k < deltas.size(); k++){ 
                file >> (ret.velocity.v[i][j][k]);
            }
        }
    }

    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < M1; j++) {
            for (int k = 0; k < deltas.size(); k++){ 
                file >> (ret.velocity_flow.v[i][j][k]);
            }
        }
    }
    
    // int last_use[N][M]{};
    // int UT = 0;
    // P cur_p;
    // char type;
    // int dirs[N][M]{};

    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < M1; j++) {
            file >> ret.last_use[i][j];
        }
    }
    file >> ret.UT;
    file >> ret.cur_p;
    int type;
    file >> type;
    ret.type = (char) type;
    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < M1; j++) {
            file >> ret.dirs[i][j];
        }
    }
    
    int n;
    for (size_t x = 0; x < N1; x++) {
        for (size_t y = 0; y < M1; y++) {
            file >> n;
            ret.field[x][y] = (char) n;
        }
    }

    file.close();
    return ret;
}



#endif
