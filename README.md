# Nitro Enclave `suma5` en C++

Proyecto minimo para entender una comunicacion `vsock` entre parent y enclave.

## Que hace

El proceso del enclave:

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

El esquema esta en `proto/sum5.proto`.

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

## Construir la imagen Docker del enclave

```bash
docker build -t sum5-enclave:latest .
```

## Construir el `.eif` con `nitro-cli`

```bash
nitro-cli build-enclave \
  --docker-uri sum5-enclave:latest \
  --output-file sum5-enclave.eif
```

## Ejecutar en el enclave

```bash
./build/sum5_enclave
```

O con puerto explicito:

```bash
./build/sum5_enclave 5005
```

## Lo que tiene que hacer el parent

El parent debe:

1. Abrir un socket `AF_VSOCK`.
2. Conectarse al CID del enclave y al puerto `5005`.
3. Construir un `Sum5Request`.
4. Serializarlo con Protobuf.
5. Enviar primero la longitud del payload en 4 bytes big-endian.
6. Enviar el payload serializado.
7. Leer 4 bytes de longitud.
8. Leer el payload y parsearlo como `Sum5Response`.

Si el parent manda `37`, el enclave devolvera `42`.

## Nota

El codigo de este repositorio solo implementa el lado del enclave, que era el objetivo de esta prueba.
El comando `nitro-cli build-enclave` debe ejecutarse en Linux, tal y como indica la documentacion oficial de AWS.
