# calc

A simple command line calculator.

# Installation

```bash
git clone https://github.com/leon-zanker/calc calc
cd calc
gcc calc.c -o calc -lm
mkdir -p ~/.local/bin
cp calc ~/.local/bin 
```

# Usage

```bash
calc "(5 + -4.25) / (-2 - ((3.4 ^ 3) * 0.1)) % 2"
```

Operators must be separated from numbers by whitespace.

Numbers directly prefixed by + or - become signed with that prefix.

# Supported operators

- Addition:       +
- Subtraction:    -
- Multiplication: *
- Division:       /
- Exponentiation: ^
- Modulus:        %
