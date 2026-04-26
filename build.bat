cmake -S . -B build-openqmc -DCMAKE_BUILD_TYPE=Release -DWOST_TOY_USE_OPENQMC=ON
cmake --build build-openqmc --config Release --parallel