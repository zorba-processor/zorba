#!/usr/bin/env python3
"""Fetch the frozen snapshot of public geospatial feeds used by the agent
benchmark (bench/agents/data/snapshot/) and write bench/agents/data/SOURCES.md.

Run once; the result is committed. The benchmark itself never touches the
network for data. Only sources that are public domain or under an open
license that allows redistribution are used; nothing needs an API key.

Usage: fetch_snapshot.py [--out bench/agents/data]
"""

import argparse
import csv
import datetime
import hashlib
import io
import json
import os
import shutil
import sys
import time
import urllib.parse
import urllib.request

USER_AGENT = "zorba-agent-benchmark-snapshot/1.0 (one-off fetch)"

USGS = "https://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/"
USGS_TERMS = ("Public domain (U.S. Government work). USGS asks for "
              "acknowledgment: \"U.S. Geological Survey\". "
              "https://www.usgs.gov/information-policies-and-instructions/"
              "copyrights-and-credits")
CELESTRAK = "https://celestrak.org/NORAD/elements/gp.php"
CELESTRAK_TERMS = ("Free to use and redistribute with attribution to "
                   "CelesTrak (https://celestrak.org); GP data derived from "
                   "public 18th Space Defense Squadron data. "
                   "https://celestrak.org/webmaster.php")
GDACS_TERMS = ("GDACS (UN OCHA, UNOSAT and European Commission JRC/ECHO). "
               "The GDACS terms of use "
               "(https://www.gdacs.org/About/termofuse.aspx) are a "
               "disclaimer with no reuse restriction; the site's copyright "
               "notice is the European Commission legal notice: reuse "
               "authorised provided the source is acknowledged "
               "(Decision 2011/833/EU). "
               "https://commission.europa.eu/legal-notice_en")
OPEN_METEO_TERMS = ("CC BY 4.0, \"Weather data by Open-Meteo.com\" "
                    "(https://open-meteo.com/en/license).")
FIRMS_TERMS = ("NASA open data, no restrictions on use or redistribution; "
               "acknowledgment requested: \"NASA Fire Information for "
               "Resource Management System (FIRMS)\". "
               "https://www.earthdata.nasa.gov/engage/open-data-services-"
               "software-policies/data-use-guidance")
NHC_TERMS = ("Public domain (U.S. Government work, NOAA National Hurricane "
             "Center). https://www.weather.gov/disclaimer")
AUTHORED_TERMS = ("Written for this benchmark (CC0). Coordinates are the "
                  "commonly published city-centre coordinates.")

FIRMS_URL = ("https://firms.modaps.eosdis.nasa.gov/data/active_fire/"
             "modis-c6.1/csv/MODIS_C6_1_Global_24h.csv")
# Regions (min lon, max lon, min lat, max lat) the fire file is cut down to.
FIRMS_REGIONS = {
    "asia_pacific": (90.0, 180.0, -25.0, 45.0),
    "americas": (-120.0, -30.0, -60.0, 35.0),
}

# City, country, latitude, longitude: cities near the seismic and fire
# activity of the snapshot week (Vanuatu, Japan, Philippines, Indonesia,
# Peru, Chile, Iran, Alaska).
CITIES = [
    ("anchorage", "Anchorage", "United States", 61.2181, -149.9003),
    ("ambon", "Ambon", "Indonesia", -3.6954, 128.1814),
    ("bushehr", "Bushehr", "Iran", 28.9234, 50.8203),
    ("davao", "Davao", "Philippines", 7.0731, 125.6128),
    ("kupang", "Kupang", "Indonesia", -10.1772, 123.6070),
    ("lima", "Lima", "Peru", -12.0464, -77.0428),
    ("luganville", "Luganville", "Vanuatu", -15.5126, 167.1766),
    ("manila", "Manila", "Philippines", 14.5995, 120.9842),
    ("port-vila", "Port Vila", "Vanuatu", -17.7333, 168.3273),
    ("santiago", "Santiago", "Chile", -33.4489, -70.6693),
    ("tokyo", "Tokyo", "Japan", 35.6895, 139.6917),
]
# The weather file of this city is deliberately truncated (see SOURCES.md):
# the malformed-file task depends on it.
TRUNCATED_CITY = "anchorage"
TRUNCATE_AT = 0.5

