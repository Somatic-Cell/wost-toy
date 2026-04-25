#!/usr/bin/env python3
"""Plot summary CSV produced by mixed_boundary_benchmark."""
from __future__ import annotations
import argparse
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt


def plot_metric(df: pd.DataFrame, metric: str, output: Path, title: str) -> None:
    methods = list(df["method"].drop_duplicates())
    ks = sorted(df["k"].unique())
    x = range(len(ks))
    width = 0.8 / max(1, len(methods))

    fig, ax = plt.subplots(figsize=(11, 5.5))
    for j, method in enumerate(methods):
        vals = []
        for k in ks:
            row = df[(df["k"] == k) & (df["method"] == method)]
            vals.append(float(row[metric].iloc[0]) if len(row) else float("nan"))
        offsets = [i + (j - (len(methods)-1)/2) * width for i in x]
        ax.bar(offsets, vals, width, label=method)

    if "ratio" in metric:
        ax.axhline(1.0, linestyle="--", linewidth=1)
    ax.set_xticks(list(x))
    ax.set_xticklabels([str(k) for k in ks])
    ax.set_xlabel("boundary frequency k")
    ax.set_ylabel(metric)
    ax.set_title(title)
    ax.legend(fontsize=8, ncol=2)
    fig.tight_layout()
    output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output, dpi=180)
    plt.close(fig)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("summary_csv", type=Path)
    ap.add_argument("--out-dir", type=Path, default=Path("results/plots"))
    args = ap.parse_args()

    df = pd.read_csv(args.summary_csv)
    plot_metric(df, "variance_ratio_vs_mc", args.out_dir / "variance_ratio_vs_mc.png", "Variance ratio vs MC")
    plot_metric(df, "work_ratio_vs_mc", args.out_dir / "work_ratio_vs_mc.png", "Variance × time ratio vs MC")
    plot_metric(df, "rmse", args.out_dir / "rmse.png", "RMSE")
    plot_metric(df, "std", args.out_dir / "std.png", "Std across randomizations")
    print(f"Wrote plots to {args.out_dir}")


if __name__ == "__main__":
    main()
