# onyx

```ascii
                  ██████  ████████   █████ ████ █████ █████
                 ███░░███░░███░░███ ░░███ ░███ ░░███ ░░███ 
                ░███ ░███ ░███ ░███  ░███ ░███  ░░░█████░  
                ░███ ░███ ░███ ░███  ░███ ░███   ███░░░███ 
                ░░██████  ████ █████ ░░███████  █████ █████
                 ░░░░░░  ░░░░ ░░░░░   ░░░░░███ ░░░░░ ░░░░░ 
                                      ███ ░███             
                                     ░░██████              
                                      ░░░░░░            
```

## 1. Vision and scope

`onyx` is an initiative for modern, accurate programming, combining performance
with a compact syntax. It is a research language intended to serve several
purposes in computing, with native support for numerical programming and
multidimensional tensors.

This document is organised from the smallest concepts (syntax, values and
expressions) to the most advanced ones (tensors, objects and implementation).
Rules in the final **Open Design Questions** section are not yet part of the
stable language specification.

### Design principles

- Prefer predictable rules over many special cases.
- Keep type annotations optional when inference is unambiguous.
- Make numerical conversion and tensor-shape rules explicit.
- Specify errors and edge cases before relying on them in an implementation.

## 2. Core syntax

### Blocks and comments

Blocks are introduced by `:` and delimited by indentation. Comments use `(<`
to open and `>)` to close; they can be single-line or multi-line.

```onyx
if active:
    print("Running")

(< This is a comment. >)
```

**To decide:** indentation policy (spaces/tabs, width) and whether empty blocks
are allowed.

### Variable declarations

Variables can be declared using inference or an explicit type annotation.

```onyx
variable_name = data
variable_name: data_type = data
```

Bindings are mutable by default. A later assignment to an existing variable is
permitted:

```onyx
value = 1
value = 2
```

**To decide:** constants, declarations without a value, and the exact point at
which an annotation is checked.

## 3. Values and types

### Basic data types

We can note that these are the fundamental data types found in many
programming language. However, we need to mention new types integration to
the language, to improve syntax and prorgamming accross precision,
optimization, and diversity.

- `string`

- `int` (, int32, int64, int128, int256, etc...)

- `real` (, r32, r64, r128, r256, etc...)

- `complex`

- `quaternion`

- `bool`

- `arr` for array

- `none`

Note: There is interoperability between numerical types (i.e. `int`, `real`,
`complex`, and `quaternion`).

This suggests that the basic operators can be applied in between them without any problem.

### Numeric operations and interoperability

Here we'll discuss about type compatibility with some operations and tell more details on interaction between types and operations (interoperability).

#### Numeric promotion

Numeric operations follow a single widening rule rather than a long per-operator list.
The hierarchy is:

`int -> real -> complex -> quaternion`

The result of a binary numeric operation is the widest numeric type present in the operands.
This means the following are the canonical cases:

- `int + int = int`
- `int + real = real`
- `real + real = real`
- `int + complex = complex`
- `real + complex = complex`
- `complex + complex = complex`
- `int + quaternion = quaternion`
- `real + quaternion = quaternion`
- `complex + quaternion = quaternion`
- `quaternion + quaternion = quaternion`

The same rule applies to subtraction, multiplication, and division among
numeric values. Exponentiation involving quaternions remains to be specified.
This gives a compact and predictable rule set without requiring every pairwise combination to be listed explicitly.

Examples:

- `2 + 2 = 4`
- `1 + 2.856295 = 3.856295`
- `2.52 + 2.48 = 5.0`
- `(2 + 52i) + (2 + 48i) = 4 + 100i`
- `1.79809804 + (2 + 8i) = 3.79809804 + 8i`
- `2 - 2 = 0`
- `1 - 2.856295 = -1.856295`
- `2.52 - 2.48 = 0.04`
- `(2 + 52i) - (2 + 48i) = 4i`
- `1.79809804 - (2 + 8i) = -0.20190196 - 8i`
- `2 * 2 = 4`
- `1 * 2.856295 = 2.856295`
- `2.52 * 2.48 = 6.2496`
- `(2 + 52i) * (2 + 48i) = -2492 + 200i`
- `1.79809804 * (2 + 8i) = 3.59619608 + 14.38478432i`
- `2 / 4 = 0.5`

