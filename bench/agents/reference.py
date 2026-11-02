#!/usr/bin/env python3
"""Python reference answers for the agent benchmark tasks.

Computes the ground truth of every task in tasks/tasks.json from the frozen
snapshot, independently of the JSONiq queries in queries/ (check_truth.py
compares the two). Standard library only.

Usage:
  reference.py [--snapshot DIR] [--task ID ...]   print the answers as JSON
  reference.py --write                            update tasks/ground_truth.json
  reference.py --check                            fail if ground_truth.json is stale
"""

import argparse
import collections
import csv
import json
import math
import os
import re
import sys
import xml.etree.ElementTree as ET
from email.utils import parsedate_to_datetime
from html.parser import HTMLParser

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_SNAPSHOT = os.path.join(HERE, "data", "snapshot")
TRUTH = os.path.join(HERE, "tasks", "ground_truth.json")

EARTH_RADIUS_KM = 6371.0          # haversine (stated in the tasks)
MU_KM3_S2 = 398600.4418           # Earth's gravitational parameter
EQUATORIAL_RADIUS_KM = 6378.137

QML = "{http://quakeml.org/xmlns/bed/1.2}"
CAT = "{http://anss.org/xmlns/catalog/0.1}"
GDACS = "{http://www.gdacs.org}"
GEORSS = "{http://www.georss.org/georss}"


class Borderline(Exception):
    """A value is too close to a task threshold for an unambiguous answer."""


def haversine(lat1, lon1, lat2, lon2):
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp = p2 - p1
    dl = math.radians(lon2 - lon1)
    h = (math.sin(dp / 2) ** 2 +
         math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2)
    return 2 * EARTH_RADIUS_KM * math.asin(math.sqrt(h))


def not_near(value, threshold, margin, what):
    if abs(value - threshold) < margin:
        raise Borderline(f"{what}: {value} is within {margin} of {threshold}")


class Snapshot:
    def __init__(self, root):
        self.root = root

    def path(self, rel):
        return os.path.join(self.root, rel)

    def json(self, rel):
        with open(self.path(rel), encoding="utf-8") as f:
            return json.load(f)

    def csv(self, rel):
        with open(self.path(rel), encoding="utf-8", newline="") as f:
            return list(csv.DictReader(f))

    def xml(self, rel):
        return ET.parse(self.path(rel)).getroot()

    # --- earthquakes -------------------------------------------------------

    def geojson(self):
        return self.json("earthquakes/usgs_m2.5_week.geojson")["features"]

    def quakes_csv(self):
        return self.csv("earthquakes/usgs_m2.5_week.csv")

    def big_quakes(self):
        """M>=4.5 GeoJSON events: (id, lat, lon, mag, time_ms)."""
        return [(f["id"], f["geometry"]["coordinates"][1],
                 f["geometry"]["coordinates"][0], f["properties"]["mag"],
                 f["properties"]["time"])
                for f in self.geojson() if f["properties"]["mag"] >= 4.5]

    def quakeml(self):
        """{id: {"mag", "depth_m"}} from the preferred magnitude/origin."""
        out = {}
        root = self.xml("earthquakes/usgs_m4.5_week_quakeml.xml")
        for e in root.iter(QML + "event"):
            eid = e.get(CAT + "datasource") + e.get(CAT + "eventid")
            pm = e.findtext(QML + "preferredMagnitudeID")
            po = e.findtext(QML + "preferredOriginID")
            mag = next(m for m in e.findall(QML + "magnitude")
                       if m.get("publicID") == pm)
            org = next(o for o in e.findall(QML + "origin")
                       if o.get("publicID") == po)
            out[eid] = {
                "mag": float(mag.findtext(f"{QML}mag/{QML}value")),
                "depth_m": float(org.findtext(f"{QML}depth/{QML}value")),
            }
        return out

    # --- other sources -----------------------------------------------------

    def fires(self):
        rows = []
        for name in sorted(os.listdir(self.path("fires"))):
            if name.endswith(".csv"):
                for r in self.csv("fires/" + name):
                    rows.append(("fires/" + name, r))
        return rows

    def gdacs_items(self):
        return list(self.xml("alerts/gdacs_rss.xml").iter("item"))

    def cities(self):
        return self.csv("weather/cities.csv")

    def weather(self):
        """{file: parsed JSON or None if it cannot be parsed}."""
        out = {}
        for name in sorted(os.listdir(self.path("weather"))):
            if not name.endswith(".json"):
                continue
            rel = "weather/" + name
            try:
                out[rel] = self.json(rel)
            except ValueError:
                out[rel] = None
        return out

    def saffir_simpson(self):
        """[(category, lower bound km/h)] from the NHC HTML table."""
        with open(self.path("reference/nhc_saffir_simpson_scale.html"),
                  encoding="utf-8", errors="replace") as f:
            page = f.read()
        p = TableParser()
        p.feed(page)
        out = []
        for row in p.rows:
            if len(row) < 2:
                continue
            cat = re.match(r"\s*(\d)", row[0])
            kmh = re.search(r"(\d+)(?:-\d+)?\s*km/h", row[1])
            if cat and kmh:
                out.append((int(cat.group(1)), int(kmh.group(1))))
        return out


