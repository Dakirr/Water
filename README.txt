Про ДЗ 2:
    Cказать, в общем, нечего, вся информация есть в комментариях в файле fluid.h
    При попытке все это понять, в какой-то момент LLM сказала мне следующее:
    
        "This cleaned-up approach avoids preprocessor tricks that obscure the code and directly
        leverages C++'s type system features, like templates and constexpr variables, 
        improving clarity, type safety, and maintainability. You'll thank yourself later for moving away
        from this macro-heavy design."
        
Про ДЗ 3:
    Есть идея использовать ThreadPool для параллелизации dfs. 
    Сейчас есть параллелизация пары циклов в FluidSim::simulate, которая не дает существенного прироста по скорости.
