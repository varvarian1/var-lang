# Language Architecture

## Description

This project is a programming language built for fun, learning, and experimentation. The language features static typing, functions, control flow structures, and a custom interpreter that executes code directly from the Abstract Syntax Tree (AST).

## Project Structure

The most important directory is `include/core` - it's the heart of the interpreter. Core has lexer, parser, ast, and interpreter directories.

### Lexer

- Responsible for tokenizing source code
- Utilizes a **Finite-state machine** to identify tokens
- Converts raw source code into a stream of tokens for parsing
- Recognizes keywords (`func`, `let`, `if`, `else`, `for`, `return`, `echo`, `as`)
- Recognizes operators (`+`, `-`, `*`, `/`, `<`, `>`, `<=`, `>=`, `==`, `!=`)
- Recognizes literals (numbers, strings, identifiers)
- Handles whitespace and comments

**States in the FSM:**
- `InNewToken` - Starting state, checks first character
- `InIdentifier` - Building identifier or keyword
- `InNumber` - Building numeric literal
- `InString` - Building string literal
- `InCompleteToken` - Token is ready to be added

### Parser

- Responsible for analyzing the token stream
- Implements a **Recursive descent parser** to build an Abstract Syntax Tree (AST)
- Checks syntax and constructs hierarchical representation of code
- Uses operator precedence parsing for expressions

### Abstract Syntax Tree (AST)

The AST represents the structure of the program in a tree format. Each node corresponds to a language construct:

**Expression Nodes:**
- `BinaryExpr` - Arithmetic operations (+, -, *, /)
- `ComparisonExpr` - Comparison operations (<, >, <=, >=, ==, !=)
- `LiteralNumber` - Numeric constants
- `LiteralString` - String constants
- `Identifier` - Variable/function references
- `FunctionCall` - Function invocation

**Statement Nodes:**
- `VariableDeclarationStmt` - Variable declaration with type
- `AssignStmt` - Variable assignment
- `BlockStmt` - Sequence of statements in curly braces
- `IfStmt` - Conditional execution with optional else branch
- `ForStmt` - Loop with initialization, condition, and increment
- `FunctionStmt` - Function definition with parameters and body
- `ReturnStmt` - Return value from function
- `EchoStmt` - Output to console

### Interpreter

- Responsible for executing the AST
- Implements the **Visitor pattern** to traverse the AST
- Manages variable storage and function lookup
- Evaluates expressions and executes statements

**Key Components:**

**Value** - Represents runtime values:
- Supports multiple types: Number, String, Boolean, Void
- Provides type checking and conversion methods

**Environment** - Manages variable and function storage:
- Hierarchical scoping (supports nested blocks)
- Variable lookup searches parent scopes
- Functions are stored and can be called by name

**Visitor Pattern Implementation:**
- Each AST node implements an `accept()` method
- The interpreter implements all `visit()` methods
- Double dispatch enables proper method resolution