class TableParser(HTMLParser):
    """Collects the cell texts of the first <table>."""

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.rows, self.depth, self.done = [], 0, False
        self.row, self.cell = None, None

    def handle_starttag(self, tag, attrs):
        if self.done:
            return
        if tag == "table":
            self.depth += 1
        elif self.depth and tag == "tr":
            self.row = []
        elif self.depth and tag in ("td", "th"):
            self.cell = []
        elif self.cell is not None and tag == "br":
            self.cell.append(" ")

    def handle_endtag(self, tag):
        if self.done:
            return
        if tag == "table":
            self.depth -= 1
            self.done = self.depth == 0
        elif tag in ("td", "th") and self.cell is not None:
            self.row.append(" ".join("".join(self.cell).split()))
            self.cell = None
        elif tag == "tr" and self.row is not None:
            self.rows.append(self.row)
            self.row = None

    def handle_data(self, data):
        if self.cell is not None:
            self.cell.append(data)


# --- tasks -----------------------------------------------------------------

def t01_largest_quake(s):
    feats = s.geojson()
    top = max(f["properties"]["mag"] for f in feats)
    best = [f for f in feats if f["properties"]["mag"] == top]
    if len(best) != 1:
        raise Borderline(f"{len(best)} events share the largest magnitude")
    f = best[0]
    return {"id": f["id"], "mag": top, "place": f["properties"]["place"]}


def t02_magtype_counts(s):
    c = collections.Counter(f["properties"]["magType"] for f in s.geojson())
    return {"counts": [{"magType": k, "count": v}
                       for k, v in sorted(c.items())]}


def t03_m45_stats(s):
    mags = [q[3] for q in s.big_quakes()]
    return {"count": len(mags), "mean_mag": round(sum(mags) / len(mags), 2)}


def t04_busiest_day(s):
    c = collections.Counter(r["time"][:10] for r in s.quakes_csv())
    (day, n), (_, n2) = c.most_common(2)
    if n == n2:
        raise Borderline("two days tie for the most events")
    return {"date": day, "count": n}


def t05_geojson_vs_csv(s):
    g = {f["id"] for f in s.geojson()}
    c = {r["id"] for r in s.quakes_csv()}
    return {"only_in_geojson": sorted(g - c), "only_in_csv": sorted(c - g)}


def t06_mag_revisions(s):
    g = {f["id"]: f["properties"]["mag"] for f in s.geojson()}
    out = []
    for r in s.quakes_csv():
        if r["id"] in g:
            cm = float(r["mag"])
            d = abs(cm - g[r["id"]])
            if d > 0.001:
                not_near(d, 0.001, 0.0005, "magnitude difference")
                out.append({"id": r["id"], "geojson_mag": g[r["id"]],
                            "csv_mag": cm})
    return {"events": sorted(out, key=lambda e: e["id"])}


def t07_quakeml_coverage(s):
    q = s.quakeml()
    big = {f["id"] for f in s.geojson() if f["properties"]["mag"] >= 4.5}
    return {"geojson_m45_not_in_quakeml": sorted(big - set(q)),
            "quakeml_not_in_geojson_m45": sorted(set(q) - big)}


