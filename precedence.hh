enum Precedence {
    PREC_NONE,         // The lowest precedence, used for non-operators
    PREC_ASSIGNMENT,   // Assignment operators: =, +=, -=, *=, /=
    PREC_OR,           // Logical OR operator: or
    PREC_AND,          // Logical AND operator: and
    PREC_EQUALITY,     // Equality operators: ==, !=
    PREC_COMPARISON,   // Comparison operators: <, >, <=, >=
    PREC_TERM,         // Addition and subtraction: +, -
    PREC_FACTOR,       // Multiplication and division: *, /, %
    PREC_UNARY,        // Unary operators: !, -
    PREC_CALL,         // Function or method call: . ()
    PREC_PRIMARY       // The highest precedence, used for primary expressions
};