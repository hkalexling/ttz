#include "ttz.h"

#include "countries.h"
#include "zones.h"

typedef struct {
  uint8_t size;
  uint16_t offset;
} ttz_token_t;

typedef struct {
  const char *data;
  uint8_t start;
  uint8_t size;
} ttz_string_t;

uint8_t ttz_strlen(const char *s) {
  uint8_t len = 0;
  while (*s++) {
    len++;
  }
  return len;
}

uint8_t ttz_min(uint8_t a, uint8_t b) { return a < b ? a : b; }
uint8_t ttz_max(uint8_t a, uint8_t b) { return a > b ? a : b; }

uint8_t ttz_strcmpn(const char *s1, const char *s2, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    if (s1[i] == 0 || s2[i] == 0) {
      return s1[i] - s2[i];
    }
    if (s1[i] != s2[i]) {
      return s1[i] - s2[i];
    }
  }
  return 0;
}

uint8_t levenstein(const char *s1, const char *s2) {
  uint8_t len1 = ttz_strlen(s1);
  uint8_t len2 = ttz_strlen(s2);

  uint8_t d[len1 + 1][len2 + 1];
  for (uint8_t i = 0; i <= len1; i++) {
    d[i][0] = i;
  }
  for (uint8_t j = 0; j <= len2; j++) {
    d[0][j] = j;
  }
  for (uint8_t i = 1; i <= len1; i++) {
    for (uint8_t j = 1; j <= len2; j++) {
      uint8_t cost = s1[i - 1] == s2[j - 1] ? 0 : 1;
      d[i][j] = ttz_min(d[i - 1][j] + 1,
                        ttz_min(d[i][j - 1] + 1, d[i - 1][j - 1] + cost));
    }
  }

  uint8_t dis = d[len1][len2];
  uint8_t res = ttz_max(len1, len2) - dis;

  return res;
}

uint8_t str_to_codes(const char *s, uint5_t *codes) {
  uint8_t len = ttz_strlen(s);
  int j = 0;
  for (int i = 0; i < len; i++) {
    char c = s[i];
    if (c == 0)
      break;
    if (c == ' ') {
      codes[j++].value = 0;
      continue;
    }
    if (c >= 'A' && c <= 'Z')
      c += 32;
    if (c < 'a' || c > 'z')
      continue;
    codes[j++].value = c - 'a' + 1;
  }
  codes[j].value = 0;
  return j;
}

uint5_data_t uint5_unpack(uint5_array_t array, uint16_t idx) {
  const uint16_t offset = array.offsets[idx];
  uint8_t size = array.offsets[idx + 1] - offset;
  return (uint5_data_t){array.data + offset, offset, size};
}

uint8_data_t uint8_unpack(uint8_array_t array, uint16_t idx) {
  const uint16_t offset = array.offsets[idx];
  uint8_t size = array.offsets[idx + 1] - offset;
  return (uint8_data_t){array.data + offset, offset, size};
}

uint16_data_t uint16_unpack(uint16_array_t array, uint16_t idx) {
  const uint16_t offset = array.offsets[idx];
  uint8_t size = array.offsets[idx + 1] - offset;
  return (uint16_data_t){array.data + offset, offset, size};
}

ttz_token_t get_token(uint16_t idx) {
  uint5_data_t data = uint5_unpack(ttz_tokens, idx);
  return (ttz_token_t){data.size, data.offset};
}

uint8_t render_token(ttz_token_t token, char *buf) {
  const uint5_t *data = ttz_tokens.data + token.offset;
  uint8_t len = token.size;
  for (uint8_t i = 0; i < len; i++) {
    buf[i] = data[i].value + 'a' - 1;
  }
  buf[len] = 0;
  return len + 1;
}

uint8_t tokenize(const char *s) {
  uint8_t i = 0;
  while (1) {
    char c = s[i];
    if (c == ' ' || c == '\'' || c == '(' || c == ')' || c == 0) {
      break;
    }
    i++;
  }
  return i;
}

