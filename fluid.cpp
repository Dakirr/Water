//include <bits/stdc++.h>
#include "./LiquidSim/LiquidSim.h"
#include "./LiquidSim/LiquidSimDynamic.h"
#include <cstring>
#include <random>
#include <thread>
#include <execution>
#include "./Parcer/simulator.hpp"
#include "./LiquidSim/ThreadPool.h"


#ifndef TYPES
#error "TYPES is not defined"
#endif

#ifdef SIZES

using LST = types::Simulator<types::TypesList<TYPES>, SIZES>;

#else

using LST = types::Simulator<types::TypesList<TYPES>>;

#endif

int main() {   
    sum_vec();
    // Вариант 1. Про сами симуляторы и сохранение (они работают нормально, вроде.)
    using P = Fixed<32, 8>;
    using V = FastFixed<64, 8>;
    using VF = Fixed<16, 8>;
    
    LiquidSim<36, 84, 1000000, P, V, VF> LS = LiquidSim<36, 84, 1000000, P, V, VF>();
    LiquidSimDynamic<1000000, P, V, VF> LSD = load<1000000, P, V, VF>("Test_5");
    LSD.save("Test_5_new");
    LSD.simulate();   

    // Вариант 2. Создание из флагов (работает, кажется, но очень странно. Я не знаю, это моя проблема, или прикол физики. Оно компилируется и что-то выводит, во всяком случае.)
    // Код непосредственно обработки флагов написан не мной, но туда впаян мой симулятор, и я, кажется, понимаю, что происходит.
    
    const LST sim = LST::from_params(types::SimulationParams{
        .p_type_name      = "FLOAT",
        .v_type_name      = "FIXED(32,  5)",
        .v_flow_type_name = "DOUBLE",
    });

    sim.start_on_field(types::Context{
        .field =
            std::vector<std::vector<char>>{
                {'#', '#', '#', '#', '#'},
                {'#', '#', '.', '.', '#'},
                {'#', ' ', ' ', ' ', '#'},
                {'#', '#', '#', '#', '#'},
            },
    });


}