This rule keeps the language consistent with the usual arithmetic hierarchy while separating numeric semantics from non-numeric operations such as string concatenation and tensor behavior.

The exact treatment of precision, overflow, integer division, and division by zero remains a design question to be resolved in the numeric model.

#### Arithmetic operators

- `+` adds numeric values according to numeric promotion. `string + string`
  concatenates strings.
- `-` and `*` operate on numeric values according to numeric promotion.
- `/` operates on numeric values according to numeric promotion. Division of two
  integers yields a `real` result.
- `//` is integer division. It accepts integers only and returns an `int`.
  Its rounding rule for negative operands remains part of the numeric model.
- `%` is the integer remainder operator. It accepts integers only. For a positive
  divisor `b`, `a % b` is the unique remainder `r` such that
  `a = b * q + r` and `0 <= r < b`. Division by zero is an error.
- `^` is numeric exponentiation. It has a higher precedence than `*` and `/` and
  is right-associative: `2 ^ 3 ^ 2` means `2 ^ (3 ^ 2)`. Quaternion powers are
  defined in the **Quaternions** section.
- Unary `+value` and `-value` are supported for numeric values.
- `ln()` and `exp()` accept `int`, `real`, `complex`, and `quaternion` values.
  Quaternion semantics are defined in the **Quaternions** section.
- `abs()` accepts numeric values. It returns the modulus of a complex number and
  the norm of a quaternion, in both cases as a `real`.

The precise handling of non-integer powers, domains, precision, overflow, and
exceptional floating-point values remains part of the numeric model.

#### Comparison operators

Onyx provides `==`, `!=`, `<`, `<=`, `>`, and `>=`.

- Numeric comparisons apply numeric promotion, so `2 == 2.0` is `true`.
- Strings are compared by their contents.
- `none == none` is `true`.
- Ordering comparisons (`<`, `<=`, `>`, `>=`) are invalid for `complex` and
  `quaternion`, because these types have no natural total order.
- Tensor comparison semantics are intentionally deferred.

#### Boolean operators

Onyx uses keyword boolean operators:

```onyx
not active
is_admin and has_access
is_guest or is_admin
```

`and` and `or` use short-circuit evaluation. Conditions in `if`, `elif`, and
other control-flow constructs must evaluate to `bool`; values such as `0`, an
empty string, and an array are not implicitly converted to `bool`.

#### Operator precedence

The currently defined precedence, from highest to lowest, is:

1. Function calls and indexing (indexing syntax remains open).
2. `^` (right-associative).
3. Unary `+` and unary `-`.
4. `*`, `/`, and `%`.
5. `+` and `-`.
6. `<`, `<=`, `>`, and `>=`.
7. `==` and `!=`.
8. `not`.
9. `and`.
10. `or`.

Arithmetic operators at the same precedence level are evaluated from left to
right, except `^`.

Array operations, including elementwise operations, matrix products, and
broadcasting, remain to be specified separately.

#### Quaternions

A `quaternion` has four real components and is written mathematically as
`a + bi + cj + dk`. It extends `complex` through the subspace `a + bi`.

Quaternions are constructed with the canonical component order `(real, i, j, k)`:

```onyx
q: quaternion = quaternion(1, 2, 3, 4) (< 1 + 2i + 3j + 4k >)
```

The units follow the standard multiplication rules:

```text
i² = j² = k² = -1
ij = k, jk = i, ki = j
ji = -k, kj = -i, ik = -j
```

Quaternion multiplication is associative but not commutative. Therefore,
`a * b` and `b * a` must not be assumed equal when either operand is a
quaternion. The language fixes `i` as the complex unit embedded in a
quaternion, making promotion from `complex` to `quaternion` unambiguous.

`abs(q)` returns the real quaternion norm. Division by the zero quaternion is
an error.

`exp(q)` is defined for every quaternion. `ln(q)` is defined for every non-zero
quaternion and uses a deterministic principal branch. For a negative real value
represented as a quaternion, whose imaginary axis is otherwise ambiguous, the
principal logarithm uses the `i` axis:

```onyx
ln(quaternion(-2, 0, 0, 0)) (< ln(2) + πi >)
```