void clean(char *str) {
  const char *chars = " '()";
  const char *strs[] = {"the", "of", "and", "'s"};

  char *write_ptr = str;

  for (uint8_t i = 0; str[i]; i++) {
    char c = str[i];
    if (c >= 'A' && c <= 'Z')
      c += 32;
    if (c < 'a' || c > 'z')
      continue;

    uint8_t found = 0;
    for (uint8_t j = 0; j < sizeof(strs) / sizeof(strs[0]); j++) {
      uint8_t len = ttz_strlen(strs[j]);
      if (ttz_strcmpn(str + i, strs[j], len) == 0 &&
          (str[i + len] == ' ' || str[i + len] == 0)) {
        i += len - 1;
        found = 1;
        break;
      }
    }
    if (found)
      continue;
    for (uint8_t j = 0; j < ttz_strlen(chars); j++) {
      if (chars[j] == c) {
        found = 1;
        break;
      }
    }
    if (found)
      continue;
    *write_ptr++ = c;
  }

  *write_ptr = 0;
}

uint8_t codes_to_str(const uint5_t *codes, uint8_t len, char *s) {
  uint8_t i = 0;
  for (uint8_t j = 0; j < len; j++) {
    uint8_t c = codes[j].value;
    if (c == 0)
      break;
    s[i++] = c + 'a' - 1;
  }
  s[i] = 0;
  return i;
}

uint8_t tokens_to_str(const ttz_token_t *tokens, uint8_t len, char *s) {
  uint8_t i = 0;
  for (uint8_t j = 0; j < len; j++) {
    ttz_token_t token = tokens[j];
    uint8_t size = render_token(token, s + i);
    i += size - 1;
  }
  s[i] = 0;
  return i;
}

uint8_t data_to_str(const uint8_t *data, uint8_t len, char *s) {
  ttz_token_t tokens[len];
  for (uint8_t i = 0; i < len; i++) {
    tokens[i] = get_token(data[i]);
  }
  return tokens_to_str(tokens, len, s);
}

uint8_t render_uint5(uint5_array_t array, uint16_t idx, char *buf) {
  uint5_data_t data = uint5_unpack(array, idx);
  return codes_to_str(data.data, data.size, buf);
}

uint8_t render_uint8_t(uint8_array_t array, uint16_t idx, char *buf) {
  uint8_data_t data = uint8_unpack(array, idx);
  return data_to_str(data.data, data.size, buf);
}

uint8_t ttz_render_country_name(uint8_t idx, char *buf) {
  return render_uint8_t(ttz_country_name, idx, buf);
}

uint8_t ttz_render_official_name(uint8_t idx, char *buf) {
  return render_uint8_t(ttz_official_name, idx, buf);
}

uint8_t ttz_render_country_code(uint8_t idx, char *buf) {
  const uint5_t *codes = ttz_country_codes[idx];
  return codes_to_str(codes, 2, buf);
}

uint8_t ttz_render_city(uint16_t idx, char *buf) {
  return render_uint5(ttz_cities, idx, buf);
}

uint8_t ttz_render_zone_code(uint16_t idx, char *buf) {
  const uint5_t *codes = ttz_zone_codes[idx];
  return codes_to_str(codes, 5, buf);
}

typedef struct {
  uint16_t idx;
  uint8_t score;
} match_t;

void sort_matches(match_t *matches, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    uint16_t max_idx = i;
    for (uint8_t j = i + 1; j < count; j++) {
      if (matches[j].score > matches[max_idx].score) {
        max_idx = j;
      }
    }
    if (max_idx != i) {
      match_t tmp = matches[i];
      matches[i] = matches[max_idx];
      matches[max_idx] = tmp;
    }
  }
}

ttz_match_t ttz_find_country(char *s, uint8_t count, uint8_t *idx,
                             uint8_t *scores) {
  clean(s);

  char buf[1024];
  uint8_t len;

  match_t matches[TTZ_COUNTRY_COUNT];

  for (uint8_t i = 0; i < TTZ_COUNTRY_COUNT; i++) {
    matches[i].idx = i;

    len = ttz_render_country_name(i, buf);
    if (ttz_strcmpn(s, buf, len) == 0) {
      idx[0] = i;
      return TTZ_EXACT;
    }
    matches[i].score = levenstein(s, buf);

    len = ttz_render_official_name(i, buf);
    if (ttz_strcmpn(s, buf, len) == 0) {
      idx[0] = i;
      return TTZ_EXACT;
    }
    matches[i].score += levenstein(s, buf);

    matches[i].score /= 2;
  }

  sort_matches(matches, TTZ_COUNTRY_COUNT);

  for (uint8_t i = 0; i < count; i++) {
    idx[i] = matches[i].idx;
    scores[i] = matches[i].score;
  }

  return TTZ_SCORE;
}