OPEN_METEO_PARAMS = {
    "current": "temperature_2m,relative_humidity_2m,precipitation,"
               "wind_speed_10m,weather_code",
    "daily": "temperature_2m_max,temperature_2m_min,precipitation_sum,"
             "wind_speed_10m_max",
    "timezone": "UTC",
    "forecast_days": "3",
}


def fetch(url):
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=120) as resp:
                if resp.status != 200:
                    raise RuntimeError(f"HTTP {resp.status} for {url}")
                return resp.read()
        except Exception as e:  # noqa: BLE001 - retried, then re-raised
            if attempt == 2:
                raise RuntimeError(f"cannot fetch {url}: {e}") from e
            time.sleep(5 * (attempt + 1))
    raise AssertionError("unreachable")


def now():
    return datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%dT%H:%M:%SZ")


class Snapshot:
    def __init__(self, out):
        self.root = os.path.join(out, "snapshot")
        self.out = out
        self.entries = []

    def write(self, rel, data, url, terms, note=""):
        path = os.path.join(self.root, rel)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "wb") as f:
            f.write(data)
        self.entries.append({
            "path": rel, "url": url, "retrieved": now(), "terms": terms,
            "note": note, "size": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
        })
        print(f"{rel}: {len(data)} bytes", file=sys.stderr)

    def get(self, rel, url, terms, note=""):
        self.write(rel, fetch(url), url, terms, note)


def firms(snap):
    raw = fetch(FIRMS_URL)
    rows = list(csv.reader(io.StringIO(raw.decode("utf-8"))))
    header, body = rows[0], rows[1:]
    lat, lon = header.index("latitude"), header.index("longitude")
    for name, (x0, x1, y0, y1) in FIRMS_REGIONS.items():
        buf = io.StringIO()
        w = csv.writer(buf, lineterminator="\n")
        w.writerow(header)
        n = 0
        for r in body:
            if x0 <= float(r[lon]) <= x1 and y0 <= float(r[lat]) <= y1:
                w.writerow(r)
                n += 1
        snap.write(f"fires/firms_modis_{name}_24h.csv",
                   buf.getvalue().encode("utf-8"), FIRMS_URL, FIRMS_TERMS,
                   f"Filtered to longitude {x0}..{x1}, latitude {y0}..{y1} "
                   f"({n} of {len(body)} rows); columns and values "
                   "unchanged.")


def weather(snap):
    buf = io.StringIO()
    w = csv.writer(buf, lineterminator="\n")
    w.writerow(["file", "city", "country", "latitude", "longitude"])
    for key, city, country, la, lo in CITIES:
        w.writerow([f"weather/{key}.json", city, country, la, lo])
    snap.write("weather/cities.csv", buf.getvalue().encode("utf-8"),
               "(none)", AUTHORED_TERMS,
               "Index of the weather files: city name and coordinates.")
    for key, city, country, la, lo in CITIES:
        params = dict(OPEN_METEO_PARAMS, latitude=str(la), longitude=str(lo))
        url = ("https://api.open-meteo.com/v1/forecast?" +
               urllib.parse.urlencode(params, safe=","))
        data = fetch(url)
        note = ""
        if key == TRUNCATED_CITY:
            data = data[:int(len(data) * TRUNCATE_AT)]
            note = (f"DELIBERATELY CORRUPTED: truncated to the first "
                    f"{int(TRUNCATE_AT * 100)}% of the response bytes, so it "
                    "is not valid JSON. The malformed-file task depends on "
                    "it.")
        snap.write(f"weather/{key}.json", data, url, OPEN_METEO_TERMS, note)
        time.sleep(1)