Quaternion powers accept integer and real exponents:

- `q ^ n`, where `n` is an `int`, uses integer powers; `q ^ 0` is the
  multiplicative identity `quaternion(1, 0, 0, 0)`.
- `q ^ r`, where `r` is a `real`, is defined as `exp(r * ln(q))` using the
  principal logarithm.
- A quaternion exponent is not supported in the first version.

A little remark: all mathematical operations listed here have exactly the same properties as the mathematical rules operate.
`token` and `map` are set aside for now -- to be introduced later once their syntax is decided.

## 4. The `arr` type — Multidimensional tensors

`arr` is not a flat list: it is a true multidimensional tensor, meaning
each of its axes is independent from the others (as opposed to matrices
that would be concatenated/assembled into blocks).

```onyx
arr[ElementType] <d1, ..., dN>
```

- `ElementType` (inside `[...]`) is the type stored in every cell of the
  tensor (homogeneous: every cell has the same type).
- `<d1, ..., dN>` (inside `<...>`) is the shape: the size of each axis.
- The rank (number of axes) is never written separately -- it is simply
  the number of sizes given between `<...>`. Writing 3 sizes always
  means a rank-3 tensor, automatically; there is no redundant "rank"
  parameter to keep in sync.

```onyx
(< Basic declaration, for memory allocation for example.
   <2, 3, 6> means 2 shells, 3 lines, 6 columns. >)
var_array: arr[r128] <2, 3, 6>

(< Declaration with an initial value: the literal is a flat list, given
   in order, matching the total element count (1x1x10 = 10 values here). >)
var_array: arr[int32] <1, 1, 10> = [1, 2, 3, 4, 5, 6, 7, 53, 19, 10]
```

Note: `arr` is a tensor abstraction with independent axes. It is separate from
hypercomplex-number systems such as quaternions and, potentially in the future,
Clifford algebras.

### Indexing, shape, and mutation

An `arr` is indexed with one integer coordinate per axis. The number of indices
must equal the tensor rank:

```onyx
values[index]                    (< arr[T] <n> >)
values[layer, line, index]       (< arr[T] <d1, d2, d3> >)
```

An index outside the bounds of its axis is an error. Supplying too few or too
many indices is also an error. An indexed cell can be read or written when the
assigned value is compatible with the array element type.

```onyx
values[index] = value
```

Every array exposes `.shape`, which represents its full shape. A rank-one array
also exposes `.length`, which is its sole dimension:

```onyx
numbers: arr[int] <5> = [64, 25, 12, 22, 11]
size: int = numbers.length
dimensions = numbers.shape
```

**To decide:** the type and formatting of `.shape`, negative indices, slices,
and views.

## 5. Language constructs

### Lambdas

```onyx
lam_func = lambda (n: int) = n + 1
result = lam_func(2)
print("And the result is: {result}") (< It should be 3 >)
print("And the result is: {lam_func(8)}")

(< Zero-parameter lambda: parentheses are always required, never omitted. >)
no_op = lambda () = none
```

There is only one way to use this: only as shown in the example. `=`
separates the parameter list from the body -- no return type is written
after the parentheses, and the parentheses are never omitted, even with
zero parameters.

### Named functions

A named function has a parameter list, a required return type annotation, and
an indented body. The `->` between the parameter list and the return type is
part of the function signature. `=> expression` returns immediately from the
current function.

```onyx
add(a: int, b: int) -> int:
    => a + b

absolute(n: int) -> int:
    if n < 0:
        => -n
    => n

display(text: string) -> none:
    print(text)
    => none
```

`=>` is a return construct, not a lambda syntax. A function return type is
checked against the type of every returned expression.

An `arr` parameter grants the function mutable access to the passed array
storage. Mutations therefore affect the caller's array; no `mut` marker is
required in the first version.

```onyx
selection_sort(values: arr[int] <n>) -> arr[int] <n>:
    values[0] = 0
    => values
```

After `selection_sort(numbers)`, `numbers` contains the modified values. The
precise model for assignment, aliasing, copies, and views remains open.

`::` is reserved for a future `process` construct and has no meaning in the
current language.

**To decide:** recursion, function overloading, named-function scope, closure
capture, and whether all control-flow paths must return a value.

### Classes

