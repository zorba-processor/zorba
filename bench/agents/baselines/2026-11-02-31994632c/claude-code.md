# Agent benchmark: claude-sonnet-5-5 (backend claude-code)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 4.2 | 6.2 | 24829 | 1.8035 | 20.7 | 0.97 | 18 | 2.2077 |
| B | 20 | 60 | 1.00 | 2.5 | 4.5 | 13834 | 0.8962 | 12.9 | 1.00 | 20 | 1.0905 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 4.0 | 6.0 | 18621 | 0.0188 | 16.8 | 1.00 | submitted 3 | 0.0223 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8888 | 0.0051 | 8.9 | 1.00 | submitted 3 | 0.0042 |
| t02_magtype_counts | A | 3 | 1.00 | 3.3 | 5.3 | 14947 | 0.0168 | 14.2 | 0.67 | submitted 3 | 0.0205 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9024 | 0.0081 | 10.3 | 1.00 | submitted 3 | 0.0091 |
| t03_m45_stats | A | 3 | 1.00 | 3.0 | 5.0 | 13405 | 0.0158 | 13.8 | 1.00 | submitted 3 | 0.0195 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8811 | 0.0081 | 9.8 | 1.00 | submitted 3 | 0.0093 |
| t04_busiest_day | A | 3 | 1.00 | 5.3 | 7.3 | 27830 | 0.0267 | 21.3 | 1.00 | submitted 3 | 0.0322 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9098 | 0.0081 | 10.4 | 1.00 | submitted 3 | 0.0095 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 4.7 | 6.7 | 25139 | 0.0280 | 31.1 | 1.00 | submitted 3 | 0.0337 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9435 | 0.0095 | 12.5 | 1.00 | submitted 3 | 0.0111 |
| t06_mag_revisions | A | 3 | 1.00 | 7.3 | 9.3 | 44791 | 0.0416 | 27.0 | 1.00 | submitted 3 | 0.0472 |
| t06_mag_revisions | B | 3 | 1.00 | 3.3 | 5.3 | 15568 | 0.0166 | 19.5 | 1.00 | submitted 3 | 0.0189 |
| t07_quakeml_coverage | A | 3 | 1.00 | 2.3 | 4.3 | 14086 | 0.0223 | 14.5 | 1.00 | submitted 3 | 0.0294 |
| t07_quakeml_coverage | B | 3 | 1.00 | 3.0 | 5.0 | 16014 | 0.0163 | 13.9 | 1.00 | submitted 3 | 0.0186 |
| t08_deepest_quakeml | A | 3 | 1.00 | 2.7 | 4.7 | 13628 | 0.0168 | 11.7 | 1.00 | submitted 3 | 0.0217 |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.0 | 5.0 | 15309 | 0.0147 | 12.9 | 1.00 | submitted 3 | 0.0174 |
| t09_highest_apogee | A | 3 | 1.00 | 4.0 | 6.0 | 35279 | 0.0467 | 22.7 | 0.67 | submitted 3 | 0.0614 |
| t09_highest_apogee | B | 3 | 1.00 | 3.3 | 5.3 | 26837 | 0.0303 | 16.1 | 1.00 | submitted 3 | 0.0406 |
| t10_low_inclination | A | 3 | 1.00 | 2.0 | 4.0 | 9186 | 0.0118 | 11.1 | 1.00 | submitted 3 | 0.0156 |
| t10_low_inclination | B | 3 | 1.00 | 2.7 | 4.7 | 26633 | 0.0386 | 14.2 | 1.00 | submitted 3 | 0.0551 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.7 | 4.7 | 12510 | 0.0153 | 19.5 | 1.00 | submitted 3 | 0.0190 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9583 | 0.0102 | 11.8 | 1.00 | submitted 3 | 0.0120 |
| t12_saffir_bounds | A | 3 | 1.00 | 2.7 | 4.7 | 16873 | 0.0253 | 12.5 | 1.00 | submitted 3 | 0.0367 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8206 | 0.0068 | 12.0 | 1.00 | submitted 3 | 0.0079 |
| t13_cyclone_categories | A | 3 | 1.00 | 4.0 | 6.0 | 15550 | 0.0230 | 22.0 | 1.00 | submitted 3 | 0.0299 |
| t13_cyclone_categories | B | 3 | 1.00 | 3.0 | 5.0 | 21391 | 0.0243 | 13.0 | 1.00 | submitted 3 | 0.0315 |
| t14_fires_near_quakes | A | 3 | 1.00 | 7.0 | 9.0 | 50289 | 0.0643 | 43.1 | 1.00 | submitted 3 | 0.0730 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.0 | 5.0 | 14850 | 0.0147 | 19.1 | 1.00 | submitted 3 | 0.0160 |
| t15_city_nearest_quake | A | 3 | 1.00 | 5.3 | 7.3 | 33819 | 0.0461 | 26.7 | 1.00 | submitted 3 | 0.0536 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 10862 | 0.0148 | 13.1 | 1.00 | submitted 3 | 0.0166 |
| t16_malformed_weather | A | 3 | 1.00 | 2.0 | 4.0 | 11038 | 0.0145 | 9.6 | 1.00 | submitted 3 | 0.0195 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9123 | 0.0057 | 8.8 | 1.00 | submitted 3 | 0.0053 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 5.0 | 7.0 | 35034 | 0.0540 | 27.9 | 1.00 | submitted 3 | 0.0650 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.0 | 5.0 | 18551 | 0.0253 | 12.6 | 1.00 | submitted 3 | 0.0305 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 6.0 | 8.0 | 40589 | 0.0497 | 24.7 | 1.00 | submitted 3 | 0.0608 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 17375 | 0.0203 | 15.3 | 1.00 | submitted 3 | 0.0248 |
| t19_firms_summary | A | 3 | 1.00 | 5.3 | 7.3 | 32914 | 0.0336 | 22.7 | 1.00 | submitted 3 | 0.0395 |
| t19_firms_summary | B | 3 | 1.00 | 2.7 | 4.7 | 12472 | 0.0142 | 13.1 | 1.00 | submitted 3 | 0.0172 |
| t20_missing_magerror | A | 3 | 1.00 | 6.0 | 8.0 | 31056 | 0.0299 | 21.4 | 1.00 | submitted 3 | 0.0352 |
| t20_missing_magerror | B | 3 | 1.00 | 2.0 | 4.0 | 8647 | 0.0070 | 11.1 | 1.00 | submitted 3 | 0.0080 |
