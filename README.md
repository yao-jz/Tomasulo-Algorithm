# Tomasulo-Algorithm

Tomasulo algorithm implementation and visualization

## Introduction

A compiler for assembly code, Tomasulo's various micro-architectural units, mainly Load/Store buffers, integer adders, integer multipliers, and their respective reserved stations, are implemented as a virtual machine on top of that. I also implement a visual interface to show the state changes of the pipeline CPUs.

The algorithm is implemented in C++, and the visualization is implemented in Python.

## Supported instruction sets

LW, SW, ADD, ADDI, SUB, AND, ANDI, BEQZ, J, HALT, NOOP

## Architecture

<img width="914" alt="arch" src="https://user-images.githubusercontent.com/53652885/194689530-7fb04506-199e-4500-9e18-420c4350879b.png">
