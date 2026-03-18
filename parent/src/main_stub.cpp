#include <cstdlib>
#include <iostream>

int main() {
    std::cerr
        << "El cliente vsock real solo se compila en Linux.\n"
        << "Compila este proyecto dentro de un entorno Linux para hablar con el enclave.\n";
    return EXIT_FAILURE;
}