def t08_deepest_quakeml(s):
    q = s.quakeml()
    ranked = sorted(q.items(), key=lambda kv: -kv[1]["depth_m"])
    if ranked[0][1]["depth_m"] == ranked[1][1]["depth_m"]:
        raise Borderline("two events share the largest depth")
    eid, v = ranked[0]
    return {"id": eid, "depth_km": round(v["depth_m"] / 1000, 3)}


def apogee_km(mean_motion, ecc):
    n = mean_motion * 2 * math.pi / 86400.0        # rad/s
    a = (MU_KM3_S2 / n ** 2) ** (1 / 3)
    return a * (1 + ecc) - EQUATORIAL_RADIUS_KM


def t09_highest_apogee(s):
    rows = s.csv("satellites/celestrak_stations.csv")
    ranked = sorted(((apogee_km(float(r["MEAN_MOTION"]),
                                float(r["ECCENTRICITY"])), r) for r in rows),
                    key=lambda x: -x[0])
    if ranked[0][0] - ranked[1][0] < 10:
        raise Borderline("highest apogees are within 10 km")
    alt, r = ranked[0]
    return {"norad_cat_id": int(r["NORAD_CAT_ID"]),
            "object_name": r["OBJECT_NAME"], "apogee_km": round(alt, 1)}


def t10_low_inclination(s):
    root = s.xml("satellites/celestrak_stations_omm.xml")
    names = []
    for seg in root.iter("segment"):
        inc = float(seg.findtext("data/meanElements/INCLINATION"))
        not_near(inc, 45.0, 0.01, "inclination")
        if inc < 45.0:
            names.append(seg.findtext("metadata/OBJECT_NAME"))
    return {"object_names": sorted(names)}


def t11_gdacs_counts(s):
    items = s.gdacs_items()
    c = collections.Counter(i.findtext(GDACS + "eventtype") for i in items)
    red = sum(1 for i in items if i.findtext(GDACS + "alertlevel") == "Red")
    return {"by_type": [{"eventtype": k, "count": v}
                        for k, v in sorted(c.items())],
            "red_alerts": red}


def t12_saffir_bounds(s):
    bounds = s.saffir_simpson()
    if [c for c, _ in bounds] != [1, 2, 3, 4, 5]:
        raise Borderline(f"unexpected table rows: {bounds}")
    return {"lower_bounds_kmh": [b for _, b in bounds]}


def category(kmh, bounds):
    cat = 0
    for c, lo in bounds:
        if kmh >= lo:
            cat = c
    return cat


def t13_cyclone_categories(s):
    bounds = s.saffir_simpson()
    out = []
    for i in s.gdacs_items():
        if i.findtext(GDACS + "eventtype") != "TC":
            continue
        kmh = float(i.find(GDACS + "severity").get("value"))
        for _, lo in bounds:
            not_near(kmh, lo, 0.05, "cyclone wind speed")
        out.append({"eventid": i.findtext(GDACS + "eventid"),
                    "name": i.findtext(GDACS + "eventname"),
                    "wind_kmh": round(kmh, 1),
                    "category": category(kmh, bounds)})
    return {"cyclones": sorted(out, key=lambda c: c["eventid"])}


def t14_fires_near_quakes(s, radius=100.0):
    quakes = s.big_quakes()
    count, hit = 0, set()
    for _, r in s.fires():
        la, lo = float(r["latitude"]), float(r["longitude"])
        near = False
        for qid, qla, qlo, _, _ in quakes:
            d = haversine(la, lo, qla, qlo)
            not_near(d, radius, 0.05, "fire-quake distance")
            if d <= radius:
                near = True
                hit.add(qid)
        count += near
    return {"fire_detections": count, "quake_ids": sorted(hit)}


def t15_city_nearest_quake(s):
    quakes = s.big_quakes()
    out = []
    for c in s.cities():
        la, lo = float(c["latitude"]), float(c["longitude"])
        ranked = sorted((haversine(la, lo, q[1], q[2]), q[0]) for q in quakes)
        if ranked[1][0] - ranked[0][0] < 1.0:
            raise Borderline(f"two quakes are nearly equidistant from {c}")
        out.append({"city": c["city"], "quake_id": ranked[0][1],
                    "distance_km": round(ranked[0][0], 1)})
    return {"cities": sorted(out, key=lambda c: c["city"])}


