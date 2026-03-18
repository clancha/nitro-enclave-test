# Nitro Enclave `suma5` en C++

Proyecto minimo para entender una comunicacion `vsock` entre parent y enclave.

## Estructura

```text
.
├── parent/
│   ├── CMakeLists.txt
│   ├── Dockerfile
│   └── src/
└── enclave/
    ├── CMakeLists.txt
    ├── Dockerfile
    ├── proto/
    └── src/
```

## Que hace

El parent:

1. Abre un socket `AF_VSOCK`.
2. Se conecta al CID del enclave y al puerto `5005`.
3. Construye un `Sum5Request`.
4. Lo serializa con Protobuf.
5. Envía el mensaje al enclave.
6. Lee el `Sum5Response`.
7. Muestra el resultado por pantalla.

El enclave:

1. Abre un socket `AF_VSOCK`.
2. Escucha en el puerto `5005` por defecto.
3. Recibe un mensaje Protobuf `Sum5Request`.
4. Extrae el campo `value`.
5. Calcula `n + 5`.
6. Devuelve un mensaje Protobuf `Sum5Response`.

## Estructura del protocolo

Peticion:

- 4 bytes con la longitud del payload Protobuf
- payload serializado de `Sum5Request`

Respuesta:

- 4 bytes con la longitud del payload Protobuf
- payload serializado de `Sum5Response`

El esquema esta en [enclave/proto/sum5.proto](/Users/carloslancha/Desktop/proyecto_imse/nitro-enclave-test/enclave/proto/sum5.proto).

```proto
syntax = "proto3";

package nitroenclave;

message Sum5Request {
  int32 value = 1;
}

message Sum5Response {
  int32 result = 1;
}
```

Ejemplo:

- parent envia `Sum5Request { value: 7 }`
- enclave responde `Sum5Response { result: 12 }`

## Compilar

```bash
cmake -S . -B build
cmake --build build
```

Esto genera:

- `build/parent/sum5_parent`
- `build/enclave/sum5_enclave`

## Construir la imagen Docker del enclave

```bash
docker build -t sum5-enclave:latest ./enclave
```

## Construir la imagen Docker del parent

El `Dockerfile` del parent necesita como contexto la raiz del repo, porque reutiliza el esquema Protobuf de `enclave/proto`.

```bash
docker build -t sum5-parent:latest -f parent/Dockerfile .
```

## Construir el `.eif` con `nitro-cli`

```bash
nitro-cli build-enclave \
  --docker-uri sum5-enclave:latest \
  --output-file sum5-enclave.eif
```

## Ejecutar el enclave

```bash
./build/enclave/sum5_enclave
```

O con puerto explicito:

```bash
./build/enclave/sum5_enclave 5005
```

## Ejecutar el parent

Uso:

```bash
./build/parent/sum5_parent <enclave_cid> <numero> [puerto]
```

Ejemplo:

```bash
./build/parent/sum5_parent 16 37
```

Si el enclave esta en el CID `16`, el parent enviara `37` y recibira `42`.

Con Docker:

```bash
docker run --rm sum5-parent:latest 16 37
```

## Nota

El comando `nitro-cli build-enclave` debe ejecutarse en Linux, tal y como indica la documentacion oficial de AWS.
En macOS este proyecto compila stubs informativos; el codigo real `vsock` solo se compila en Linux.
