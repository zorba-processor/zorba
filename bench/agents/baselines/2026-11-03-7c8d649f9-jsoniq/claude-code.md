# Agent benchmark: claude-sonnet-5-5 (backend claude-code, language jsoniq)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 3.7 | 5.7 | 23983 | 1.7500 | 19.4 | 0.95 | 17 | 2.1899 |
| B | 20 | 60 | 1.00 | 2.6 | 4.6 | 15436 | 0.8443 | 13.8 | 0.97 | 18 | 0.9715 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 2.3 | 4.3 | 12863 | 0.0150 | 14.7 | 1.00 | submitted 3 | 0.0197 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8888 | 0.0075 | 13.0 | 1.00 | submitted 3 | 0.0084 |
| t02_magtype_counts | A | 3 | 1.00 | 3.0 | 5.0 | 16685 | 0.0189 | 11.6 | 0.67 | submitted 3 | 0.0236 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 8947 | 0.0057 | 10.4 | 1.00 | submitted 3 | 0.0052 |
| t03_m45_stats | A | 3 | 1.00 | 3.0 | 5.0 | 16568 | 0.0192 | 16.6 | 1.00 | submitted 3 | 0.0241 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8812 | 0.0052 | 10.8 | 1.00 | submitted 3 | 0.0044 |
| t04_busiest_day | A | 3 | 1.00 | 2.7 | 4.7 | 14738 | 0.0167 | 12.9 | 1.00 | submitted 3 | 0.0217 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9084 | 0.0053 | 12.0 | 1.00 | submitted 3 | 0.0048 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 4.7 | 6.7 | 29775 | 0.0299 | 20.2 | 1.00 | submitted 3 | 0.0365 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9446 | 0.0065 | 10.8 | 1.00 | submitted 3 | 0.0059 |
| t06_mag_revisions | A | 3 | 1.00 | 7.7 | 9.7 | 55521 | 0.0493 | 39.1 | 1.00 | submitted 3 | 0.0562 |
| t06_mag_revisions | B | 3 | 1.00 | 3.3 | 5.3 | 15277 | 0.0129 | 13.3 | 1.00 | submitted 3 | 0.0131 |
| t07_quakeml_coverage | A | 3 | 1.00 | 3.0 | 5.0 | 18858 | 0.0234 | 16.9 | 1.00 | submitted 3 | 0.0296 |
| t07_quakeml_coverage | B | 3 | 1.00 | 4.0 | 6.0 | 19180 | 0.0158 | 18.2 | 1.00 | submitted 3 | 0.0163 |
| t08_deepest_quakeml | A | 3 | 1.00 | 2.3 | 4.3 | 13269 | 0.0174 | 11.6 | 1.00 | submitted 3 | 0.0224 |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.3 | 5.3 | 17043 | 0.0166 | 19.5 | 1.00 | submitted 3 | 0.0193 |
| t09_highest_apogee | A | 3 | 1.00 | 3.3 | 5.3 | 32066 | 0.0473 | 30.2 | 1.00 | submitted 3 | 0.0626 |
| t09_highest_apogee | B | 3 | 1.00 | 3.0 | 5.0 | 25419 | 0.0281 | 18.2 | 0.67 | submitted 3 | 0.0376 |
| t10_low_inclination | A | 3 | 1.00 | 2.3 | 4.3 | 12876 | 0.0152 | 16.4 | 1.00 | submitted 3 | 0.0202 |
| t10_low_inclination | B | 3 | 1.00 | 3.0 | 5.0 | 41074 | 0.0379 | 15.3 | 1.00 | submitted 3 | 0.0506 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.3 | 4.3 | 13398 | 0.0166 | 15.0 | 0.67 | submitted 3 | 0.0214 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9610 | 0.0084 | 11.7 | 0.67 | submitted 3 | 0.0089 |
| t12_saffir_bounds | A | 3 | 1.00 | 3.0 | 5.0 | 21150 | 0.0283 | 11.0 | 1.00 | submitted 3 | 0.0405 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8257 | 0.0047 | 8.9 | 1.00 | submitted 3 | 0.0044 |
| t13_cyclone_categories | A | 3 | 1.00 | 4.0 | 6.0 | 20273 | 0.0307 | 12.6 | 1.00 | submitted 3 | 0.0418 |
| t13_cyclone_categories | B | 3 | 1.00 | 3.3 | 5.3 | 35450 | 0.0400 | 20.3 | 1.00 | submitted 3 | 0.0538 |
| t14_fires_near_quakes | A | 3 | 1.00 | 5.7 | 7.7 | 45528 | 0.0619 | 36.9 | 0.67 | submitted 3 | 0.0704 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.3 | 5.3 | 16198 | 0.0157 | 15.1 | 1.00 | submitted 3 | 0.0167 |
| t15_city_nearest_quake | A | 3 | 1.00 | 3.7 | 5.7 | 26181 | 0.0371 | 20.7 | 1.00 | submitted 3 | 0.0448 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 10867 | 0.0130 | 11.7 | 1.00 | submitted 3 | 0.0134 |
| t16_malformed_weather | A | 3 | 1.00 | 2.3 | 4.3 | 12161 | 0.0157 | 9.9 | 1.00 | submitted 3 | 0.0206 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9013 | 0.0053 | 9.1 | 1.00 | submitted 3 | 0.0046 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 3.7 | 5.7 | 22324 | 0.0424 | 25.2 | 1.00 | submitted 3 | 0.0550 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.0 | 5.0 | 17837 | 0.0204 | 15.2 | 1.00 | submitted 3 | 0.0223 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 4.7 | 6.7 | 31219 | 0.0380 | 18.5 | 1.00 | submitted 3 | 0.0470 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 15533 | 0.0149 | 22.6 | 1.00 | submitted 3 | 0.0164 |
| t19_firms_summary | A | 3 | 1.00 | 5.7 | 7.7 | 36566 | 0.0341 | 21.8 | 1.00 | submitted 3 | 0.0403 |
| t19_firms_summary | B | 3 | 1.00 | 2.0 | 4.0 | 11304 | 0.0095 | 10.7 | 1.00 | submitted 3 | 0.0098 |
| t20_missing_magerror | A | 3 | 1.00 | 4.7 | 6.7 | 27648 | 0.0261 | 26.4 | 1.00 | submitted 3 | 0.0316 |
| t20_missing_magerror | B | 3 | 1.00 | 2.7 | 4.7 | 11478 | 0.0080 | 9.7 | 1.00 | submitted 3 | 0.0078 |
