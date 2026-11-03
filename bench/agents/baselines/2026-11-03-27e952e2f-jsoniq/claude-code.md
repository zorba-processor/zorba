# Agent benchmark: claude-sonnet-5-5 (backend claude-code, language jsoniq)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 3.9 | 5.8 | 25108 | 1.7633 | 18.5 | 0.97 | 18 | 2.1799 |
| B | 20 | 60 | 0.98 | 2.5 | 4.5 | 13499 | 0.8314 | 12.7 | 0.98 | 19 | 0.9885 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 2.0 | 4.0 | 10509 | 0.0127 | 11.4 | 1.00 | submitted 3 | 0.0169 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8888 | 0.0075 | 8.5 | 1.00 | submitted 3 | 0.0084 |
| t02_magtype_counts | A | 3 | 1.00 | 2.7 | 4.7 | 14349 | 0.0169 | 11.3 | 0.67 | submitted 3 | 0.0213 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 8947 | 0.0079 | 13.7 | 1.00 | submitted 3 | 0.0089 |
| t03_m45_stats | A | 3 | 1.00 | 2.7 | 4.7 | 13773 | 0.0160 | 12.0 | 1.00 | submitted 3 | 0.0203 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8856 | 0.0083 | 11.2 | 1.00 | submitted 3 | 0.0095 |
| t04_busiest_day | A | 3 | 1.00 | 2.7 | 4.7 | 13848 | 0.0155 | 13.3 | 1.00 | submitted 3 | 0.0200 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9083 | 0.0075 | 10.3 | 1.00 | submitted 3 | 0.0085 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 4.0 | 6.0 | 23332 | 0.0258 | 16.5 | 1.00 | submitted 3 | 0.0312 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9513 | 0.0094 | 9.5 | 1.00 | submitted 3 | 0.0108 |
| t06_mag_revisions | A | 3 | 1.00 | 6.7 | 8.7 | 42677 | 0.0379 | 30.4 | 1.00 | submitted 3 | 0.0434 |
| t06_mag_revisions | B | 3 | 1.00 | 2.7 | 4.7 | 12569 | 0.0136 | 12.3 | 1.00 | submitted 3 | 0.0154 |
| t07_quakeml_coverage | A | 3 | 1.00 | 2.3 | 4.3 | 15555 | 0.0230 | 12.4 | 1.00 | submitted 3 | 0.0308 |
| t07_quakeml_coverage | B | 3 | 1.00 | 3.7 | 5.7 | 17076 | 0.0157 | 17.9 | 1.00 | submitted 3 | 0.0173 |
| t08_deepest_quakeml | A | 3 | 1.00 | 3.3 | 5.3 | 19159 | 0.0216 | 13.7 | 1.00 | submitted 3 | 0.0274 |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.0 | 5.0 | 15416 | 0.0164 | 14.6 | 1.00 | submitted 3 | 0.0200 |
| t09_highest_apogee | A | 3 | 1.00 | 2.0 | 4.0 | 18667 | 0.0409 | 22.2 | 1.00 | submitted 3 | 0.0550 |
| t09_highest_apogee | B | 3 | 1.00 | 3.3 | 5.3 | 16207 | 0.0153 | 12.0 | 1.00 | submitted 3 | 0.0183 |
| t10_low_inclination | A | 3 | 1.00 | 2.3 | 4.3 | 12196 | 0.0146 | 7.9 | 1.00 | submitted 3 | 0.0192 |
| t10_low_inclination | B | 3 | 1.00 | 2.7 | 4.7 | 31056 | 0.0274 | 12.6 | 1.00 | submitted 3 | 0.0356 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.0 | 4.0 | 11047 | 0.0143 | 10.9 | 0.67 | submitted 3 | 0.0186 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9396 | 0.0100 | 13.2 | 1.00 | submitted 3 | 0.0117 |
| t12_saffir_bounds | A | 3 | 1.00 | 3.0 | 5.0 | 17377 | 0.0205 | 11.0 | 1.00 | submitted 3 | 0.0282 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8321 | 0.0071 | 10.1 | 1.00 | submitted 3 | 0.0084 |
| t13_cyclone_categories | A | 3 | 1.00 | 5.3 | 7.3 | 26236 | 0.0271 | 20.8 | 1.00 | submitted 3 | 0.0326 |
| t13_cyclone_categories | B | 3 | 0.67 | 3.7 | 5.7 | 24995 | 0.0282 | 15.0 | 0.67 | submitted 3 | 0.0365 |
| t14_fires_near_quakes | A | 3 | 1.00 | 7.0 | 9.0 | 51904 | 0.0644 | 40.9 | 1.00 | submitted 3 | 0.0728 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.0 | 5.0 | 14431 | 0.0150 | 15.5 | 1.00 | submitted 3 | 0.0166 |
| t15_city_nearest_quake | A | 3 | 1.00 | 3.0 | 5.0 | 18942 | 0.0309 | 19.3 | 1.00 | submitted 3 | 0.0370 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 10939 | 0.0164 | 10.6 | 1.00 | submitted 3 | 0.0191 |
| t16_malformed_weather | A | 3 | 1.00 | 2.0 | 4.0 | 10655 | 0.0145 | 12.1 | 1.00 | submitted 3 | 0.0193 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9000 | 0.0082 | 10.4 | 1.00 | submitted 3 | 0.0096 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 3.7 | 5.7 | 21365 | 0.0382 | 19.4 | 1.00 | submitted 3 | 0.0487 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.0 | 5.0 | 18062 | 0.0228 | 18.9 | 1.00 | submitted 3 | 0.0265 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 6.7 | 8.7 | 59179 | 0.0623 | 27.6 | 1.00 | submitted 3 | 0.0746 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 17369 | 0.0203 | 14.7 | 1.00 | submitted 3 | 0.0247 |
| t19_firms_summary | A | 3 | 1.00 | 5.0 | 7.0 | 29607 | 0.0309 | 19.7 | 1.00 | submitted 3 | 0.0365 |
| t19_firms_summary | B | 3 | 1.00 | 2.0 | 4.0 | 11323 | 0.0126 | 13.4 | 1.00 | submitted 3 | 0.0150 |
| t20_missing_magerror | A | 3 | 1.00 | 8.7 | 10.7 | 71792 | 0.0597 | 36.5 | 1.00 | submitted 3 | 0.0727 |
| t20_missing_magerror | B | 3 | 1.00 | 2.0 | 4.0 | 8541 | 0.0075 | 9.5 | 1.00 | submitted 3 | 0.0087 |
