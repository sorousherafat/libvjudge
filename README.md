# libvjudge

the core of an automatic judge for verilog HDL code.

## Components

- [`libvcd`](https://github.com/sorousherafat/libvcd) is used as an in-house solution to parsing `.vcd` files.
- [`vjudge-cli`](https://github.com/sorousherafat/vjudge-cli) is a command-line tool for automatic judge of verilog HDL code, written in C, leveraging `libvjudge`.
- [`vjudge-server`](https://github.com/mehradMilan/vjudge-server) is a HTTP server that listens for GitHub's `push` webhook events to trigger the judge, written in Go, leveraging `libvjudge`.
