#!/usr/bin/env python
import re
import os
import csv
from pathlib import Path

count = int(os.environ.get('COUNTRY_COUNT') or 100)

in_csv = Path(__file__).parent.parent / 'resources' / 'countries.csv'
out_h = Path(__file__).parent.parent / 'src' / 'countries.h'
out_c = Path(__file__).parent.parent / 'src' / 'countries.c'

with open(in_csv, 'r') as f:
    reader = csv.reader(f)
    rows = [next(reader)[:3] for _ in range(count)]

def str_to_codes(s):
    s = s.lower()
    codes = []

    for c in s:
        value = ord(c) - ord('a') + 1
        if value > 26 or value < 1:
            continue
        codes.append(value)

    return codes

def tokenize(s):
    s = s.encode('ascii', 'ignore').decode('ascii').lower()
    str_tokens = [t for t in re.split(r'\s+|\(|\)|\band\b|\bthe\b|\bof\b|\'s', s) if t]

    idx_list = []
    for t in str_tokens:
        try:
            idx = tokens.index(t)
        except ValueError:
            tokens.append(t)
            idx = len(tokens) - 1
        idx_list.append(idx)

    return idx_list

def generate_token_strs(tokens):
    token_codes = [str_to_codes(t) for t in tokens]
    token_sizes = [len(c) for c in token_codes]
    token_offsets = [sum(token_sizes[:i]) for i in range(len(token_sizes) + 1)]
    token_array_size = sum(token_sizes)

    tokens_h_str = 'extern const uint5_array_t ttz_tokens;'

    tokens_str = f'const uint5_t tokens_data[{token_array_size}] = {{\n'
    for i, codes in enumerate(token_codes):
        tokens_str += f'  {", ".join(map(lambda c: f"{{{c}}}", codes))}, // {tokens[i]}\n'
    tokens_str += '};\n\n'

    tokens_str += f'const uint16_t tokens_offsets[{len(tokens) + 1}] = {{\n'
    for i, offset in enumerate(token_offsets):
        comment = tokens[i] if i < len(tokens) else "end"
        tokens_str += f'  {offset}, // {comment}\n'
    tokens_str += '};\n\n'

    tokens_str += 'const uint5_array_t ttz_tokens = {\n'
    tokens_str += f'  {sum(token_sizes)}, // size\n'
    tokens_str += '  tokens_data, // data\n'
    tokens_str += '  tokens_offsets // offsets\n'
    tokens_str += '};\n'

    return tokens_h_str, tokens_str, token_array_size, max(token_sizes)

def generate_name_strs(countries, key):
    name_codes = [country[key]['codes'] for country in countries]
    name_sizes = [len(c) for c in name_codes]
    name_offsets = [sum(name_sizes[:i]) for i in range(len(name_sizes) + 1)]
    name_array_size = sum(name_sizes)

    name_h_str = f'extern const uint8_array_t ttz_{key};'

    name_str = f'const uint8_t {key}_data[{name_array_size}] = {{\n'
    for i, codes in enumerate(name_codes):
        name_str += f'  {", ".join(map(str, codes))}, // {countries[i][key]["str"]}\n'
    name_str += '};\n\n'

    name_str += f'const uint16_t {key}_offsets[{len(countries) + 1}] = {{\n'
    for i, offset in enumerate(name_offsets):
        comment = countries[i][key]["str"] if i < len(countries) else "end"
        name_str += f'  {offset}, // {comment}\n'
    name_str += '};\n\n'

    name_str += f'const uint8_array_t ttz_{key} = {{\n'
    name_str += f'  {sum(name_sizes)}, // size\n'
    name_str += f'  {key}_data, // data\n'
    name_str += f'  {key}_offsets // offsets\n'
    name_str += '};\n'

    return name_h_str, name_str, name_array_size, max(name_sizes)

def code_gen(tokens, countries):
    code_h_str = 'extern const uint5_t ttz_country_codes[TTZ_COUNTRY_COUNT][2];'
    code_str = 'const uint5_t ttz_country_codes[TTZ_COUNTRY_COUNT][2] = {\n'
    for i, country in enumerate(countries):
        code_str += f'  {{ {", ".join(map(lambda c: f"{{{c}}}", country["code"]["codes"]))} }}, // {country["code"]["str"]}\n'
    code_str += '};\n'

    tokens_h_str, tokens_str, token_array_size, max_token_size = generate_token_strs(tokens)
    name_h_str, name_str, name_array_size, max_name_size = generate_name_strs(countries, 'country_name')
    official_h_str, official_str, official_array_size, max_official_size = generate_name_strs(countries, 'official_name')

    h_str = f'''#ifndef COUNTRIES_H
#define COUNTRIES_H

#include "stdint.h"

typedef struct {{
  uint8_t value : 5;
}} uint5_t;

typedef struct {{
  const uint16_t size;
  const uint5_t* data;
  const uint16_t* offsets;
}} uint5_array_t;

typedef struct {{
  const uint16_t size;
  const uint8_t* data;
  const uint16_t* offsets;
}} uint8_array_t;

typedef struct {{
  const uint16_t size;
  const uint16_t* data;
  const uint16_t* offsets;
}} uint16_array_t;

typedef struct {{
  const uint5_t* data;
  const uint16_t offset;
  const uint8_t size;
}} uint5_data_t;

typedef struct {{
  const uint8_t* data;
  const uint16_t offset;
  const uint8_t size;
}} uint8_data_t;

typedef struct {{
  const uint16_t* data;
  const uint16_t offset;
  const uint8_t size;
}} uint16_data_t;

#define TTZ_COUNTRY_COUNT {len(countries)}

#define TTZ_TOKEN_COUNT {len(tokens)}
#define TTZ_MAX_TOKEN_SIZE {max_token_size}
#define TTZ_TOKEN_ARRAY_SIZE {token_array_size}

#define TTZ_MAX_COUNTRY_NAME_SIZE {max_name_size}
#define TTZ_COUNTRY_NAME_ARRAY_SIZE {name_array_size}

#define TTZ_MAX_OFFICIAL_NAME_SIZE {max_official_size}
#define TTZ_OFFICIAL_NAME_ARRAY_SIZE {official_array_size}

{code_h_str}
{tokens_h_str}
{name_h_str}
{official_h_str}

#endif
'''

    c_str = f'''#include "countries.h"

const char char_table[27] = {{'\\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                             'i',  'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q',
                             'r',  's', 't', 'u', 'v', 'w', 'x', 'y', 'z'}};

{code_str}
{tokens_str}
{name_str}
{official_str}
'''

    return h_str, c_str

tokens = []
countries = []

for row in rows:
    name, code, official_name = row

    name_tokens = tokenize(name)
    official_name_tokens = tokenize(official_name)
    codes = str_to_codes(code)

    if len(codes) != 2:
        continue

    countries.append({ 
                      'country_name': {'str': name, 'codes': name_tokens}, 
                      'code': {'str': code, 'codes': codes},
                      'official_name': { 'str': official_name, 'codes': official_name_tokens }
                      })

h_str, c_str = code_gen(tokens, countries)

with open(out_h, 'w') as f:
    f.write(h_str)
    print(f'{out_h} generated')

with open(out_c, 'w') as f:
    f.write(c_str)
    print(f'{out_c} generated')
