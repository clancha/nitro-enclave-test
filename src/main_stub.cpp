#include <cstdlib>
#include <iostream>

int main() {
    std::cerr
        << "Este binario solo implementa vsock real en Linux/Nitro Enclaves.\n"
        << "Compila el proyecto dentro de un entorno Linux para ejecutar el servidor del enclave.\n";
    return EXIT_FAILURE;
}
