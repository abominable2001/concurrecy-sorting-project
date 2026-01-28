# tools/plot_results.py
# Usage:
#   python3 tools/plot_results.py results.csv 1000000
# The script saves:
#   figures/figure1_execution_time_with_threads_N1000000.png
#   figures/figure2_mergesort_time_vs_threads.png
#   figures/figure3_quicksort_time_vs_threads.png
#   figures/figure4_speedup_vs_threads_N1000000.png
#   figures/figure5_efficiency_vs_threads_N1000000.png

import sys, csv, os

# Use a non-GUI backend so this works inside Cursor or any headless env
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

if len(sys.argv) < 3:
    print("usage: python3 tools/plot_results.py results.csv <N_for_key_figs>")
    sys.exit(1)

csv_path = sys.argv[1]
N_focus = int(sys.argv[2])

# Ensure output dir exists
out_dir = os.path.join(os.path.dirname(os.path.dirname(csv_path)), "figures") \
          if os.path.dirname(csv_path) else "figures"
os.makedirs(out_dir, exist_ok=True)

rows = []
with open(csv_path) as f:
    rd = csv.DictReader(f)
    for r in rd:
        rows.append({
            "ALG": r["ALG"].strip(),     # "QS" or "MS"
            "MODE": r["MODE"].strip(),   # "SEQ" or "PAR"
            "N": int(r["N"]),
            "T": int(r["T"]),
            "TIME_MS": float(r["TIME_MS"])
        })

def subset(N=None, ALG=None, MODE=None):
    out = rows
    if N is not None: out = [x for x in out if x["N"]==N]
    if ALG is not None: out = [x for x in out if x["ALG"]==ALG]
    if MODE is not None: out = [x for x in out if x["MODE"]==MODE]
    return out

def times_vs_threads(N, ALG):
    seq = subset(N, ALG, "SEQ")
    par = subset(N, ALG, "PAR")
    t_seq = seq[0]["TIME_MS"] if seq else None
    par_sorted = sorted(par, key=lambda x: x["T"])
    T = [x["T"] for x in par_sorted]
    Y = [x["TIME_MS"] for x in par_sorted]
    return t_seq, T, Y

def speedup_vs_threads(N, ALG):
    t_seq, T, Y = times_vs_threads(N, ALG)
    if t_seq is None or len(T)==0: return [], []
    S = [t_seq / y for y in Y]
    return T, S

def efficiency(T, S):
    return [s/t if t>0 else 0.0 for s,t in zip(S,T)]

# -------- Figure 1: Execution time with Threads (for N_focus) --------
plt.figure()
for ALG in ["QS","MS"]:
    t_seq, T, Y = times_vs_threads(N_focus, ALG)
    if T:
        plt.plot(T, Y, marker='o', label=f"{ALG}-PAR")
    if t_seq is not None:
        plt.scatter([1],[t_seq], marker='x', label=f"{ALG}-SEQ")
plt.xlabel("Threads (T)")
plt.ylabel("Time (ms)")
plt.title(f"Figure 1: Execution time with Threads (N={N_focus})")
plt.grid(True); plt.legend()
f1 = os.path.join(out_dir, f"figure1_execution_time_with_threads_N{N_focus}.png")
plt.savefig(f1, dpi=160)

# -------- Figure 2: Mergesort (execution time vs threads) ----------
plt.figure()
Ns = sorted(set(x["N"] for x in rows))
for N in Ns:
    _, T, Y = times_vs_threads(N, "MS")
    if T:
        plt.plot(T, Y, marker='o', label=f"N={N}")
plt.xlabel("Threads (T)")
plt.ylabel("Time (ms)")
plt.title("Figure 2: Merge sort (execution time vs threads)")
plt.grid(True); plt.legend()
f2 = os.path.join(out_dir, "figure2_mergesort_time_vs_threads.png")
plt.savefig(f2, dpi=160)

# -------- Figure 3: Quicksort (execution time vs threads) ----------
plt.figure()
for N in Ns:
    _, T, Y = times_vs_threads(N, "QS")
    if T:
        plt.plot(T, Y, marker='o', label=f"N={N}")
plt.xlabel("Threads (T)")
plt.ylabel("Time (ms)")
plt.title("Figure 3: Quick Sort (execution time vs threads)")
plt.grid(True); plt.legend()
f3 = os.path.join(out_dir, "figure3_quicksort_time_vs_threads.png")
plt.savefig(f3, dpi=160)

# -------- Figure 4: Speedup vs threads (for N_focus) ---------------
plt.figure()
for ALG in ["QS","MS"]:
    T, S = speedup_vs_threads(N_focus, ALG)
    if T:
        plt.plot(T, S, marker='o', label=f"{ALG}")
plt.xlabel("Threads (T)")
plt.ylabel("Speedup (SEQ time / PAR time)")
plt.title(f"Figure 4: Speedup vs Threads (N={N_focus})")
plt.grid(True); plt.legend()
f4 = os.path.join(out_dir, f"figure4_speedup_vs_threads_N{N_focus}.png")
plt.savefig(f4, dpi=160)

# -------- Figure 5: Parallel efficiency vs threads (for N_focus) ---
plt.figure()
for ALG in ["QS","MS"]:
    T, S = speedup_vs_threads(N_focus, ALG)
    if T:
        E = efficiency(T, S)
        plt.plot(T, E, marker='o', label=f"{ALG}")
plt.xlabel("Threads (T)")
plt.ylabel("Parallel efficiency (speedup/T)")
plt.title(f"Figure 5: Parallel Efficiency vs Threads (N={N_focus})")
plt.grid(True); plt.legend()
f5 = os.path.join(out_dir, f"figure5_efficiency_vs_threads_N{N_focus}.png")
plt.savefig(f5, dpi=160)

print("\nSaved figures:")
for p in [f1,f2,f3,f4,f5]:
    print(" -", p)
print("\nOpen these PNGs from the 'figures' folder and insert into your report.")