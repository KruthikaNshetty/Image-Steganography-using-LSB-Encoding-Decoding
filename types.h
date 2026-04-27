#ifndef TYPES_H
#define TYPES_H

/* User defined types */
typedef unsigned int uint;  // Unsigned integer alias

/* Status will be used in fn. return type */
typedef enum
{
    e_failure, // Operation failed
    e_success // Operation successful
} Status;

typedef enum
{
    e_encode, // Encode operation
    e_decode,  // Decode operation
    e_unsupported  // Unsupported operation
} OperationType;

#endif