def t16_malformed_weather(s):
    names = {c["file"]: c["city"] for c in s.cities()}
    w = s.weather()
    bad = sorted(k for k, v in w.items() if v is None)
    good = [(v["current"]["temperature_2m"], names[k])
            for k, v in w.items() if v is not None]
    good.sort(reverse=True)
    if good[0][0] == good[1][0]:
        raise Borderline("two cities share the highest temperature")
    return {"city": good[0][1], "temperature_c": good[0][0],
            "unreadable_files": bad}


def t17_gdacs_eq_match(s):
    quakes = s.big_quakes()
    out = []
    for i in s.gdacs_items():
        if i.findtext(GDACS + "eventtype") != "EQ":
            continue
        la, lo = map(float, i.findtext(GEORSS + "point").split())
        t = parsedate_to_datetime(i.findtext(GDACS + "fromdate"))
        tms = t.timestamp() * 1000
        match = sorted((haversine(la, lo, q[1], q[2]), q[0]) for q in quakes
                       if abs(q[4] - tms) <= 120000)
        match = [m for m in match if m[0] <= 50]
        if len(match) > 1 and match[1][0] - match[0][0] < 1.0:
            raise Borderline(f"several USGS matches for {i.findtext('guid')}")
        out.append({"eventid": i.findtext(GDACS + "eventid"),
                    "usgs_id": match[0][1] if match else None})
    return {"matches": sorted(out, key=lambda m: m["eventid"])}


def t18_hot_cities_near_alerts(s, radius=500.0, tmax=30.0):
    points = []
    for i in s.gdacs_items():
        la, lo = map(float, i.findtext(GEORSS + "point").split())
        points.append((la, lo))
    w = s.weather()
    out = []
    for c in s.cities():
        data = w.get(c["file"])
        if data is None:
            continue
        hot = data["daily"]["temperature_2m_max"][0]
        not_near(hot, tmax, 0.05, "daily maximum")
        la, lo = float(c["latitude"]), float(c["longitude"])
        n = 0
        for pla, plo in points:
            d = haversine(la, lo, pla, plo)
            not_near(d, radius, 0.5, "city-alert distance")
            n += d <= radius
        if hot >= tmax and n:
            out.append({"city": c["city"], "alerts_within_500km": n})
    return {"cities": sorted(out, key=lambda c: c["city"])}


def t19_firms_summary(s):
    per = collections.OrderedDict()
    for f, r in s.fires():
        e = per.setdefault(f, {"file": f, "detections": 0,
                               "high_confidence": 0, "total_frp": 0.0})
        e["detections"] += 1
        e["high_confidence"] += int(r["confidence"]) >= 80
        e["total_frp"] += float(r["frp"])
    for e in per.values():
        e["total_frp"] = round(e["total_frp"], 1)
    return {"files": list(per.values())}


def t20_missing_magerror(s):
    ids = sorted(r["id"] for r in s.quakes_csv() if r["magError"] == "")
    return {"count": len(ids), "ids": ids}


TASKS = {name: fn for name, fn in globals().items()
         if re.match(r"t\d\d_", name) and callable(fn)}


def compute(snapshot, ids=None):
    s = Snapshot(snapshot)
    out = collections.OrderedDict()
    for name in sorted(TASKS):
        if ids and name not in ids:
            continue
        out[name] = TASKS[name](s)
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--snapshot", default=DEFAULT_SNAPSHOT)
    ap.add_argument("--task", action="append")
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    try:
        answers = compute(args.snapshot, args.task)
    except Borderline as e:
        sys.exit(f"reference.py: ambiguous task data: {e}")
    text = json.dumps(answers, indent=1, ensure_ascii=False) + "\n"
    if args.write:
        with open(TRUTH, "w", encoding="utf-8") as f:
            f.write(text)
    elif args.check:
        with open(TRUTH, encoding="utf-8") as f:
            if json.load(f) != json.loads(text):
                sys.exit("reference.py: tasks/ground_truth.json is stale; "
                         "run reference.py --write")
        print("ground_truth.json matches the reference")
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
