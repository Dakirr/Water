//include <bits/stdc++.h>
#include "./LiquidSim/LiquidSim.h"
#include "./LiquidSim/LiquidSimDynamic.h"
#include "./LiquidSim/Thread.h"

#include <cstring>
#include <random>
#include <thread>
#include <execution>
#include "./Parcer/simulator.hpp"
#include <chrono>
#include <limits>

#ifndef TYPES
#error "TYPES is not defined"
#endif

#ifdef SIZES

using LST = types::Simulator<types::TypesList<TYPES>, SIZES>;

#else

using LST = types::Simulator<types::TypesList<TYPES>>;

#endif

extern bool flg_rnd;
int main(int argc, char** argv) { 
    std::string p_type_name, v_type_name, vf_type_name;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--p-type=", 0) == 0) {
            p_type_name = arg.substr(9);
        } else if (arg.rfind("--v-type=", 0) == 0) {
            v_type_name = arg.substr(9);
        } else if (arg.rfind("--v-flow-type=", 0) == 0) {
            vf_type_name = arg.substr(14);
        }
    }
    if (p_type_name.empty() || v_type_name.empty() || vf_type_name.empty()){
        std::cerr << "Error: --p-type, --v-type, and --vf-type flags are required.\n";
        return 1;
    }
    
    // Вариант 1. Про сами симуляторы и сохранение (они работают нормально, вроде.)
    // Тут также показан пример ДЗ 3.
    // На моей машине:
    //      За 100 тиков:
    //                для неоптимизированной программы: 13992 ms
    //                для  оптимизированной  программы: 7843 ms


    using P = Fixed<32, 8>;
    using V = FastFixed<64, 8>;
    using VF = Fixed<16, 8>;

    std::thread rand_thread(rand_process);
    optimise = 1;
    auto start = std::chrono::high_resolution_clock::now();
    auto LSD2 = load<1, P, V, VF>("LiquidSimStatic_Tick_№300");
    LSD2.save("Test_5_new");
    LSD2.name = "LiquidSimDynamic1";
    LSD2.simulate();   
    auto load_end = std::chrono::high_resolution_clock::now();
    auto load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(load_end - start);
    std::cerr << "Loading time with optimisations: " << load_duration.count() << "ms\n";
    flg_rnd = true;
    rand_thread.join();


    optimise = 0;
    start = std::chrono::high_resolution_clock::now();
    auto LSD = load<1, P, V, VF>("LiquidSimStatic_Tick_№300");
    LSD.save("Test_5_new2");
    LSD.name = "LiquidSimDynamic1";
    LSD.simulate();   
    load_end = std::chrono::high_resolution_clock::now();
    load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(load_end - start);
    std::cerr << "Loading time without optimisations: " << load_duration.count() << "ms\n";

    

    // Вариант 2. Создание из флагов (работает, кажется, но немного странно. Я не знаю, это моя проблема, или прикол физики.)
    // Оно должно компилироваться и выводить поле во всяком случае.
    // Код непосредственно обработки флагов написан не мной, но туда впаян мой симулятор, и я, кажется, понимаю, что происходит.
    // Подробнее можно увидеть в LST::start_simulation() [./Parcer/simulator.hpp:102:0]
    // Оно может в обработку размеров
    const LST sim = LST::from_params(types::SimulationParams{
        .p_type_name      = p_type_name,
        .v_type_name      = v_type_name,
        .v_flow_type_name = vf_type_name,
    }); 

    sim.start_on_field(types::Context{
        .field =
            std::vector<std::vector<char>>{
                {'#', '#', '#', '#', '#', '#',},
                {'#', '#', '.', '.', ' ', '#',},
                {'#', ' ', ' ', ' ', '.', '#',},
                {'#', '#', '#', '#', '#', '#',},
            },
    });



    //file_random.close();
}
