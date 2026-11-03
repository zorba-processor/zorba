# Agent benchmark: claude-sonnet-5-5 (backend claude-code, language xquery)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 0.98 | 3.5 | 5.5 | 22696 | 1.6847 | 11.1 | 0.93 | 17 | 2.1363 |
| B | 20 | 60 | 1.00 | 2.5 | 4.5 | 14065 | 0.8981 | 6.8 | 0.98 | 19 | 1.0933 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 2.7 | 4.7 | 14657 | 0.0165 | 6.5 | 1.00 | submitted 3 | 0.0212 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8888 | 0.0075 | 4.7 | 1.00 | submitted 3 | 0.0084 |
| t02_magtype_counts | A | 3 | 1.00 | 3.0 | 5.0 | 16880 | 0.0194 | 8.0 | 1.00 | submitted 3 | 0.0243 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9034 | 0.0083 | 5.1 | 1.00 | submitted 3 | 0.0096 |
| t03_m45_stats | A | 3 | 1.00 | 3.3 | 5.3 | 18437 | 0.0205 | 9.4 | 1.00 | submitted 3 | 0.0254 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8856 | 0.0082 | 5.2 | 1.00 | submitted 3 | 0.0095 |
| t04_busiest_day | A | 3 | 1.00 | 3.0 | 5.0 | 16309 | 0.0175 | 6.8 | 1.00 | submitted 3 | 0.0224 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9082 | 0.0085 | 5.2 | 1.00 | submitted 3 | 0.0102 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 4.0 | 6.0 | 25963 | 0.0292 | 11.5 | 1.00 | submitted 3 | 0.0361 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9317 | 0.0092 | 5.2 | 1.00 | submitted 3 | 0.0106 |
| t06_mag_revisions | A | 3 | 1.00 | 6.3 | 8.3 | 44619 | 0.0413 | 17.9 | 1.00 | submitted 3 | 0.0477 |
| t06_mag_revisions | B | 3 | 1.00 | 3.0 | 5.0 | 13743 | 0.0144 | 7.3 | 1.00 | submitted 3 | 0.0166 |
| t07_quakeml_coverage | A | 3 | 1.00 | 2.7 | 4.7 | 16844 | 0.0217 | 7.9 | 1.00 | submitted 3 | 0.0278 |
| t07_quakeml_coverage | B | 3 | 1.00 | 3.3 | 5.3 | 16997 | 0.0158 | 9.1 | 1.00 | submitted 3 | 0.0173 |
| t08_deepest_quakeml | A | 3 | 1.00 | 2.0 | 4.0 | 11378 | 0.0155 | 6.9 | 1.00 | submitted 3 | 0.0203 |
| t08_deepest_quakeml | B | 3 | 1.00 | 2.7 | 4.7 | 13930 | 0.0156 | 7.7 | 1.00 | submitted 3 | 0.0191 |
| t09_highest_apogee | A | 3 | 1.00 | 2.7 | 4.7 | 23368 | 0.0426 | 15.2 | 0.67 | submitted 3 | 0.0574 |
| t09_highest_apogee | B | 3 | 1.00 | 3.3 | 5.3 | 38567 | 0.0419 | 8.8 | 1.00 | submitted 3 | 0.0572 |
| t10_low_inclination | A | 3 | 1.00 | 2.0 | 4.0 | 11175 | 0.0140 | 5.7 | 1.00 | submitted 3 | 0.0189 |
| t10_low_inclination | B | 3 | 1.00 | 2.7 | 4.7 | 22228 | 0.0269 | 6.9 | 1.00 | submitted 3 | 0.0368 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.0 | 4.0 | 11668 | 0.0151 | 5.9 | 0.33 | submitted 3 | 0.0199 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9557 | 0.0097 | 5.4 | 1.00 | submitted 3 | 0.0112 |
| t12_saffir_bounds | A | 3 | 1.00 | 2.3 | 4.3 | 21156 | 0.0348 | 7.1 | 1.00 | submitted 3 | 0.0521 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8319 | 0.0072 | 5.7 | 1.00 | submitted 3 | 0.0086 |
| t13_cyclone_categories | A | 3 | 0.67 | 3.7 | 5.7 | 18168 | 0.0246 | 8.1 | 0.67 | submitted 3 | 0.0324 |
| t13_cyclone_categories | B | 3 | 1.00 | 3.3 | 5.3 | 20547 | 0.0226 | 8.5 | 1.00 | submitted 3 | 0.0280 |
| t14_fires_near_quakes | A | 3 | 1.00 | 5.7 | 7.7 | 43936 | 0.0539 | 28.7 | 1.00 | submitted 3 | 0.0623 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.0 | 5.0 | 14951 | 0.0169 | 8.3 | 1.00 | submitted 3 | 0.0198 |
| t15_city_nearest_quake | A | 3 | 1.00 | 3.3 | 5.3 | 23441 | 0.0341 | 13.7 | 1.00 | submitted 3 | 0.0414 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 10809 | 0.0153 | 7.4 | 1.00 | submitted 3 | 0.0174 |
| t16_malformed_weather | A | 3 | 1.00 | 2.0 | 4.0 | 10697 | 0.0147 | 7.0 | 1.00 | submitted 3 | 0.0196 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9163 | 0.0085 | 5.3 | 1.00 | submitted 3 | 0.0100 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 4.0 | 6.0 | 23378 | 0.0403 | 15.6 | 1.00 | submitted 3 | 0.0513 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.3 | 5.3 | 21742 | 0.0284 | 10.8 | 1.00 | submitted 3 | 0.0346 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 4.7 | 6.7 | 32192 | 0.0404 | 12.6 | 1.00 | submitted 3 | 0.0512 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 14787 | 0.0149 | 7.8 | 1.00 | submitted 3 | 0.0170 |
| t19_firms_summary | A | 3 | 1.00 | 4.7 | 6.7 | 29583 | 0.0293 | 11.9 | 1.00 | submitted 3 | 0.0354 |
| t19_firms_summary | B | 3 | 1.00 | 2.3 | 4.3 | 12163 | 0.0124 | 6.8 | 0.67 | submitted 3 | 0.0144 |
| t20_missing_magerror | A | 3 | 1.00 | 5.7 | 7.7 | 40066 | 0.0365 | 15.1 | 1.00 | submitted 3 | 0.0449 |
| t20_missing_magerror | B | 3 | 1.00 | 2.0 | 4.0 | 8618 | 0.0071 | 4.7 | 1.00 | submitted 3 | 0.0081 |
