#include <stddef.h>
#include <stdint.h>

void *memcpy_aligned(void *dest, const void *src, size_t n) {
  size_t i;
  uint32_t *d = dest;
  const uint32_t *s = src;
  n /= sizeof(*d);

  for (i = 0; i < n; i++)
    d[i] = s[i];

  return d;
}

void *memmove_aligned(void *dest, const void *src, size_t n) {
  return memcpy_aligned(dest, src, n);
}

void *memset_aligned(void *dst0, int val, size_t length) {
  uint32_t *ptr = dst0;
  length /= sizeof(*ptr);

  while (length--)
    *ptr++ = val;
  return dst0;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
  unsigned char u1, u2;

  if (!s1 && !s2)
    return 0;
  if (!s1)
    return -1;
  if (!s2)
    return 1;

  for ( ; n-- ; s1++, s2++) {
    u1 = * (unsigned char *) s1;
    u2 = * (unsigned char *) s2;
    if ( u1 != u2)
      return (u1-u2);
  }
  return 0;
}
