#!/usr/bin/env python
import requests
from bs4 import BeautifulSoup
import pandas as pd
from pathlib import Path

population_url = "https://en.wikipedia.org/wiki/List_of_countries_and_dependencies_by_population"
gdp_url = "https://en.wikipedia.org/wiki/List_of_countries_by_GDP_(nominal)"
iso_url = "https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes"

def get_population_data():
    response = requests.get(population_url)
    soup = BeautifulSoup(response.content, 'lxml')
    table = soup.find('table', {'class': ['wikitable', 'sortable']})
    rows = table.find_all('tr')

    population_dict = {}

    for row in rows[1:]:
        columns = row.find_all('td')
        country_link_a = columns[1].find('a')
        if country_link_a:
            country_name = country_link_a.get('title')
            population_count = columns[2].get_text(strip=True).replace(',', '')
            population_dict[country_name] = population_count

    return population_dict

def get_gdp_data():
    gdp_response = requests.get(gdp_url)
    gdp_soup = BeautifulSoup(gdp_response.content, 'lxml')
    gdp_table = gdp_soup.find('table', {'class': 'wikitable'})
    gdp_rows = gdp_table.find_all('tr')

    gdp_dict = {}

    for row in gdp_rows[1:]:
        columns = row.find_all('td')
        if not len(columns):
            continue
        a_tag = columns[0].find('a')
        if not a_tag:
            continue
        country_name = a_tag.get_text(strip=True)
        if country_name:
            gdp_value = columns[1].get_text(strip=True).replace(',', '')
            gdp_dict[country_name] = gdp_value

    return gdp_dict

def get_iso_codes():
    iso_response = requests.get(iso_url)
    iso_soup = BeautifulSoup(iso_response.content, 'lxml')
    iso_table = iso_soup.find('table', {'class': 'wikitable'})
    iso_rows = iso_table.find_all('tr')

    iso_dict = {}

    for row in iso_rows[1:]:
        columns = row.find_all('td')
        if len(columns) < 4:
            continue
        country_name = columns[0].find_all('a')[1].get('title')
        if country_name:
            official_name = columns[1].find('a').get_text(strip=True)
            country_code = columns[3].get_text(strip=True)
            iso_dict[country_name] = [country_code, official_name]

    return iso_dict

if __name__ == '__main__':
    population_data = get_population_data()
    gdp_data = get_gdp_data()
    iso_data = get_iso_codes()

    countries = []
    official_names = []
    country_codes = []
    populations = []
    gdps = []

    for country_name, data in iso_data.items():
        code, official_name = data
        population = population_data.get(country_name, 'N/A')
        gdp = gdp_data.get(country_name, 'N/A')

        countries.append(country_name)
        official_names.append(official_name)
        country_codes.append(code)
        populations.append(population)
        gdps.append(gdp)

    df = pd.DataFrame({
        'Country': countries,
        'Country Code': country_codes,
        'Official Name': official_names,
        'Population': populations,
        'GDP': gdps
    })

    df['GDP'] = pd.to_numeric(df['GDP'], errors='coerce').fillna(0)
    df['Population'] = pd.to_numeric(df['Population'], errors='coerce').fillna(0)

    df['Normalized GDP'] = df['GDP'] / df['GDP'].max()
    df['Normalized Population'] = df['Population'] / df['Population'].max()
    df['Score'] = (df['Normalized GDP'] + df['Normalized Population']) / 2

    df = df.sort_values(by='Score', ascending=False)

    output = Path(__file__).parent.parent / 'resources' / 'countries.csv'

    df.to_csv(output, index=False, header=False)

    print(f"Processed {len(df)} countries")
