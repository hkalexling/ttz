#!/usr/bin/env python
import os
import csv
from pathlib import Path
from datetime import datetime
from dataclasses import dataclass

country_count = int(os.environ.get('COUNTRY_COUNT') or 100)
min_year = int(os.environ.get('MIN_YEAR') or 1980)
max_year = int(os.environ.get('MAX_YEAR') or 2030)

zone_csv = Path(__file__).parent.parent / 'vendor' / 'time_zone.csv'
country_csv = Path(__file__).parent.parent / 'resources' / 'countries.csv'
out_h = Path(__file__).parent.parent / 'src' / 'zones.h'
out_c = Path(__file__).parent.parent / 'src' / 'zones.c'

with open(country_csv, 'r') as f:
    reader = csv.reader(f)
    country_codes = [next(reader)[1] for _ in range(country_count)]

@dataclass
class Zone:
    name: str
    country_code: str
    zone_code: str
    start: int
    offset: int
    is_dst: int
    year: int

    def __init__(self, row):
        self.name = row[0]
        self.country_code = row[1]
        self.zone_code = row[2]
        self.start = int(row[3])
        self.offset = int(row[4])
        self.is_dst = int(row[5])
        self.year = datetime.fromtimestamp(self.start).year

@dataclass
class PackedZone:
    city: int
    country: int
    zone: int
    start: int
    offset: int
    is_dst: int

zones = []
name_zones = {}

with open(zone_csv, 'r') as f:
    reader = csv.reader(f)
    for row in reader:
        country_code = row[1]
        zone_code = row[2]
        start = row[3]

        if not country_code in country_codes:
            continue

        if not zone_code.isalpha():
            continue

        zone = Zone(row)
        if zone.year > int(max_year):
            continue

        zone_name = row[0]
        if not zone_name in name_zones:
            name_zones[zone_name] = []
        name_zones[zone_name].append(zone)

for nzones in name_zones.values():
    zone_max_year = max(nzones, key=lambda z: z.year).year
    if zone_max_year >= int(min_year):
        nzones = [z for z in nzones if z.year >= int(min_year)]
    else:
        nzones = [nzones[-1]]
    zones += nzones

def str_to_codes(s):
    s = s.lower()
    codes = []

    for c in s:
        value = ord(c) - ord('a') + 1
        if value > 26 or value < 1:
            continue
        codes.append(value)

    return codes

cities = []
zone_codes = []

def pack_zone(zone):
    city_name = zone.name.split('/')[-1].replace('_', '').encode('ascii', 'ignore').decode('ascii').lower()

    try:
        city_idx = cities.index(city_name)
    except ValueError:
        cities.append(city_name)
        city_idx = len(cities) - 1

    country_idx = country_codes.index(zone.country_code)

    try:
        zone_idx = zone_codes.index(zone.zone_code)
    except ValueError:
        zone_codes.append(zone.zone_code)
        zone_idx = len(zone_codes) - 1

    return PackedZone(city_idx, country_idx, zone_idx, zone.start, zone.offset, zone.is_dst)

packed_zones = [pack_zone(zone) for zone in zones]

h_str = f'''#include "countries.h"

#define TTZ_CITY_COUNT {len(cities)}
#define TTZ_CODE_COUNT {len(zone_codes)}
#define TTZ_ZONE_COUNT {len(zones)}

typedef struct {{
  uint8_t city;
  uint8_t country_code;
  uint8_t zone;
  int64_t start;
  int offset;
  uint8_t is_dst : 1;
}} ttz_zone_t;

extern const uint5_array_t ttz_cities;
extern const uint5_t ttz_zone_codes[TTZ_CODE_COUNT][5];
extern const ttz_zone_t ttz_zones[TTZ_ZONE_COUNT];
'''

city_codes = [str_to_codes(c) for c in cities]
city_sizes = [len(c) for c in city_codes]
city_offsets = [sum(city_sizes[:i]) for i in range(len(city_sizes) + 1)]
city_array_size = sum(city_sizes)

city_str = f'const uint5_t city_data[{city_array_size}] = {{\n'
for i, codes in enumerate(city_codes):
    city_str += f'  {", ".join(map(lambda c: f"{{{c}}}", codes))}, // {cities[i]}\n'
city_str += '};\n\n'

city_str += f'const uint16_t city_offsets[{len(cities) + 1}] = {{\n'
for i, offset in enumerate(city_offsets):
    comment = cities[i] if i < len(cities) else "end"
    city_str += f'  {offset}, // {comment}\n'
city_str += '};\n\n'

city_str += 'const uint5_array_t ttz_cities = {\n'
city_str += f'  {city_array_size}, // size\n'
city_str += '  city_data, // data\n'
city_str += '  city_offsets // offsets\n'
city_str += '};\n'

code_str = 'const uint5_t ttz_zone_codes[TTZ_CODE_COUNT][5] = {\n'
for i, code in enumerate(zone_codes):
    codes = str_to_codes(code)
    codes += [0] * (5 - len(codes))
    code_str += f'  {{{", ".join(map(lambda c: f"{{{c}}}", codes))}}}, // {code}\n'
code_str += '};\n'

zone_str = 'const ttz_zone_t ttz_zones[TTZ_ZONE_COUNT] = {\n'
for zone in packed_zones:
    zone_str += f'  {{ {zone.city}, {zone.country}, {zone.zone}, {zone.start}, {zone.offset}, {zone.is_dst} }},\n'
zone_str += '};\n'

c_str = f'''#include "zones.h"

{city_str}
{code_str}
{zone_str}
'''

out_h.write_text(h_str)
print(f'{out_h} generated')

out_c.write_text(c_str)
print(f'{out_c} generated')
