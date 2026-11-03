# Agent benchmark: claude-sonnet-5-5 (backend claude-code, language jsoniq)

A = zorba-mcp tools only; B = bash sandbox only. Tokens include cache reads and writes; repeatability is the share of runs that gave the most common answer. Cost USD is the harness estimate from list prices; reported USD is the total_cost_usd that Claude Code reports (claude-code backend only; an estimate, not billing).

## Aggregate

| config | tasks | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | fully repeatable tasks | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 20 | 60 | 1.00 | 3.5 | 5.5 | 22860 | 1.6994 | 10.8 | 0.95 | 17 | 2.1468 |
| B | 20 | 60 | 1.00 | 2.6 | 4.6 | 14658 | 0.7303 | 6.9 | 0.97 | 18 | 0.7957 |

## Per task

| task | config | runs | accuracy | tool calls | turns | tokens | cost USD | wall s | repeatability | stops | reported USD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| t01_largest_quake | A | 3 | 1.00 | 2.7 | 4.7 | 14547 | 0.0163 | 6.5 | 1.00 | submitted 3 | 0.0209 |
| t01_largest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 8897 | 0.0054 | 5.1 | 1.00 | submitted 3 | 0.0047 |
| t02_magtype_counts | A | 3 | 1.00 | 2.3 | 4.3 | 13126 | 0.0162 | 6.5 | 0.67 | submitted 3 | 0.0209 |
| t02_magtype_counts | B | 3 | 1.00 | 2.0 | 4.0 | 8899 | 0.0057 | 5.0 | 1.00 | submitted 3 | 0.0053 |
| t03_m45_stats | A | 3 | 1.00 | 2.7 | 4.7 | 14557 | 0.0170 | 7.3 | 1.00 | submitted 3 | 0.0216 |
| t03_m45_stats | B | 3 | 1.00 | 2.0 | 4.0 | 8809 | 0.0059 | 5.0 | 1.00 | submitted 3 | 0.0056 |
| t04_busiest_day | A | 3 | 1.00 | 3.0 | 5.0 | 16599 | 0.0182 | 7.3 | 1.00 | submitted 3 | 0.0234 |
| t04_busiest_day | B | 3 | 1.00 | 2.0 | 4.0 | 9085 | 0.0053 | 4.7 | 1.00 | submitted 3 | 0.0048 |
| t05_geojson_vs_csv | A | 3 | 1.00 | 3.0 | 5.0 | 17729 | 0.0219 | 8.3 | 1.00 | submitted 3 | 0.0278 |
| t05_geojson_vs_csv | B | 3 | 1.00 | 2.0 | 4.0 | 9266 | 0.0063 | 5.1 | 1.00 | submitted 3 | 0.0056 |
| t06_mag_revisions | A | 3 | 1.00 | 6.0 | 8.0 | 53233 | 0.0511 | 16.4 | 1.00 | submitted 3 | 0.0628 |
| t06_mag_revisions | B | 3 | 1.00 | 3.0 | 5.0 | 13789 | 0.0111 | 7.5 | 1.00 | submitted 3 | 0.0112 |
| t07_quakeml_coverage | A | 3 | 1.00 | 2.7 | 4.7 | 16798 | 0.0219 | 8.5 | 1.00 | submitted 3 | 0.0281 |
| t07_quakeml_coverage | B | 3 | 1.00 | 3.3 | 5.3 | 17000 | 0.0135 | 9.0 | 1.00 | submitted 3 | 0.0136 |
| t08_deepest_quakeml | A | 3 | 1.00 | 2.0 | 4.0 | 11343 | 0.0156 | 6.9 | 1.00 | submitted 3 | 0.0203 |
| t08_deepest_quakeml | B | 3 | 1.00 | 3.0 | 5.0 | 16221 | 0.0161 | 7.6 | 1.00 | submitted 3 | 0.0191 |
| t09_highest_apogee | A | 3 | 1.00 | 2.0 | 4.0 | 19211 | 0.0410 | 15.3 | 0.67 | submitted 3 | 0.0555 |
| t09_highest_apogee | B | 3 | 1.00 | 3.0 | 5.0 | 32640 | 0.0272 | 8.0 | 1.00 | submitted 3 | 0.0343 |
| t10_low_inclination | A | 3 | 1.00 | 3.0 | 5.0 | 16184 | 0.0176 | 7.5 | 1.00 | submitted 3 | 0.0226 |
| t10_low_inclination | B | 3 | 1.00 | 2.7 | 4.7 | 29900 | 0.0241 | 6.7 | 1.00 | submitted 3 | 0.0304 |
| t11_gdacs_counts | A | 3 | 1.00 | 2.0 | 4.0 | 11694 | 0.0155 | 6.3 | 0.67 | submitted 3 | 0.0203 |
| t11_gdacs_counts | B | 3 | 1.00 | 2.0 | 4.0 | 9506 | 0.0084 | 5.6 | 0.67 | submitted 3 | 0.0090 |
| t12_saffir_bounds | A | 3 | 1.00 | 2.7 | 4.7 | 19425 | 0.0273 | 7.1 | 1.00 | submitted 3 | 0.0396 |
| t12_saffir_bounds | B | 3 | 1.00 | 2.0 | 4.0 | 8663 | 0.0069 | 6.6 | 1.00 | submitted 3 | 0.0071 |
| t13_cyclone_categories | A | 3 | 1.00 | 3.7 | 5.7 | 18216 | 0.0249 | 7.9 | 1.00 | submitted 3 | 0.0327 |
| t13_cyclone_categories | B | 3 | 1.00 | 3.7 | 5.7 | 22267 | 0.0213 | 10.0 | 1.00 | submitted 3 | 0.0245 |
| t14_fires_near_quakes | A | 3 | 1.00 | 6.7 | 8.7 | 46750 | 0.0568 | 26.9 | 1.00 | submitted 3 | 0.0658 |
| t14_fires_near_quakes | B | 3 | 1.00 | 3.0 | 5.0 | 15086 | 0.0130 | 9.0 | 1.00 | submitted 3 | 0.0130 |
| t15_city_nearest_quake | A | 3 | 1.00 | 3.3 | 5.3 | 19997 | 0.0322 | 13.0 | 1.00 | submitted 3 | 0.0391 |
| t15_city_nearest_quake | B | 3 | 1.00 | 2.0 | 4.0 | 10881 | 0.0135 | 7.8 | 1.00 | submitted 3 | 0.0144 |
| t16_malformed_weather | A | 3 | 1.00 | 2.0 | 4.0 | 10587 | 0.0142 | 6.7 | 1.00 | submitted 3 | 0.0190 |
| t16_malformed_weather | B | 3 | 1.00 | 2.0 | 4.0 | 9162 | 0.0058 | 5.0 | 1.00 | submitted 3 | 0.0055 |
| t17_gdacs_eq_match | A | 3 | 1.00 | 4.3 | 6.3 | 31681 | 0.0486 | 16.9 | 1.00 | submitted 3 | 0.0606 |
| t17_gdacs_eq_match | B | 3 | 1.00 | 4.7 | 6.7 | 27701 | 0.0284 | 11.7 | 1.00 | submitted 3 | 0.0324 |
| t18_hot_cities_near_alerts | A | 3 | 1.00 | 5.3 | 7.3 | 41691 | 0.0492 | 16.4 | 1.00 | submitted 3 | 0.0611 |
| t18_hot_cities_near_alerts | B | 3 | 1.00 | 3.0 | 5.0 | 14816 | 0.0123 | 7.6 | 1.00 | submitted 3 | 0.0126 |
| t19_firms_summary | A | 3 | 1.00 | 5.3 | 7.3 | 33513 | 0.0324 | 13.6 | 1.00 | submitted 3 | 0.0385 |
| t19_firms_summary | B | 3 | 1.00 | 2.0 | 4.0 | 11953 | 0.0083 | 6.1 | 0.67 | submitted 3 | 0.0078 |
| t20_missing_magerror | A | 3 | 1.00 | 5.3 | 7.3 | 30320 | 0.0286 | 11.0 | 1.00 | submitted 3 | 0.0352 |
| t20_missing_magerror | B | 3 | 1.00 | 2.0 | 4.0 | 8618 | 0.0049 | 4.8 | 1.00 | submitted 3 | 0.0044 |
