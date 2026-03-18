#include <arpa/inet.h>
#include <linux/vm_sockets.h>
#include <sys/socket.h>
#include <unistd.h>

#include <google/protobuf/stubs/common.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "sum5.pb.h"

namespace {

constexpr std::uint32_t kDefaultPort = 5005;

std::int32_t suma5(std::int32_t value) {
    if (value > std::numeric_limits<std::int32_t>::max() - 5) {
        throw std::overflow_error("sumar 5 desborda int32");
    }
    return value + 5;
}

bool read_exact(int fd, void* buffer, std::size_t length) {
    auto* bytes = static_cast<std::uint8_t*>(buffer);
    std::size_t offset = 0;

    while (offset < length) {
        const ssize_t received = ::recv(fd, bytes + offset, length - offset, 0);
        if (received == 0) {
            return false;
        }
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("recv fallo: ") + std::strerror(errno));
        }
        offset += static_cast<std::size_t>(received);
    }

    return true;
}

template <typename Message>
bool read_message(int fd, Message& message) {
    std::uint32_t size_be = 0;
    if (!read_exact(fd, &size_be, sizeof(size_be))) {
        return false;
    }

    const std::uint32_t size = ntohl(size_be);
    std::string payload(size, '\0');
    if (size > 0 && !read_exact(fd, payload.data(), payload.size())) {
        return false;
    }

    if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        throw std::runtime_error("No se pudo parsear el mensaje protobuf");
    }

    return true;
}

void write_exact(int fd, const void* buffer, std::size_t length) {
    const auto* bytes = static_cast<const std::uint8_t*>(buffer);
    std::size_t offset = 0;

    while (offset < length) {
        const ssize_t sent = ::send(fd, bytes + offset, length - offset, 0);
        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("send fallo: ") + std::strerror(errno));
        }
        offset += static_cast<std::size_t>(sent);
    }
}

template <typename Message>
void write_message(int fd, const Message& message) {
    const std::size_t size = message.ByteSizeLong();
    if (size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("El mensaje protobuf es demasiado grande");
    }

    std::string payload(size, '\0');
    if (!message.SerializeToArray(payload.data(), static_cast<int>(payload.size()))) {
        throw std::runtime_error("No se pudo serializar el mensaje protobuf");
    }

    const std::uint32_t size_be = htonl(static_cast<std::uint32_t>(payload.size()));
    write_exact(fd, &size_be, sizeof(size_be));
    if (!payload.empty()) {
        write_exact(fd, payload.data(), payload.size());
    }
}

std::uint32_t parse_port(int argc, char** argv) {
    if (argc < 2) {
        return kDefaultPort;
    }

    const std::string raw = argv[1];
    std::size_t consumed = 0;
    const unsigned long port = std::stoul(raw, &consumed, 10);
    if (consumed != raw.size() || port > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument("puerto invalido");
    }

    return static_cast<std::uint32_t>(port);
}

void handle_client(int client_fd) {
    nitroenclave::Sum5Request request_message;
    if (!read_message(client_fd, request_message)) {
        std::cerr << "Cliente desconectado antes de enviar la peticion completa\n";
        return;
    }

    const std::int32_t request = request_message.value();
    const std::int32_t response = suma5(request);

    nitroenclave::Sum5Response response_message;
    response_message.set_result(response);

    std::cout << "Peticion recibida: " << request << ", respuesta: " << response << '\n';
    write_message(client_fd, response_message);
}

}  // namespace

int main(int argc, char** argv) {
    try {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        const std::uint32_t port = parse_port(argc, argv);

        const int server_fd = ::socket(AF_VSOCK, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "No se pudo crear el socket vsock: " << std::strerror(errno) << '\n';
            google::protobuf::ShutdownProtobufLibrary();
            return EXIT_FAILURE;
        }

        sockaddr_vm address {};
        address.svm_family = AF_VSOCK;
        address.svm_cid = VMADDR_CID_ANY;
        address.svm_port = port;

        if (::bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            std::cerr << "bind fallo: " << std::strerror(errno) << '\n';
            ::close(server_fd);
            google::protobuf::ShutdownProtobufLibrary();
            return EXIT_FAILURE;
        }

        if (::listen(server_fd, 16) < 0) {
            std::cerr << "listen fallo: " << std::strerror(errno) << '\n';
            ::close(server_fd);
            google::protobuf::ShutdownProtobufLibrary();
            return EXIT_FAILURE;
        }

        std::cout << "Enclave escuchando en vsock puerto " << port << '\n';

        while (true) {
            const int client_fd = ::accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) {
                if (errno == EINTR) {
                    continue;
                }
                std::cerr << "accept fallo: " << std::strerror(errno) << '\n';
                break;
            }

            try {
                handle_client(client_fd);
            } catch (const std::exception& ex) {
                std::cerr << "Error al procesar cliente: " << ex.what() << '\n';
            }

            ::close(client_fd);
        }

        ::close(server_fd);
        google::protobuf::ShutdownProtobufLibrary();
        return EXIT_FAILURE;
    } catch (const std::exception& ex) {
        std::cerr << "Error fatal: " << ex.what() << '\n';
        google::protobuf::ShutdownProtobufLibrary();
        return EXIT_FAILURE;
    }
}
