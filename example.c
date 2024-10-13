#include "src/ttz.h"

#include "stdio.h"
#include "string.h"
#include "time.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: %s <input>\n", argv[0]);
    return 1;
  }

  long now = time(NULL);
  printf("current time: %ld\n", now);

  ttz_match_t match;
  ttz_error_t err;
  int offset;

  uint8_t idx1 = 0;
  char s1[1024];
  strcpy(s1, argv[1]);
  match = ttz_find_country_code(s1, &idx1);
  if (match > 0) {
    ttz_render_country_code(idx1, s1);
    printf("matched country code: %s\n", s1);

    err = ttz_offset_by_country(idx1, now, &offset);
    if (err == TTZ_MULTI_ZONES) {
      printf("multiple zones found for country %s\n", s1);
    } else if (err == TTZ_SUCCESS) {
      printf("offset for country %s: %d\n", s1, offset);
    }
  }

  uint16_t idx2 = 0;
  strcpy(s1, argv[1]);
  match = ttz_find_zone_code(s1, &idx2);
  if (match > 0) {
    ttz_render_zone_code(idx2, s1);
    printf("matched zone code: %s\n", s1);

    ttz_offset_by_zone_code(idx2, now, &offset);
    printf("offset for zone %s: %d\n", s1, offset);
  }

  uint16_t cities[2];
  uint8_t city_scores[2];
  strcpy(s1, argv[1]);
  match = ttz_find_city(s1, 2, cities, city_scores);
  if (match <= 0) {
    printf("no matched city\n");
  } else if (match == 1) {
    ttz_render_city(cities[0], s1);
    printf("exact matched city: %s\n", s1);

    ttz_offset_by_city(cities[0], now, &offset);
    printf("offset for city %s: %d\n", s1, offset);
  } else {
    for (int i = 0; i < 2; i++) {
      ttz_render_city(cities[i], s1);
      ttz_offset_by_city(cities[i], now, &offset);
      printf("matched city: %s score: %d offset: %d\n", s1, city_scores[i],
             offset);
    }
  }

  uint8_t countries[2];
  uint8_t country_scores[2];
  strcpy(s1, argv[1]);
  match = ttz_find_country(s1, 2, countries, country_scores);
  if (match <= 0) {
    printf("no matched country\n");
  } else if (match == 1) {
    ttz_render_country_name(countries[0], s1);
    printf("exact matched country: %s\n", s1);

    err = ttz_offset_by_country(countries[0], now, &offset);
    if (err == TTZ_MULTI_ZONES) {
      printf("multiple zones found for country %s\n", s1);
    } else if (err == TTZ_SUCCESS) {
      printf("offset for country %s: %d\n", s1, offset);
    }
  } else {
    for (int i = 0; i < 2; i++) {
      ttz_render_country_name(countries[i], s1);
      err = ttz_offset_by_country(countries[i], now, &offset);
      char offset_str[1024];
      if (err == TTZ_SUCCESS) {
        sprintf(offset_str, "%d", offset);
      } else {
        strcpy(offset_str, "multiple zones");
      }
      printf("matched country: %s score: %d offset: %s\n", s1,
             country_scores[i], offset_str);
    }
  }

  return 0;
}
