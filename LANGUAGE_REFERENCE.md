Here's the LANGUAGE REFERENCE saved as a Markdown file:

# VAR Programming Language Reference

## Table of Contents
1. [Introduction](#introduction)
2. [Basic Syntax](#basic-syntax)
3. [Data Types](#data-types)
4. [Variables](#variables)
5. [Operators](#operators)
6. [Functions](#functions)
7. [Control Flow](#control-flow)
8. [Built-in Functions](#built-in-functions)
9. [Comments](#comments)
10. [Example Programs](#example-programs)

---

## Introduction

VAR is a statically-typed, C-style programming language with an interpreter-based implementation. It supports integers, floats, strings, booleans, functions, and standard control flow constructs.

---

## Basic Syntax

VAR statements end with semicolons (`;`). Code blocks are enclosed in curly braces (`{}`).

```var
// Variable declaration
int x = 10;

// Function definition
func main() as int {
    return 0;
}
```

---

## Data Types

| Type | Description | Example |
|------|-------------|---------|
| `int` | Integer numbers | `42`, `-17`, `0` |
| `float` | Floating-point numbers | `3.14`, `-0.5`, `2.0` |
| `str` | String literals | `"Hello, World!"` |
| `bool` | Boolean values | `true`, `false` |
| `void` | No return value (functions only) | - |

---

## Variables

### Declaration

Variables are declared using the syntax: `type name = value;`

```var
int age = 25;
float pi = 3.14159;
str greeting = "Hello";
bool isReady = true;
```

### Declaration without Initialization

Uninitialized variables get default values:
- `int` → `0`
- `float` → `0.0`
- `str` → `""` (empty string)
- `bool` → `false`

```var
int count;      // = 0
str name;       // = ""
```

### Assignment

Use the `=` operator:

```var
int x = 5;
x = 10;         // Reassign
```

---

## Operators

### Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `x + y` |
| `-` | Subtraction | `x - y` |
| `*` | Multiplication | `x * y` |
| `/` | Division | `x / y` |

```var
int a = 10 + 5;      // 15
int b = a - 3;       // 12
int c = b * 2;       // 24
int d = c / 4;       // 6 (integer division truncates)
```

### Comparison Operators

| Operator | Description |
|----------|-------------|
| `==` | Equal to |
| `!=` | Not equal to |
| `<` | Less than |
| `<=` | Less than or equal |
| `>` | Greater than |
| `>=` | Greater than or equal |

```var
if (x == 10) { }
if (y != 5) { }
if (age >= 18) { }
```

### Increment/Decrement Operators

| Operator | Description |
|----------|-------------|
| `++` | Increment by 1 |
| `--` | Decrement by 1 |

Both prefix and postfix forms are supported:

```var
int i = 5;
i++;            // Postfix: i becomes 6
++i;            // Prefix: i becomes 7

int a = i++;    // a = 7, i = 8
int b = ++i;    // b = 9, i = 9
```

### Operator Precedence

From highest to lowest:

1. Postfix: `++` `--`
2. Unary: `+` `-` `++` `--` (prefix)
3. Multiplicative: `*` `/`
4. Additive: `+` `-`
5. Relational: `<` `>` `<=` `>=`
6. Equality: `==` `!=`
7. Assignment: `=`

---

## Functions

### Function Declaration

```var
func name(parameters) as returnType {
    // body
    return value;
}
```

### Parameters

Parameters are declared with type and name:

```var
func add(int a, int b) as int {
    return a + b;
}
```

### Return Types

- Use `as <type>` after the parameter list
- If no return type is specified, defaults to `void`
- Use `return` to return a value

```var
func greet(str name) as void {
    echo("Hello, " + name);  // No return needed
}

func getPi() as float {
    return 3.14159;
}
```

### The `main` Function

Every program must have a `main` function as the entry point:

```var
func main() as int {
    // Program starts here
    return 0;
}
```

### Calling Functions

```var
func main() as int {
    int result = add(5, 3);     // 8
    greet("Alice");
    return 0;
}
```

---

## Control Flow

### If-Else Statement

```var
if (condition) {
    // executed if condition is true
} else {
    // executed if condition is false
}
```

Example:

```var
int score = 85;

if (score >= 90) {
    echo("A grade");
} else if (score >= 80) {
    echo("B grade");
} else {
    echo("C grade or below");
}
```

### For Loop

```var
for (initializer; condition; increment) {
    // loop body
}
```

Example:

```var
// Count from 0 to 9
for (int i = 0; i < 10; i++) {
    echo(i);
}

// Count down
for (int i = 10; i > 0; i--) {
    echo(i);
}
```

---

## Built-in Functions

### `echo()`

Outputs values to the console. Accepts any number of arguments (comma-separated).

```var
echo("Hello");           // Output: Hello
echo(42);                // Output: 42
echo(3.14);              // Output: 3.14

// Multiple arguments
echo("Value:", x, "times");
```

---

## Comments

VAR supports both single-line and multi-line comments.

### Single-line comments

```var
// This is a comment
int x = 10;  // Inline comment
```

### Multi-line comments

```var
/*
   This is a
   multi-line comment
*/
int y = 20;
```

---

## Complete Example Programs

### Example 1: Basic Arithmetic

```var
func add(int a, int b) as int {
    return a + b;
}

func main() as int {
    int x = 10;
    int y = 20;
    int sum = add(x, y);
    
    echo("Sum:", sum);
    return 0;
}
```

### Example 2: Loop and Condition

```var
func isEven(int n) as bool {
    return n % 2 == 0;
}

func main() as int {
    for (int i = 1; i <= 10; i++) {
        if (isEven(i)) {
            echo(i, "is even");
        } else {
            echo(i, "is odd");
        }
    }
    return 0;
}
```

### Example 3: Function with Float

```var
func calculateArea(float radius) as float {
    float pi = 3.14159;
    return pi * radius * radius;
}

func main() as int {
    float r = 5.0;
    float area = calculateArea(r);
    echo("Area of circle with radius", r, "is", area);
    return 0;
}
```

### Example 4: Nested Loops

```var
func main() as int {
    for (int i = 1; i <= 5; i++) {
        for (int j = 1; j <= i; j++) {
            echo("*", "");
        }
        echo("");  // New line
    }
    return 0;
}
```

---

## Running VAR Programs

```bash
# Run a file
./var program.var

# Check syntax only
./var --check program.var

# Execute inline code
./var --eval "echo('Hello')"

# REPL mode
./var --eval

# Help
./var --help
```

---

## Notes and Limitations

1. **Integer Division**: Division of two integers truncates toward zero
2. **Type Preservation**: `int` + `int` = `int`, `float` + `int` = `float`
3. **String Concatenation**: Use `+` to concatenate strings
4. **Variable Scope**: Variables are block-scoped
5. **Function Overloading**: Not supported
6. **Arrays**: Not currently supported

---

*This reference is based on VAR language version as of May 2026.*
```

To save this to a file, you can copy the content above or run:

```bash
cat > LANGUAGE_REFERENCE.md << 'EOF'
[paste the content above]
EOF
```