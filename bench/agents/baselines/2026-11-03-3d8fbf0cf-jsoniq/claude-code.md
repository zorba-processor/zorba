# Agent benchmark: claude-sonnet-5-5 (backend claude-code, language jsoniq)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 3.6 | 5.6 | 22104 | 1.6372 | 19.1 | 0.95 | 17 | 2.0463 |
| B | 20 | 60 | 1.00 | 2.5 | 4.5 | 13982 | 0.7447 | 12.0 | 1.00 | 20 | 0.8361 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 2.0 | 4.0 | 10532 | 0.0128 | 13.8 | 1.00 | submitted 3 | 0.0170 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8888 | 0.0051 | 11.3 | 1.00 | submitted 3 | 0.0042 |
| t02_magtype_counts | A | 3 | 1.00 | 3.0 | 5.0 | 15987 | 0.0179 | 14.5 | 0.67 | submitted 3 | 0.0224 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9001 | 0.0054 | 7.9 | 1.00 | submitted 3 | 0.0047 |
| t03_m45_stats | A | 3 | 1.00 | 2.7 | 4.7 | 13729 | 0.0158 | 14.0 | 1.00 | submitted 3 | 0.0200 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8812 | 0.0057 | 7.8 | 1.00 | submitted 3 | 0.0052 |
| t04_busiest_day | A | 3 | 1.00 | 3.0 | 5.0 | 15436 | 0.0165 | 17.8 | 1.00 | submitted 3 | 0.0210 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9082 | 0.0053 | 10.2 | 1.00 | submitted 3 | 0.0048 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 4.0 | 6.0 | 24753 | 0.0269 | 18.2 | 1.00 | submitted 3 | 0.0328 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9450 | 0.0070 | 10.0 | 1.00 | submitted 3 | 0.0067 |
| t06_mag_revisions | A | 3 | 1.00 | 6.0 | 8.0 | 39566 | 0.0366 | 24.7 | 1.00 | submitted 3 | 0.0426 |
| t06_mag_revisions | B | 3 | 1.00 | 3.0 | 5.0 | 13741 | 0.0120 | 13.3 | 1.00 | submitted 3 | 0.0126 |
| t07_quakeml_coverage | A | 3 | 1.00 | 3.3 | 5.3 | 27054 | 0.0368 | 16.0 | 1.00 | submitted 3 | 0.0504 |
| t07_quakeml_coverage | B | 3 | 1.00 | 3.3 | 5.3 | 17058 | 0.0130 | 12.0 | 1.00 | submitted 3 | 0.0129 |
| t08_deepest_quakeml | A | 3 | 1.00 | 2.3 | 4.3 | 13337 | 0.0171 | 13.2 | 1.00 | submitted 3 | 0.0224 |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.0 | 5.0 | 14751 | 0.0108 | 12.2 | 1.00 | submitted 3 | 0.0110 |
| t09_highest_apogee | A | 3 | 1.00 | 2.7 | 4.7 | 23011 | 0.0437 | 22.6 | 0.67 | submitted 3 | 0.0584 |
| t09_highest_apogee | B | 3 | 1.00 | 3.0 | 5.0 | 24414 | 0.0258 | 12.8 | 1.00 | submitted 3 | 0.0342 |
| t10_low_inclination | A | 3 | 1.00 | 2.0 | 4.0 | 10592 | 0.0133 | 10.2 | 1.00 | submitted 3 | 0.0178 |
| t10_low_inclination | B | 3 | 1.00 | 3.0 | 5.0 | 40495 | 0.0483 | 17.1 | 1.00 | submitted 3 | 0.0684 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.0 | 4.0 | 11047 | 0.0142 | 9.5 | 1.00 | submitted 3 | 0.0185 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9609 | 0.0068 | 11.8 | 1.00 | submitted 3 | 0.0062 |
| t12_saffir_bounds | A | 3 | 1.00 | 3.0 | 5.0 | 16169 | 0.0176 | 17.9 | 1.00 | submitted 3 | 0.0234 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8309 | 0.0052 | 9.6 | 1.00 | submitted 3 | 0.0052 |
| t13_cyclone_categories | A | 3 | 1.00 | 5.0 | 7.0 | 24146 | 0.0257 | 12.7 | 1.00 | submitted 3 | 0.0315 |
| t13_cyclone_categories | B | 3 | 1.00 | 3.0 | 5.0 | 16236 | 0.0149 | 14.8 | 1.00 | submitted 3 | 0.0168 |
| t14_fires_near_quakes | A | 3 | 1.00 | 5.7 | 7.7 | 49265 | 0.0603 | 53.4 | 0.67 | submitted 3 | 0.0708 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.0 | 5.0 | 14509 | 0.0139 | 16.5 | 1.00 | submitted 3 | 0.0144 |
| t15_city_nearest_quake | A | 3 | 1.00 | 4.3 | 6.3 | 30459 | 0.0434 | 22.5 | 1.00 | submitted 3 | 0.0505 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.3 | 4.3 | 11035 | 0.0140 | 14.0 | 1.00 | submitted 3 | 0.0151 |
| t16_malformed_weather | A | 3 | 1.00 | 2.3 | 4.3 | 12150 | 0.0157 | 12.9 | 1.00 | submitted 3 | 0.0206 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9135 | 0.0059 | 8.4 | 1.00 | submitted 3 | 0.0057 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 3.7 | 5.7 | 22340 | 0.0415 | 18.5 | 1.00 | submitted 3 | 0.0533 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.0 | 5.0 | 17874 | 0.0181 | 17.7 | 1.00 | submitted 3 | 0.0185 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 4.7 | 6.7 | 25691 | 0.0335 | 20.4 | 1.00 | submitted 3 | 0.0414 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 14907 | 0.0141 | 14.4 | 1.00 | submitted 3 | 0.0152 |
| t19_firms_summary | A | 3 | 1.00 | 4.7 | 6.7 | 28373 | 0.0304 | 23.1 | 1.00 | submitted 3 | 0.0363 |
| t19_firms_summary | B | 3 | 1.00 | 2.7 | 4.7 | 12675 | 0.0108 | 9.5 | 1.00 | submitted 3 | 0.0111 |
| t20_missing_magerror | A | 3 | 1.00 | 5.3 | 7.3 | 28440 | 0.0260 | 27.0 | 1.00 | submitted 3 | 0.0306 |
| t20_missing_magerror | B | 3 | 1.00 | 2.3 | 4.3 | 9664 | 0.0062 | 9.7 | 1.00 | submitted 3 | 0.0059 |
