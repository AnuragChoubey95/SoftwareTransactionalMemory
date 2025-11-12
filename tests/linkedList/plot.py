#!/usr/bin/env python3
import pandas as pd
import plotly.graph_objects as go

df = pd.read_csv("results.csv")

fig = go.Figure()

for opt, color in zip(['m', 'f', 'a'], ['blue', 'red', 'green']):
    sub = df[df['option'] == opt]
    pivot = sub.pivot(index='nodes', columns='threads', values='average_ratio')
    x = pivot.columns
    y = pivot.index
    z = pivot.values

    fig.add_trace(go.Surface(
        x=x, y=y, z=z,
        colorscale='Viridis',
        name=f"{opt.upper()} surface",
        opacity=0.6,
        showscale=False
    ))

    for t in x:
        line_y = y
        line_z = pivot[t].values
        fig.add_trace(go.Scatter3d(
            x=[t]*len(line_y), y=line_y, z=line_z,
            mode='lines',
            line=dict(color=color, width=4),
            name=f"{opt.upper()} line (threads={t})"
        ))

fig.update_layout(
    scene=dict(
        xaxis_title='Threads',
        yaxis_title='Nodes per Thread',
        zaxis_title='Average Ratio'
    ),
    title='STM vs Mutex, FineGrained and Atomic Ops'
)

fig.write_html("interactive_plot.html", include_plotlyjs="cdn")
print("Saved interactive_plot.html")