```onyx
(< Simulating Value class from micrograd: >)

class Value:
    #from(* data: real, children: tuple = (), op: string = "", label: string = "") (<They all become attributes. `data` is the only required one -- the others must have a default value, or it's a compile/interpret error.>)
    .grad: real = 0.0 (< All variables preceded of a dot belong to the instance. >)
    .backward: none = lambda () = none
    #format = "Value({.data})" (<Formatting function for `Value` instance.>)
```

### Output and string formatting

```onyx
var: string = "programming language."
print("How to use a {var}")
print('How to use   {var}')
```

### Pattern matching

`match` selects a branch according to the value being tested.

```onyx
value = 9

match value:
    case 1:
        print("Monday")
    case 2:
        print("Tuesday")
    case 3:
        print("Wednesday")
    case 4:
        print("Thursday")
    case 5:
        print("Friday")
    case 6:
        print("Saturday")
    case 7:
        print("Sunday")
    default:
        print("Value error encountered")
```

### Error handling: `try` / `fallback`

The `try` block is evaluated and `fallback` is executed if it cannot complete
because of an error. Error propagation, rollback behaviour, and access to the
original error remain to be defined.

```onyx
try:
    this = Value(2.6)
fallback:
    print("Error: Failed to initialize this.")
```

### Conditional execution

`if`, `elif`, and `else` select a block from boolean conditions.

```onyx
real_var = 0.366565649486516846584684

if real_var > 1:
    print("{real_var} is greater than one.")
elif real_var < 1:
    print("{real_var} is less than one")
else:
    print("{real_var} is equal to 1")
```

### `while` loops

A `while` loop repeats its body while its condition evaluates to `bool`.

```onyx
index: int = 0
while index < 3:
    print("index = {index}")
    index = index + 1
```

`=> expression` inside a loop returns from the enclosing function immediately.
It consequently exits every active loop before control leaves the function.

`:break` exits only the innermost active loop. When a `while` is used as an
expression, `:break value` exits that loop and makes `value` the result of the
`while` expression. A `while` that ends through `:break` without a value, or
whose condition becomes `false`, evaluates to `none`.

```onyx
current: int = 5

position: int = while true:
    if current >= 9:
        :break current
    current += 1
```

`continue` is reserved for future design.

### Blocks, scope, and lifetime

Every indented block creates a child lexical scope. A child scope can read and
modify variables declared in any enclosing parent scope. A parent scope cannot
access variables declared only in one of its child scopes.

```onyx
total: int = 0

while total < 3:
    increment: int = 1

    if total >= 0:
        total += increment

print(total)     (< Valid: 3 >)
print(increment) (< Error: increment is out of scope >)
```

Scope determines name visibility. Lifetime determines how long a value must
remain available in memory; its exact implementation is part of the memory
model and may be managed by the compiler. A value can outlive its declaring
block only through an explicit mechanism such as a function return or a closure
capture.

**To decide:** shadowing, closure capture rules, and the exact lifetime and
aliasing model for values that escape a block.

## 6. Object-model conventions and concepts

- The `stranger` concept

    In a class/data structure definition,
    all variables preceded by a dot belong to the
    class definition body, all other variables
    are strangers.

- Class configuration for data types and classes

    A class is first of all, a data structure and computed
    representation of data and situational concepts to illustrate
    a real world situation. Looking at an object's structure,
    we can see that it has so many useful components we can explore, such as:

  - Parents:

    An object is certainly the result of it's preceding state. If it doesn't, it means it's a new concept at new sight. In onyx, we use the `#parents(...)` syntax to enumerate and collect it's preceding components. When used, this means that the child class heritates the methods, the attributes and all possible other characteristics of its parents.

  - Exterior Factors:

    This is certainly more complex than other concepts
    to get. These are the outside variables that helps us to
    create the instance of an object, taken in account for the
    object to exist. Those vars become certainly the property of
    the object so created. They belong to the object. This is
    certainly done with the help of the `#from(...)` keyword, that
    initializes those variables in the object's constructor to
    make them it's property.

  - Format:

    All the objects we use in a situation gives us information on
    how useful they can be in a situation. They possess attributes
    and that is why they appear in a given way. This been said, it
    suggests that these informations can be represented to us the
    way we want to see them. That's why we use `#format = "..."` to
    express our class's implementation's representation, with there
    is to be known by the language.

  - The IMPORTANT concept:

    In the `#from` operator, it is important to note that, if a `*` is
    preceding a variable, this means, that the concerned variables
    are required to create a class. If no default value is attributed
    to the other vars, the program will automatically raise an error and not compile (or interpret).

