#include <ngl_hash.h>

def_struct(sip_hash_state, {
  uint64_t v[4];
})

#define SIP_ROT64(var, pos) \
  var = (((var) <<(pos)) |((var) >>(64 -(pos))))


inline static void
siphash_round(sip_hash_state * s, unsigned int rounds) {
  while (rounds-- != 0) {
    s->v[0] += s->v[1];
    SIP_ROT64(s->v[1], 13);
    s->v[1] ^= s->v[0];
    SIP_ROT64(s->v[0], 32);

    s->v[2] += s->v[3];
    SIP_ROT64(s->v[3], 16);
    s->v[3] ^= s->v[2];

    s->v[0] += s->v[3];
    SIP_ROT64(s->v[3], 21);
    s->v[3] ^= s->v[0];

    s->v[2] += s->v[1];
    SIP_ROT64(s->v[1], 17);
    s->v[1] ^= s->v[2];
    SIP_ROT64(s->v[2], 32);
  }
}

#undef SIP_ROT64

inline uint64_t
siphash_24(const ngl_siphash_key key, const uint8_t * msg, const size_t len) {
  sip_hash_state s;

  uint64_t mi, m_last = 0;
  size_t i = 0;
  size_t ngl_ords = len & ~7;
  s.v[0] = key.k[0] ^ 0x736f6d6570736575ull;
  s.v[1] = key.k[1] ^ 0x646f72616e646f6dull;
  s.v[2] = key.k[0] ^ 0x6c7967656e657261ull;
  s.v[3] = key.k[1] ^ 0x7465646279746573ull;

  /* Loop for all but last seven bytes. */
  for (i = 0; i < ngl_ords; i += 8) {
    mi = *(uint64_t *) (msg + i);
    s.v[3] ^= mi;
    siphash_round(&s, 2);
    s.v[0] ^= mi;
  }

  m_last = (uint64_t) (len & 0xff) << 56;

#define LOAD_BYTE(var, m, start, idx) \
  case idx: var |= (uint64_t)m[ i + idx - 1 ] <<((idx - 1) * 8);

  switch (len - ngl_ords) {
    /* Load all remaining bytes into m_last. */
    LOAD_BYTE(m_last, msg, i, 7);
    LOAD_BYTE(m_last, msg, i, 6);
    LOAD_BYTE(m_last, msg, i, 5);
    LOAD_BYTE(m_last, msg, i, 4);
    LOAD_BYTE(m_last, msg, i, 3);
    LOAD_BYTE(m_last, msg, i, 2);
    LOAD_BYTE(m_last, msg, i, 1);
  case 0:
  default:
    {
    }
  }

  s.v[3] ^= m_last;
  siphash_round(&s, 2);
  s.v[0] ^= m_last;

  s.v[2] ^= 0xff;

  siphash_round(&s, 4);
  return s.v[0] ^ s.v[1] ^ s.v[2] ^ s.v[3];
}

ngl_siphash_key ngl_hash_key = {{0xe8f35937acefffaa, 0x331e89da1849403f}} END

#undef SIP_ROT64
#undef LOAD_BYTE

ngl_hash
ngl_hash_ngl_val(ngl_val val) {
  return siphash_24(ngl_hash_key, (uint8_t *) & val, sizeof(val));
}

ngl_int
ngl_compare_ngl_val(ngl_val a, ngl_val b) {
  return a.integer - b.integer;
}