def write_sources(snap):
    total = sum(e["size"] for e in snap.entries)
    lines = [
        "# Snapshot sources",
        "",
        "Generated by `bench/agents/fetch_snapshot.py`; do not edit by hand.",
        "The files under `snapshot/` are a frozen copy: the benchmark never "
        "fetches data.",
        "",
        f"Total size: {total} bytes in {len(snap.entries)} files.",
        "",
        "Deliberate defects (part of the benchmark):",
        "",
        f"- `weather/{TRUNCATED_CITY}.json` is truncated and is not valid "
        "JSON.",
        f"- The three `earthquakes/` files were fetched {snap.stagger:g} "
        "minutes apart (GeoJSON, then QuakeML, then CSV), so they disagree "
        "the way copies of a live feed taken at different times do.",
        "",
    ]
    for e in snap.entries:
        lines += [
            f"## `{e['path']}`",
            "",
            f"- URL: {e['url']}",
            f"- Retrieved: {e['retrieved']}",
            f"- Size: {e['size']} bytes, SHA-256 `{e['sha256']}`",
            f"- License/terms: {e['terms']}",
        ]
        if e["note"]:
            lines.append(f"- Note: {e['note']}")
        lines.append("")
    with open(os.path.join(snap.out, "SOURCES.md"), "w") as f:
        f.write("\n".join(lines))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--out", default=os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "data"))
    ap.add_argument("--stagger-minutes", type=float, default=60.0,
                    help="delay between the GeoJSON, QuakeML and CSV "
                    "earthquake fetches (default 60)")
    args = ap.parse_args()
    snap = Snapshot(args.out)
    if os.path.isdir(snap.root):
        shutil.rmtree(snap.root)
    snap.stagger = args.stagger_minutes
    stagger = args.stagger_minutes * 60
    start = time.monotonic()

    # The three earthquake feeds are fetched STAGGER minutes apart on
    # purpose: like copies of a live feed taken at different times, they
    # then disagree (new events, events leaving the one-week window,
    # revised magnitudes and statuses). The cross-format tasks ask about
    # these real differences.
    snap.get("earthquakes/usgs_m2.5_week.geojson",
             USGS + "2.5_week.geojson", USGS_TERMS,
             "Earthquake fetch 1 of 3 (the feeds are fetched "
             f"{args.stagger_minutes:g} minutes apart on purpose).")
    snap.get("satellites/celestrak_stations_omm.xml",
             CELESTRAK + "?GROUP=stations&FORMAT=xml", CELESTRAK_TERMS,
             "CCSDS OMM XML.")
    snap.get("satellites/celestrak_stations.csv",
             CELESTRAK + "?GROUP=stations&FORMAT=csv", CELESTRAK_TERMS)
    snap.get("alerts/gdacs_rss.xml", "https://www.gdacs.org/xml/rss.xml",
             GDACS_TERMS)
    firms(snap)
    weather(snap)
    snap.get("reference/nhc_saffir_simpson_scale.html",
             "https://www.nhc.noaa.gov/aboutsshws.php", NHC_TERMS,
             "HTML page with the Saffir-Simpson hurricane wind scale table.")

    def wait_until(offset):
        left = start + offset - time.monotonic()
        if left > 0:
            print(f"waiting {left / 60:.1f} minutes", file=sys.stderr)
            time.sleep(left)

    wait_until(stagger)
    snap.get("earthquakes/usgs_m4.5_week_quakeml.xml",
             USGS + "4.5_week.quakeml", USGS_TERMS,
             "QuakeML 1.2 (saved with the .xml extension). Earthquake fetch "
             f"2 of 3, {args.stagger_minutes:g} minutes after the GeoJSON.")
    wait_until(2 * stagger)
    snap.get("earthquakes/usgs_m2.5_week.csv", USGS + "2.5_week.csv",
             USGS_TERMS,
             f"Earthquake fetch 3 of 3, {2 * args.stagger_minutes:g} minutes "
             "after the GeoJSON.")
    snap.entries.sort(key=lambda e: e["path"])
    write_sources(snap)


if __name__ == "__main__":
    main()
