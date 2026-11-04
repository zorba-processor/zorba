# Agent benchmark: claude-sonnet-5-5 (backend claude-code, language jsoniq)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 3.0 | 5.0 | 19315 | 1.4289 | 14.4 | 0.95 | 17 | 1.8268 |
| B | 20 | 60 | 1.00 | 2.6 | 4.6 | 13916 | 0.8737 | 13.5 | 0.98 | 19 | 1.0428 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 2.0 | 4.0 | 11629 | 0.0133 | 10.8 | 1.00 | submitted 3 | 0.0176 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8888 | 0.0075 | 7.4 | 1.00 | submitted 3 | 0.0084 |
| t02_magtype_counts | A | 3 | 1.00 | 2.0 | 4.0 | 11759 | 0.0137 | 9.6 | 1.00 | submitted 3 | 0.0180 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9001 | 0.0078 | 14.9 | 1.00 | submitted 3 | 0.0087 |
| t03_m45_stats | A | 3 | 1.00 | 2.3 | 4.3 | 13538 | 0.0154 | 8.7 | 1.00 | submitted 3 | 0.0198 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8812 | 0.0077 | 11.8 | 1.00 | submitted 3 | 0.0085 |
| t04_busiest_day | A | 3 | 1.00 | 2.0 | 4.0 | 11511 | 0.0128 | 8.7 | 1.00 | submitted 3 | 0.0171 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9082 | 0.0075 | 6.9 | 1.00 | submitted 3 | 0.0085 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 3.0 | 5.0 | 17693 | 0.0195 | 11.7 | 1.00 | submitted 3 | 0.0241 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9565 | 0.0096 | 12.4 | 1.00 | submitted 3 | 0.0110 |
| t06_mag_revisions | A | 3 | 1.00 | 4.7 | 6.7 | 27876 | 0.0271 | 21.4 | 1.00 | submitted 3 | 0.0318 |
| t06_mag_revisions | B | 3 | 1.00 | 3.3 | 5.3 | 15275 | 0.0156 | 18.0 | 1.00 | submitted 3 | 0.0175 |
| t07_quakeml_coverage | A | 3 | 1.00 | 3.0 | 5.0 | 17355 | 0.0215 | 13.7 | 1.00 | submitted 3 | 0.0273 |
| t07_quakeml_coverage | B | 3 | 1.00 | 3.7 | 5.7 | 18972 | 0.0184 | 20.2 | 1.00 | submitted 3 | 0.0207 |
| t08_deepest_quakeml | A | 3 | 1.00 | 2.3 | 4.3 | 14602 | 0.0177 | 10.7 | 1.00 | submitted 3 | 0.0231 |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.0 | 5.0 | 15154 | 0.0152 | 15.9 | 1.00 | submitted 3 | 0.0180 |
| t09_highest_apogee | A | 3 | 1.00 | 2.3 | 4.3 | 23478 | 0.0376 | 16.2 | 0.67 | submitted 3 | 0.0522 |
| t09_highest_apogee | B | 3 | 1.00 | 4.0 | 6.0 | 22693 | 0.0207 | 14.6 | 0.67 | submitted 3 | 0.0242 |
| t10_low_inclination | A | 3 | 1.00 | 2.0 | 4.0 | 11738 | 0.0139 | 8.0 | 1.00 | submitted 3 | 0.0185 |
| t10_low_inclination | B | 3 | 1.00 | 3.0 | 5.0 | 32315 | 0.0393 | 11.5 | 1.00 | submitted 3 | 0.0551 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.0 | 4.0 | 12173 | 0.0150 | 9.1 | 0.67 | submitted 3 | 0.0194 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9596 | 0.0098 | 8.8 | 1.00 | submitted 3 | 0.0113 |
| t12_saffir_bounds | A | 3 | 1.00 | 2.7 | 4.7 | 20081 | 0.0275 | 16.8 | 1.00 | submitted 3 | 0.0395 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8322 | 0.0071 | 10.8 | 1.00 | submitted 3 | 0.0084 |
| t13_cyclone_categories | A | 3 | 1.00 | 3.3 | 5.3 | 15823 | 0.0211 | 12.7 | 1.00 | submitted 3 | 0.0272 |
| t13_cyclone_categories | B | 3 | 1.00 | 3.3 | 5.3 | 19978 | 0.0214 | 20.8 | 1.00 | submitted 3 | 0.0260 |
| t14_fires_near_quakes | A | 3 | 1.00 | 2.7 | 4.7 | 21653 | 0.0316 | 20.1 | 0.67 | submitted 3 | 0.0399 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.0 | 5.0 | 14481 | 0.0152 | 15.0 | 1.00 | submitted 3 | 0.0169 |
| t15_city_nearest_quake | A | 3 | 1.00 | 3.7 | 5.7 | 32651 | 0.0434 | 19.2 | 1.00 | submitted 3 | 0.0531 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 10878 | 0.0161 | 9.6 | 1.00 | submitted 3 | 0.0187 |
| t16_malformed_weather | A | 3 | 1.00 | 1.7 | 3.7 | 10191 | 0.0138 | 11.2 | 1.00 | submitted 3 | 0.0184 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9146 | 0.0080 | 9.2 | 1.00 | submitted 3 | 0.0091 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 4.7 | 6.7 | 29345 | 0.0444 | 23.4 | 1.00 | submitted 3 | 0.0569 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.0 | 5.0 | 18006 | 0.0226 | 20.3 | 1.00 | submitted 3 | 0.0262 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 5.7 | 7.7 | 35856 | 0.0392 | 20.0 | 1.00 | submitted 3 | 0.0470 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 16866 | 0.0194 | 14.2 | 1.00 | submitted 3 | 0.0235 |
| t19_firms_summary | A | 3 | 1.00 | 3.0 | 5.0 | 18831 | 0.0227 | 14.7 | 1.00 | submitted 3 | 0.0281 |
| t19_firms_summary | B | 3 | 1.00 | 2.3 | 4.3 | 11628 | 0.0142 | 12.8 | 1.00 | submitted 3 | 0.0175 |
| t20_missing_magerror | A | 3 | 1.00 | 5.0 | 7.0 | 28522 | 0.0254 | 21.7 | 1.00 | submitted 3 | 0.0297 |
| t20_missing_magerror | B | 3 | 1.00 | 2.3 | 4.3 | 9661 | 0.0083 | 14.0 | 1.00 | submitted 3 | 0.0095 |