## 7. Consumption, persistence, and sharing

A variable in onyx is **linear** by default: when its value is used to
produce a new value of the same kind, the variable is **consumed** — it
becomes invalid, and the new value takes its place. This makes memory
ownership explicit without requiring pointers or manual allocation.

### What consumes a variable

A variable is consumed only when it appears as a direct operand in a
**transformation** — an operation that produces a new value of the same kind
(an `arr` producing an `arr`, a class producing a class, and so on).

The following operations **consume** their `arr` operand:

- direct assignment: `b = a`
- array-returning functions: `b = sort(a)`, `b = map(f, a)`
- array arithmetic producing a new array: `b = a + a`

The following operations **do not consume** their operand:

- reads and projections: `print(a)`, `a[0]`, `a.length`, `a.shape`
- scalar-producing reductions: `sum(a)`, `first(a)`, `product(a)`
- scalar comparisons and boolean tests

In every non-consuming case, the variable remains valid after the
operation.

```onyx
a: arr[int] <3> = [1, 2, 3]

b = a              (< consumes a: a is no longer valid >)
print(a)           (< error: a was consumed by the previous line >)
```

```onyx
a: arr[int] <3> = [1, 2, 3]

n: int = sum(a)    (< does not consume a >)
m: int = a[0]      (< does not consume a >)
print(a)           (< valid: a = [1, 2, 3] >)
```

### Persistent variables (`!i`)

A variable declared with the `!i` prefix is **persistent**. Persistent
variables are never consumed. When a transformation would consume a
persistent variable, the language operates on a copy instead; the original
variable remains valid and unchanged.

```onyx
!i a: arr[int] <3> = [1, 2, 3]

b = a              (< a is not consumed; b shares a's value >)
c = sort(a)        (< a is not consumed; c is a sorted copy >)

print(a)           (< [1, 2, 3] >)
print(c)           (< [1, 2, 3] >)
```

Persistent variables share their storage with every variable they are
assigned to, until a mutation forces a copy. This sharing is invisible to
the programmer: independent mutations always produce independent results.

```onyx
!i a: arr[int] <3> = [1, 2, 3]

b = a              (< b shares a's storage >)
c = a              (< c shares a's storage >)
d = a              (< d shares a's storage >)

d[0] = 77          (< a copy is created for d; a, b, c unchanged >)

print(a)           (< [1, 2, 3] >)
print(b)           (< [1, 2, 3] >)
print(c)           (< [1, 2, 3] >)
print(d)           (< [77, 2, 3] >)
```

**Note.** `!i` does not affect mutability. A persistent variable can be
reassigned and mutated exactly like a linear one. `!i` only controls whether
the variable is consumed by transformations.

### Explicit copy

For a linear variable that must survive one specific operation without being
declared persistent for its entire lifetime, `copy(expression)` produces an
independent value immediately.

```onyx
a: arr[int] <3> = [1, 2, 3]

b = copy(a)        (< a survives this line; b is independent >)
c = a              (< consumes a here >)

b[0] = 77          (< b is independent of c >)

print(b)           (< [77, 2, 3] >)
print(c)           (< [1, 2, 3] >)
```

`copy` is not required on a variable already declared `!i`, because the
language already guarantees that the original survives. Applying `copy` to
such a variable is permitted, but it forces an immediate physical copy
instead of a shared representation.

### Relation to scope and lifetime

Consumption affects **which names remain valid**, not the lifetime of the
storage itself. When a value is consumed, its storage may be transferred to
the consuming variable, or released when it is no longer referenced. The
compiler chooses the cheapest strategy that preserves the visible semantics.

Scope and lifetime are therefore distinct concerns:

- **Scope** determines name visibility (section *Blocks, scope, and lifetime*).
- **Consumption** determines whether a name remains valid after a transformation.
- **Lifetime** determines how long the underlying storage must remain alive.

