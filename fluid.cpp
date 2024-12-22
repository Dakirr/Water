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

int main(int argc, char** argv) {   
    std::string p_type_name, v_type_name, vf_type_name;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.rfind("--p-type=", 0) == 0) {
            p_type_name = arg.substr(9);
            cerr << p_type_name << "\n";
        } else if (arg.rfind("--v-type=", 0) == 0) {
            v_type_name = arg.substr(9);
            cerr << v_type_name << "\n";
        } else if (arg.rfind("--v-flow-type=", 0) == 0) {
            vf_type_name = arg.substr(14);
            cerr << vf_type_name << "\n";
        }
    }
    if (p_type_name.empty() || v_type_name.empty() || vf_type_name.empty()){
        std::cerr << "Error: --p-type, --v-type, and --vf-type flags are required.\n";
        return 1;
    }
    
    // std::cout << "P type: " << p_type_name << std::endl;
    // std::cout << "V type: " << v_type_name << std::endl;
    // std::cout << "VF type: " << vf_type_name << std::endl;
    //exit(0);
    
    // Вариант 1. Про сами симуляторы и сохранение (они работают нормально, вроде.)
    using P = Fixed<32, 8>;
    using V = FastFixed<64, 8>;
    using VF = Fixed<16, 8>;

    LiquidSimDynamic<101, P, V, VF> LSD = load<101, P, V, VF>("LiquidSimStatic_Tick_№300");
    LSD.save("Test_5_new");
    LSD.name = "LiquidSimDynamic";
    LSD.simulate();   



    // Вариант 2. Создание из флагов (работает, кажется, но немного странно. Я не знаю, это моя проблема, или прикол физики.)
    // Оно компилируется и что-то выводит, во всяком случае.
    // Код непосредственно обработки флагов написан не мной, но туда впаян мой симулятор, и я, кажется, понимаю, что происходит.
    // Подробнее можно увидеть в LST::start_simulation() [./Parcer/simulator.hpp:102:0]
    const LST sim = LST::from_params(types::SimulationParams{
        .p_type_name      = p_type_name,
        .v_type_name      = v_type_name,
        .v_flow_type_name = vf_type_name,
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
