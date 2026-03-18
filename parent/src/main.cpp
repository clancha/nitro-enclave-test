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

std::uint32_t parse_u32(const std::string& raw, const char* field_name) {
    std::size_t consumed = 0;
    const unsigned long value = std::stoul(raw, &consumed, 10);
    if (consumed != raw.size() || value > std::numeric_limits<std::uint32_t>::max()) {
        throw std::invalid_argument(std::string("Valor invalido para ") + field_name);
    }
    return static_cast<std::uint32_t>(value);
}

std::int32_t parse_i32(const std::string& raw, const char* field_name) {
    std::size_t consumed = 0;
    const long value = std::stol(raw, &consumed, 10);
    if (consumed != raw.size()
        || value < std::numeric_limits<std::int32_t>::min()
        || value > std::numeric_limits<std::int32_t>::max()) {
        throw std::invalid_argument(std::string("Valor invalido para ") + field_name);
    }
    return static_cast<std::int32_t>(value);
}

struct Options {
    std::uint32_t cid;
    std::int32_t value;
    std::uint32_t port;
};

Options parse_args(int argc, char** argv) {
    if (argc != 3 && argc != 4) {
        throw std::invalid_argument("Uso: sum5_parent <enclave_cid> <numero> [puerto]");
    }

    Options options {};
    options.cid = parse_u32(argv[1], "enclave_cid");
    options.value = parse_i32(argv[2], "numero");
    options.port = argc == 4 ? parse_u32(argv[3], "puerto") : kDefaultPort;
    return options;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        const Options options = parse_args(argc, argv);

        const int client_fd = ::socket(AF_VSOCK, SOCK_STREAM, 0);
        if (client_fd < 0) {
            std::cerr << "No se pudo crear el socket vsock: " << std::strerror(errno) << '\n';
            google::protobuf::ShutdownProtobufLibrary();
            return EXIT_FAILURE;
        }

        sockaddr_vm address {};
        address.svm_family = AF_VSOCK;
        address.svm_cid = options.cid;
        address.svm_port = options.port;

        if (::connect(client_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            std::cerr << "connect fallo: " << std::strerror(errno) << '\n';
            ::close(client_fd);
            google::protobuf::ShutdownProtobufLibrary();
            return EXIT_FAILURE;
        }

        nitroenclave::Sum5Request request;
        request.set_value(options.value);
        write_message(client_fd, request);

        nitroenclave::Sum5Response response;
        if (!read_message(client_fd, response)) {
            std::cerr << "El enclave cerro la conexion sin responder\n";
            ::close(client_fd);
            google::protobuf::ShutdownProtobufLibrary();
            return EXIT_FAILURE;
        }

        std::cout << response.result() << '\n';

        ::close(client_fd);
        google::protobuf::ShutdownProtobufLibrary();
        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Error fatal: " << ex.what() << '\n';
        google::protobuf::ShutdownProtobufLibrary();
        return EXIT_FAILURE;
    }
}
