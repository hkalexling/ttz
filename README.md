# TinyTZ

TinyTZ (ttz) is a lightweight timezone library for C designed to be both **tiny** and **portalble**. It avoids heap allocations, has no external dependencies, and does not require system timezone data files. The default library size is approximately 300KB, with the flexibility to adjust the amount of included timezone data.

## Building

To build the library, simply run `make`. You can customize the build by passing the following variables:

- `COUNTRY_COUNT`: Number of countries to include (default: 100).
- `MIN_YEAR`: The earliest year of timezone transition data to include (default: 1980).
- `MAX_YEAR`: The latest year of transition data to include (default: 2030).

Example: `make COUNTRY_COUNT=50 MIN_YEAR=2000 MAX_YEAR=2020`

## Usage

Documentation is available at: https://hkalexling.github.io/ttz/ttz_8h.html.

For a quick demonstration, refer to `example.c`.

## How It Works

TinyTZ extracts the top `COUNTRY_COUNT` countries from Wikipedia, ranked by population and GDP. It then filters the IANA timezone database to include transitions between `MIN_YEAR` and `MAX_YEAR` for these countries. This data is packed into the source files `countries.c` and `zones.c`, which are compiled into the library.
