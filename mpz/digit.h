/**
 * @file digit.c
 *
 * @brief Library of functions that operate on arrays of digits.
 *
 * Arrays of digits are assumed to be in little endian order.
 *
 * @author David Matlack (dmatlack)
 */
#ifndef __418_DIGIT_H__
#define __418_DIGIT_H__

#ifndef __CUDACC__ /* when compiling with gcc... */

#define __device__
#define __host__

#include <string.h>

#endif /* __CUDACC__ */

typedef unsigned char digit_t;

#define DIGIT_BASE 10

/**
 * @brief Return true (non-zero) if all of the digits in the digits array
 * are zero (and thus the corresponding number is zero.
 */
__device__ __host__ int digits_is_zero(digit_t *digits, 
                                       unsigned num_digits) {
  unsigned i;

  for (i = 0; i < num_digits; i++) {
    if (digits[i] != 0) return 0;
  }
  return 1;
}

inline __device__ __host__ void clip(unsigned long long value,
                                     digit_t* result, digit_t *carry) {
  *carry  = value / DIGIT_BASE;
  *result = value % DIGIT_BASE;
}

/**
 * @brief Find the result of a + b + carry. Store the resulting carry of this
 * operation back in the carry pointer.
 */
__device__ __host__ digit_t add(digit_t a, digit_t b, 
                                digit_t *carry) {
  unsigned long long tmp = a + b + *carry;
  digit_t result;

  clip(tmp, &result, carry);
  
  return result;
}

/**
 * @brief Compute the multiplication of two digits (plus the carry).
 *
 * e.g If DIGIT_BASE is 10, and the input carry is 0,
 *
 *  3 x 5 = 15 = (product: 5, carry: 1)
 *
 * @return The product (as well as the carry out).
 */
__device__ __host__ digit_t mult(digit_t a, digit_t b, digit_t *carry) {
  unsigned long long tmp = a*b + *carry;
  digit_t result;

  clip(tmp, &result, carry);
  
  return result;
}


/**
 * @brief Add the digit d to the list of digits (with carry)!
 *
 * @return The carry out.
 */
__device__ __host__ digit_t digits_add_across(digit_t *digits, 
                                              unsigned num_digits, digit_t d) {
  digit_t carry = d;
  unsigned i = 0;

  while (carry != 0 && i < num_digits) {
    digits[i] = add(digits[i], 0, &carry);
    i++;
  }

  return carry;
}

/**
 * @brief Perform DIGIT_BASE complement on the array of digits.
 *
 * For example, if DIGIT_BASE is 10, and the digits are 239487, the 10's
 * complement is:
 *                          +--------+
 * 239487 -> 760518 + 1 ->  | 760519 |
 *                          +--------+
 */
__device__ __host__ void digits_complement(digit_t *digits, 
                                           unsigned num_digits) {
  unsigned i;

  // Complement each digit by subtracting it from BASE-1
  for (i = 0; i < num_digits; i++) {
    digits[i] = (DIGIT_BASE - 1) - digits[i];
  }

  // Add 1
  digits_add_across(digits, num_digits, 1);

}

/**
 * @brief Compute sum = op1 + op2.
 *
 * @return The carry-out of the addition (0 if there is none).
 */
__device__ __host__ digit_t digits_add(digit_t *sum, unsigned sum_num_digits, 
                                       digit_t *op1, unsigned op1_num_digits,
                                       digit_t *op2, unsigned op2_num_digits) {
  digit_t carry = 0;
  unsigned i;

  for (i = 0; i < sum_num_digits; i++) {
    digit_t a = (i < op1_num_digits) ? op1[i] : 0;
    digit_t b = (i < op2_num_digits) ? op2[i] : 0;

    sum[i] = add(a, b, &carry);
  }

  return carry;
}


/**
 * @brief Compute product = op1 * op2 using the Long Multiplication
 * (Grade School Multiplication) Aglorithm.
 *
 * @warning It is assumed that op1 and op2 contain num_digits each
 * and product has room for at least 2*num_digits.
 *
 * @return The carry out.
 */
__device__ __host__ void long_multiplication(digit_t *product, 
                                             digit_t *op1, 
                                             digit_t *op2, 
                                             unsigned num_digits) {
  unsigned i, j;
  
  for (i = 0; i < 2*num_digits; i++) {
    product[i] = 0;
  }

  for (i = 0; i < num_digits; i++) {
    for (j = 0; j < num_digits; j++) {
      unsigned k = i + j;
      digit_t carry = 0;
      digit_t prod;

      prod = mult(op2[i], op1[j], &carry);

      digits_add_across(product + k,     2*num_digits - k,     prod);
      digits_add_across(product + k + 1, 2*num_digits - k - 1, carry);
    }
  }

}

__device__ __host__ void karatsuba(void) { 
  //TODO someway somehow someday
}

/**
 * @brief Compute op1 * op2 and store the result in product.
 *
 * @warning It is assumed that op1 and op2 contain num_digits each
 * and product has room for at least 2*num_digits.
 */
__device__ __host__ void digits_mult(digit_t *product, 
                                     digit_t *op1, 
                                     digit_t *op2, 
                                     unsigned num_digits) {
  long_multiplication(product, op1, op2, num_digits);
}

__device__ __host__ char digit_tochar(digit_t d) {
  return '0' + d;
}

__device__ __host__ digit_t digit_fromchar(char c) {
  if (c < '0' || c > '9') {
    c = '0';
  }

  return c - '0';
}

#endif /* __418_DIGIT_H__ */