ttz_match_t ttz_find_city(char *s, uint8_t count, uint16_t *idx,
                          uint8_t *scores) {
  clean(s);

  char buf[1024];
  uint8_t len;

  match_t matches[TTZ_CITY_COUNT];

  for (uint16_t i = 0; i < TTZ_CITY_COUNT; i++) {
    matches[i].idx = i;

    len = ttz_render_city(i, buf);
    if (ttz_strcmpn(s, buf, len) == 0) {
      idx[0] = i;
      return TTZ_EXACT;
    }
    matches[i].score = levenstein(s, buf);
  }

  sort_matches(matches, TTZ_CITY_COUNT);

  for (uint8_t i = 0; i < count; i++) {
    idx[i] = matches[i].idx;
    scores[i] = matches[i].score;
  }

  return TTZ_SCORE;
}

ttz_match_t ttz_find_country_code(char *s, uint8_t *idx) {
  clean(s);

  char buf[1024];
  uint8_t len;

  if (ttz_strlen(s) != 2)
    return TTZ_NONE;

  for (uint8_t i = 0; i < TTZ_COUNTRY_COUNT; i++) {
    len = ttz_render_country_code(i, buf);
    if (ttz_strcmpn(s, buf, len) == 0) {
      *idx = i;
      return TTZ_EXACT;
    }
  }

  return TTZ_NONE;
}

ttz_match_t ttz_find_zone_code(char *s, uint16_t *idx) {
  clean(s);

  char buf[1024];
  uint8_t len;

  if (ttz_strlen(s) > 5)
    return TTZ_NONE;

  for (uint16_t i = 0; i < TTZ_CODE_COUNT; i++) {
    len = ttz_render_zone_code(i, buf);
    if (ttz_strcmpn(s, buf, len) == 0) {
      *idx = i;
      return TTZ_EXACT;
    }
  }

  return TTZ_NONE;
}

ttz_error_t ttz_offset_by_zone_code(uint16_t idx, int64_t timestamp,
                                    int *offset) {
  uint8_t matched = 0;
  int _offset;

  if (idx >= TTZ_ZONE_COUNT)
    return TTZ_INVALID_INDEX;

  for (uint16_t i = 0; i < TTZ_ZONE_COUNT; i++) {
    ttz_zone_t zone = ttz_zones[i];
    if (zone.zone != idx)
      continue;
    if (zone.start < timestamp) {
      _offset = zone.offset;
      matched = 1;
      continue;
    }
    break;
  }
  if (matched) {
    *offset = _offset;
    return TTZ_SUCCESS;
  }
  return TTZ_NO_MATCH;
}

ttz_error_t ttz_offset_by_city(uint16_t idx, int64_t timestamp, int *offset) {
  uint8_t matched = 0;
  int _offset;

  if (idx >= TTZ_CITY_COUNT)
    return TTZ_INVALID_INDEX;

  for (uint16_t i = 0; i < TTZ_ZONE_COUNT; i++) {
    ttz_zone_t zone = ttz_zones[i];
    if (zone.city != idx)
      continue;
    if (zone.start < timestamp) {
      _offset = zone.offset;
      matched = 1;
      continue;
    }
    break;
  }
  if (matched) {
    *offset = _offset;
    return TTZ_SUCCESS;
  }
  return TTZ_NO_MATCH;
}

ttz_error_t ttz_offset_by_country(uint8_t idx, int64_t timestamp, int *offset) {
  int city = -1;
  int _offset;

  if (idx >= TTZ_COUNTRY_COUNT)
    return TTZ_INVALID_INDEX;

  for (uint16_t i = 0; i < TTZ_ZONE_COUNT; i++) {
    ttz_zone_t zone = ttz_zones[i];
    if (zone.country_code != idx)
      continue;
    if (zone.start < timestamp) {
      if (city != -1 && city != zone.city)
        return TTZ_MULTI_ZONES;
      city = zone.city;
      _offset = zone.offset;
      continue;
    }
  }
  if (city == -1)
    return TTZ_NO_MATCH;

  *offset = _offset;
  return TTZ_SUCCESS;
}
