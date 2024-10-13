#ifndef TTZ_H
#define TTZ_H

#include "stdint.h"

typedef enum {
  TTZ_NONE,  ///< No match found
  TTZ_EXACT, ///< We found an exact match
  TTZ_SCORE, ///< No exact match, so we return a list of matches ranked by
             ///< scores
} ttz_match_t;

typedef enum {
  TTZ_SUCCESS,       ///< No error
  TTZ_NO_MATCH,      ///< No match found
  TTZ_INVALID_INDEX, ///< Out of bounds index provided
  TTZ_MULTI_ZONES,   ///< Multiple zones found for the given country
} ttz_error_t;

/**
 * Render the country name for a given country index.
 *
 * @param[in] idx The country index.
 * @param[out] buf The buffer to write the country name to.
 * @return The length of the country name.
 */
uint8_t ttz_render_country_name(uint8_t idx, char *buf);

/**
 * Render the official name for a given country index.
 *
 * @param[in] idx The country index.
 * @param[out] buf The buffer to write the official name to.
 * @return The length of the official name.
 */
uint8_t ttz_render_official_name(uint8_t idx, char *buf);

/**
 * Render the country code for a given country index.
 *
 * @param[in] idx The country index.
 * @param[out] buf The buffer to write the country code to.
 * @return The length of the country code.
 */
uint8_t ttz_render_country_code(uint8_t idx, char *buf);

/**
 * Render the city name for a given city index.
 *
 * @param[in] idx The city index.
 * @param[out] buf The buffer to write the city name to.
 * @return The length of the city name.
 */
uint8_t ttz_render_city(uint16_t idx, char *buf);

/**
 * Render the zone code for a given zone index.
 *
 * @param[in] idx The zone index.
 * @param[out] buf The buffer to write the zone code to.
 * @return The length of the zone code.
 */
uint8_t ttz_render_zone_code(uint16_t idx, char *buf);

/**
 * Search for a country code from a query string
 *
 * @param[in] s The query string.
 * @param[in] count In case of non-exact match, the number of matches to return.
 * @param[out] idx The country indeces of the matches.
 * @param[out] scores The scores of the matches.
 * @return The match type.
 */
ttz_match_t ttz_find_country(char *s, uint8_t count, uint8_t *idx,
                             uint8_t *scores);

/**
 * Search for a city from a query string
 *
 * @param[in] s The query string.
 * @param[in] count In case of non-exact match, the number of matches to return.
 * @param[out] idx The city indeces of the matches.
 * @param[out] scores The scores of the matches.
 * @return The match type.
 */
ttz_match_t ttz_find_city(char *s, uint8_t count, uint16_t *idx,
                          uint8_t *scores);

/**
 * Search for a country code from a query string using exact match.
 *
 * @param[in] s The query string.
 * @param[out] idx The country index.
 * @return The match type.
 */
ttz_match_t ttz_find_country_code(char *s, uint8_t *idx);

/**
 * Search for a zone code from a query string using exact match.
 *
 * @param[in] s The query string.
 * @param[out] idx The zone index.
 * @return The match type.
 */
ttz_match_t ttz_find_zone_code(char *s, uint16_t *idx);

/**
 * Get the offset for a given zone index and timestamp.
 *
 * @param[in] idx The zone index.
 * @param[in] timestamp The timestamp.
 * @param[out] offset The offset.
 * @return The error code.
 */
ttz_error_t ttz_offset_by_zone_code(uint16_t idx, int64_t timestamp,
                                    int *offset);

/**
 * Get the offset for a given city index and timestamp.
 *
 * @param[in] idx The city index.
 * @param[in] timestamp The timestamp.
 * @param[out] offset The offset.
 * @return The error code.
 */
ttz_error_t ttz_offset_by_city(uint16_t idx, int64_t timestamp, int *offset);

/**
 * Get the offset for a given country index and timestamp.
 *
 * @param[in] idx The country index.
 * @param[in] timestamp The timestamp.
 * @param[out] offset The offset.
 * @return The error code.
 */
ttz_error_t ttz_offset_by_country(uint8_t idx, int64_t timestamp, int *offset);

#endif
