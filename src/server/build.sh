## Windows compilation
cd build
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..

## Make file compile
mingw32-make.exe

## Run
./main.exe
