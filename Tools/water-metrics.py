"""Settling metrics for sand.capture.steps captures (SandCell.hpp layout, row 0 at the top).

    py Tools/water-metrics.py run_s0.npy run_s300.npy run_s1200.npy --equilibrium run_s20000.npy

Height is measured in cells above the bottom row. Water settles by losing height, so the mean water height
at step N against the start and against a long-run equilibrium says how far it has settled:
settled = (h0 - hN) / (h0 - h_eq). Sand is reported alongside as a control that water tuning should not move.
Airborne cells have air directly below them.
"""
import argparse
import re

import numpy as np

AIR, STONE, SAND, WATER = 0, 1, 2, 3


def load(path):
    cells = np.load(path)
    return cells[..., 0], cells.shape[0]


def stats(material, height):
    rows = np.arange(material.shape[0])[:, None]
    above_bottom = np.broadcast_to(height - 1 - rows, material.shape)
    below = np.vstack([material[1:], np.full((1, material.shape[1]), STONE)])
    out = {}
    for name, m in (("water", WATER), ("sand", SAND)):
        mask = material == m
        out[name] = {
            "count": int(mask.sum()),
            "mean_height": float(above_bottom[mask].mean()) if mask.any() else float("nan"),
            "airborne": int((mask & (below == AIR)).sum()),
        }
    return out


def step_of(path):
    m = re.search(r"_s(\d+)\.npy$", path)
    return int(m.group(1)) if m else -1


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("captures", nargs="+")
    ap.add_argument("--equilibrium", help="a long-run capture to measure settling against")
    args = ap.parse_args()

    captures = sorted(args.captures, key=step_of)
    results = [(step_of(p), stats(*load(p))) for p in captures]
    h0 = results[0][1]["water"]["mean_height"]
    h_eq = stats(*load(args.equilibrium))["water"]["mean_height"] if args.equilibrium else None

    print(f"{'step':>7} {'water h':>9} {'settled':>8} {'airborne':>8} {'sand h':>8} {'water n':>8} {'sand n':>7}")
    for step, s in results:
        w, sd = s["water"], s["sand"]
        settled = "" if h_eq is None or h0 == h_eq else f"{(h0 - w['mean_height']) / (h0 - h_eq):8.3f}"
        print(f"{step:>7} {w['mean_height']:9.3f} {settled:>8} {w['airborne']:8d} {sd['mean_height']:8.3f} {w['count']:8d} {sd['count']:7d}")
    if h_eq is not None:
        print(f"equilibrium water h {h_eq:.3f}")


if __name__ == "__main__":
    main()
