/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

/** @file string.h API related to Pure64's string functions. */

#ifndef PURE64_STRING_H
#define PURE64_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

/** Set a range of memory to a certain value.
 * @param dst The destination address.
 * @param value The value to set each byte in the
 * memory section. This value is truncated to an 8-bit,
 * unsigned value.
 * @param size The number of bytes to set in the
 * memory section.
 * */

void pure64_memset(void *dst, int value, unsigned long int size);

/** Copy a range of memory from one location
 * to the other. This function does not check
 * for overlapping ranges.
 * @param dst The destination memory section.
 * @param src The memory section to copy.
 * @param size The number of bytes to copy.
 * */

void pure64_memcpy(void *dst, const void *src, unsigned long int size);

/** Calculate the length of a null-terminated string.
 * @param str The string to calculate the
 * length of. This must be null-terminated.
 * @returns The size of the string, not including
 * the null-terminator, in bytes.
 * */

unsigned long int pure64_strlen(const char *str);

/** Compares two strings.
 * @param a The first string to compare.
 * @param b The second string to compare.
 * @returns If a byte at @p a is greater the
 * the byte at @p b, then one is returned.
 * If they are equal then zero is returned.
 * Otherwise, a negative one is returned.
 * */

int pure64_strcmp(const char *a, const char *b);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* PURE64_STRING_H */
