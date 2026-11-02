# Agent benchmark: claude-sonnet-5-5 (backend api)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 7.2 | 7.2 | 218642 | 10.6284 | 18.3 | 0.95 | 17 | - |
| B | 20 | 60 | 1.00 | 3.1 | 3.9 | 11659 | 1.1748 | 7.9 | 0.98 | 19 | - |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 5.3 | 6.0 | 66487 | 0.0869 | 11.4 | 1.00 | submitted 3 | - |
| t01_largest_quake | B | 3 | 1.00 | 2.3 | 3.0 | 5131 | 0.0106 | 5.4 | 1.00 | submitted 3 | - |
| t02_magtype_counts | A | 3 | 1.00 | 6.0 | 7.0 | 209604 | 0.1702 | 16.4 | 0.67 | submitted 3 | - |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 3.0 | 4723 | 0.0089 | 5.2 | 1.00 | submitted 3 | - |
| t03_m45_stats | A | 3 | 1.00 | 5.0 | 5.7 | 227880 | 0.2267 | 9.3 | 1.00 | submitted 3 | - |
| t03_m45_stats | B | 3 | 1.00 | 3.3 | 4.0 | 7296 | 0.0136 | 7.1 | 1.00 | submitted 3 | - |
| t04_busiest_day | A | 3 | 1.00 | 7.0 | 7.3 | 379025 | 0.3194 | 18.3 | 1.00 | submitted 3 | - |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 3.0 | 4765 | 0.0091 | 3.0 | 1.00 | submitted 3 | - |
| t05_geojson_vs_csv | A | 3 | 1.00 | 4.7 | 5.3 | 287180 | 0.3047 | 12.5 | 1.00 | submitted 3 | - |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.3 | 3.3 | 6618 | 0.0131 | 5.4 | 1.00 | submitted 3 | - |
| t06_mag_revisions | A | 3 | 1.00 | 8.3 | 7.3 | 605971 | 0.4337 | 18.6 | 1.00 | submitted 3 | - |
| t06_mag_revisions | B | 3 | 1.00 | 4.3 | 5.3 | 15864 | 0.0262 | 12.2 | 1.00 | submitted 3 | - |
| t07_quakeml_coverage | A | 3 | 1.00 | 7.7 | 6.3 | 341170 | 0.2720 | 18.5 | 1.00 | submitted 3 | - |
| t07_quakeml_coverage | B | 3 | 1.00 | 5.0 | 5.0 | 16031 | 0.0271 | 11.9 | 0.67 | submitted 3 | - |
| t08_deepest_quakeml | A | 3 | 1.00 | 9.0 | 9.0 | 46146 | 0.0439 | 21.1 | 1.00 | submitted 3 | - |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.0 | 4.0 | 9539 | 0.0165 | 7.2 | 1.00 | submitted 3 | - |
| t09_highest_apogee | A | 3 | 1.00 | 7.3 | 7.3 | 203473 | 0.1420 | 13.7 | 1.00 | submitted 3 | - |
| t09_highest_apogee | B | 3 | 1.00 | 3.3 | 4.0 | 28427 | 0.0415 | 6.0 | 1.00 | submitted 3 | - |
| t10_low_inclination | A | 3 | 1.00 | 9.3 | 9.7 | 151738 | 0.1097 | 16.5 | 1.00 | submitted 3 | - |
| t10_low_inclination | B | 3 | 1.00 | 3.0 | 4.0 | 27776 | 0.0395 | 7.8 | 1.00 | submitted 3 | - |
| t11_gdacs_counts | A | 3 | 1.00 | 7.3 | 7.7 | 29869 | 0.0334 | 14.9 | 0.67 | submitted 3 | - |
| t11_gdacs_counts | B | 3 | 1.00 | 2.7 | 3.7 | 9013 | 0.0160 | 7.5 | 1.00 | submitted 3 | - |
| t12_saffir_bounds | A | 3 | 1.00 | 4.3 | 4.7 | 27061 | 0.0511 | 9.0 | 1.00 | submitted 3 | - |
| t12_saffir_bounds | B | 3 | 1.00 | 3.0 | 4.0 | 7214 | 0.0113 | 6.8 | 1.00 | submitted 3 | - |
| t13_cyclone_categories | A | 3 | 1.00 | 9.0 | 8.7 | 69922 | 0.0667 | 15.6 | 1.00 | submitted 3 | - |
| t13_cyclone_categories | B | 3 | 1.00 | 4.3 | 5.0 | 23037 | 0.0317 | 11.5 | 1.00 | submitted 3 | - |
| t14_fires_near_quakes | A | 3 | 1.00 | 10.0 | 8.0 | 552962 | 0.3796 | 39.9 | 0.67 | submitted 3 | - |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.7 | 4.7 | 12385 | 0.0206 | 9.6 | 1.00 | submitted 3 | - |
| t15_city_nearest_quake | A | 3 | 1.00 | 9.0 | 9.0 | 189867 | 0.1459 | 25.3 | 1.00 | submitted 3 | - |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 3.0 | 7149 | 0.0183 | 7.1 | 1.00 | submitted 3 | - |
| t16_malformed_weather | A | 3 | 1.00 | 3.3 | 4.0 | 22105 | 0.0307 | 8.4 | 1.00 | submitted 3 | - |
| t16_malformed_weather | B | 3 | 1.00 | 2.7 | 3.3 | 7156 | 0.0121 | 6.3 | 1.00 | submitted 3 | - |
| t17_gdacs_eq_match | A | 3 | 1.00 | 9.3 | 9.3 | 354102 | 0.2665 | 38.0 | 1.00 | submitted 3 | - |
| t17_gdacs_eq_match | B | 3 | 1.00 | 3.3 | 4.3 | 15603 | 0.0296 | 19.0 | 1.00 | submitted 3 | - |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 7.7 | 6.7 | 60392 | 0.0661 | 20.2 | 1.00 | submitted 3 | - |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.7 | 4.0 | 12049 | 0.0216 | 8.0 | 1.00 | submitted 3 | - |
| t19_firms_summary | A | 3 | 1.00 | 7.0 | 7.3 | 343361 | 0.2668 | 22.0 | 1.00 | submitted 3 | - |
| t19_firms_summary | B | 3 | 1.00 | 3.0 | 4.0 | 8498 | 0.0146 | 6.4 | 1.00 | submitted 3 | - |
| t20_missing_magerror | A | 3 | 1.00 | 7.3 | 8.0 | 204529 | 0.1268 | 15.6 | 1.00 | submitted 3 | - |
| t20_missing_magerror | B | 3 | 1.00 | 2.3 | 3.0 | 4899 | 0.0097 | 4.8 | 1.00 | submitted 3 | - |