A value can outlive its declaring scope only through an explicit mechanism,
such as a function return or a closure capture.

#### Summary

| Operation | Consumes the operand? |
|---|---|
| `b = a` | yes, unless `a` is `!i` |
| `b = sort(a)`, `b = map(f, a)` | yes, unless `a` is `!i` |
| `b = a + a` (array) | yes, unless `a` is `!i` |
| `print(a)`, `a[0]`, `a.length`, `a.shape` | no |
| `sum(a)`, `first(a)`, `product(a)` | no |
| `b = copy(a)` | no |
| numeric scalars (`int`, `real`, `complex`, `quaternion`, `bool`) | never consumed; always copied |
| `string` | never consumed; compared and concatenated by value |

**To decide:** the exact set of consuming operations, the behavior when a
variable is used twice in the same expression (e.g. `b = a + a`), the
interaction with closures and function returns, and the final name of the
`!i` prefix.

## 8. Open design questions

| # | Topic | Status |
| -- | ----- | ------ |
| 1 | Expression semantics (operators, precedence, function calls, assignment, and evaluation order) | partially defined |
| 2 | Variable mutability (can variables and attributes be reassigned? const vs. mutable?) | partially defined |
| 3 | Scoping and closures (how `lambda` captures variables from its enclosing scope) | partially defined |
| 4 | Functions and closures (named functions, return behavior, recursion, and closure lifetime) | partially defined |
| 5 | Type system (type inference, explicit annotations, generic types, and compatibility rules) | undone |
| 6 | Numeric promotion (conversion rules between `int`, `real`, `complex`, and `quaternion`, including precision and overflow) | partially defined |
| 7 | Error model (whether an error is a type, code/message pair, class, or another value) | undone |
| 8 | `try`/`fallback` semantics (error propagation, rollback behavior, and access to the original error) | undone |
| 9 | Class and object model (constructors, fields, methods, `self`, inheritance, and composition) | undone |
| 10 | `#from`, `#parents`, and `#format` semantics (whether they are keywords, directives, or metadata annotations) | undone |
| 11 | Member terminology (formal definition of “stranger” and the distinction between instance, class, and external variables) | undone |
| 12 | Tensor indexing and slicing (index syntax, bounds checking, negative indices, and slice semantics) | partially defined |
| 13 | Tensor operations (broadcasting, reshaping, reductions, concatenation, and elementwise versus matrix multiplication) | undone |
| 14 | Tensor shape system (compile-time versus runtime dimensions and shape mismatch behavior) | undone |
| 15 | Memory and execution model (layout, ownership, allocation, copying, views, and performance guarantees) | partially defined |
| 16 | Modules and interoperability (imports, packages, foreign-function interfaces, and external libraries) | undone |
| 17 | Diagnostics and tooling (compiler errors, warnings, formatting, testing, debugging, and documentation conventions) | undone |
| 18 | Implementation roadmap (lexer, parser, AST, type checker, interpreter/compiler, standard library, and test suite) | undone |

## 9. Recommended decision order

The following order makes the document progressively implementable:

1. Expression semantics: grammar, precedence, calls, assignments, and evaluation order.
2. Bindings and functions: mutability, scope, closures, named functions, and recursion.
3. Numeric model: default sizes, conversions, precision, overflow, and division by zero.
4. Tensors: indexing, shapes, broadcasting, operations, and memory behaviour.
5. Errors: error values and complete `try` / `fallback` semantics.
6. Objects: constructors, fields, methods, `self`, inheritance, and the exact role of `#from`, `#parents`, `#format`, and strangers.
7. Modules, tooling, standard library, and implementation roadmap.

## 10. Conformance examples to add

For each important rule, add a short program whose result or error is exact.
These examples become the first interpreter/compiler test suite.

```onyx
(< Numeric promotion. Expected type: real; expected value: 2.5. >)
value = 2 + 0.5

(< Closure semantics. The expected result depends on the capture rule. >)
offset = 1
add_offset = lambda (n: int) = n + offset

(< Tensor indexing and layout must be specified before this has a result. >)
matrix: arr[int] <2, 2> = [1, 2, 3, 4]

(< Error model and fallback semantics must be specified. >)
try:
    value = 1 / 0
fallback:
    value = none
```
