# DNS Client-Server Prototype

A Linux command-line DNS simulation implemented in C. The project models a client, a local resolver, a root server, several authoritative servers, and a small cache using DNS-style packets that can be inspected in Wireshark.

This repository is a cleaned archival version of a university course team project. Personal identifiers, the original report, and generated files have been excluded.

## Features

- UDP communication between the client and local resolver
- TCP communication between the local resolver and simulated DNS servers
- Iterative lookup across root and authoritative server processes
- Local cache stored in a text file
- Support for `A`, `NS`, `CNAME`, `MX`, and `PTR` resource records
- DNS packet encoding and decoding suitable for network inspection
- Separate loopback addresses for each simulated server

## Architecture

The processes use loopback addresses from `127.0.0.1` to `127.0.0.8`. The address mapping and port are defined in `src/DNS.h`. Each server reads records from its corresponding text file in `src/`.

```text
Client -> Local resolver -> Root server -> Authoritative server
             |
             +-> cache.txt
```

## Build

Requirements:

- Linux or another POSIX-compatible environment
- GCC
- GNU Make

From the repository root:

```bash
make
```

Executables are written to `bin/`.

## Run

The prototype uses port `53`, which may require elevated privileges or conflict with a system DNS service. Review `PORT` in `src/DNS.h` and change it to an unused unprivileged port if necessary.

Start the server processes in separate terminals from the repository root so that their text data files can be found:

```bash
./bin/root
./bin/nation
./bin/other
./bin/edu
./bin/gov
./bin/ptr
./bin/local
./bin/client
```

The client provides an interactive menu for selecting a query type and entering a domain name or reverse-lookup address.

## Repository structure

```text
src/
├── Client.c              # Interactive DNS client
├── local.c               # Local resolver and cache handling
├── Root.c                # Root server entry point
├── Nation.c              # Country-domain server entry point
├── Other.c               # Other top-level domains
├── Edu.c / Gov.c / PTR.c # Authoritative server entry points
├── DNS.c / DNS.h         # DNS packet structures and utilities
├── othersvr.c/.h         # Shared authoritative-server logic
└── *.txt                 # Sample DNS records and cache
```

## Project context

This was a course team project. The repository describes the functionality verifiable in the archived code and does not assign unverified individual ownership of particular components.

