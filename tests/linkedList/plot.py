import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv("results.csv")

# Ensure numeric types
df["threads"] = df["threads"].astype(int)
df["nodes"] = df["nodes"].astype(int)
df["average_ratio"] = df["average_ratio"].astype(float)

# Save directory
outdir = "plots"

import os
os.makedirs(outdir, exist_ok=True)

options = df["option"].unique()

for opt in options:
    subset = df[df["option"] == opt]
    pivot = subset.pivot(index="threads", columns="nodes", values="average_ratio")

    plt.figure(figsize=(8, 5))
    c = plt.imshow(
        pivot,
        origin="lower",
        aspect="auto",
        extent=[
            pivot.columns.min(),
            pivot.columns.max(),
            pivot.index.min(),
            pivot.index.max(),
        ],
        cmap="viridis",
    )
    plt.colorbar(c, label="Average Time Ratio")
    plt.title(f"Option {opt.upper()}: Time Ratio vs Threads and Nodes")
    plt.xlabel("Num Nodes per Thread")
    plt.ylabel("Num Threads")
    plt.tight_layout()
    plt.savefig(f"{outdir}/heatmap_{opt}.png", dpi=300)
    plt.close()
