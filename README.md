# wost-toy

CPU standalone prototype for randomized QMC experiments on a manufactured mixed-boundary Walk-on-Spheres toy problem.

The goal is to test whether **state-grouped / bucketed RQMC** can outperform pathwise RQMC in mixed-boundary settings before integrating anything into Zombie/FCPW.

## Model problem

Domain:

\[
\Omega=[0,1]^2
\]

Boundary conditions:

\[
u=0\quad x=0,
\]

\[
u=\cos(k\pi y)\quad x=1,
\]

\[
\partial_n u=0\quad y=0,1.
\]

Exact solution:

\[
u_k(x,y)=\frac{\sinh(k\pi x)}{\sinh(k\pi)}\cos(k\pi y).
\]

The homogeneous Neumann boundaries are handled by reflection/unfolding, so the actual walk is performed in the infinite strip with absorbing boundaries at `x=0,1`.

## Methods

- `mc`: ordinary independent Monte Carlo.
- `pathwise-rqmc`: cheap pathwise RQMC baseline using shifted Halton dimensions indexed by walker id and step.
- `step-rqmc-no-sort`: step-wise RQMC assigned to alive walkers without state sorting; this is a negative baseline.
- `full-sort-array-rqmc`: state-sorted Array-RQMC using a full CPU sort; treat this as an oracle / upper-bound variant.
- `bucketed-rqmc`: proposed lightweight state-grouped RQMC using boundary side, boundary phase and distance buckets.
- `block-bucketed-rqmc`: CPU simulation of GPU-style block-local grouping.

The current QMC engine is intentionally dependency-free. It uses shifted Halton/van der Corput / simple rank-1 lattice style points. OpenQMC can be added later as a higher-quality sampler backend.

## Build on Windows

### Visual Studio Developer PowerShell

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Run

```powershell
.\build\Release\mixed_boundary_benchmark.exe --methods all --ks 1,4,8,16 --N 1024 --M 128 --out results\summary.csv --out-trials results\trials.csv
```

Generate plots:

```powershell
python scripts\plot_results.py results\summary.csv --out-dir results\plots
```

Or run the helper script:

```powershell
.\scripts\run_phase2.ps1
```

## Build on Linux/macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/mixed_boundary_benchmark --methods all --ks 1,4,8,16 --N 1024 --M 128 --out results/summary.csv --out-trials results/trials.csv
python scripts/plot_results.py results/summary.csv --out-dir results/plots
```

## Useful command-line options

```text
--methods all,mc,pathwise,step,full,bucket,block-bucket
--ks 1,4,8,16
--N 1024
--M 128
--eps 1e-4
--phase-bins 16
--dist-bins 1
--group-every 1
--block-size 256
--out results/summary.csv
--out-trials results/trials.csv
```

## What to inspect first

1. `variance_ratio_vs_mc`: pure variance reduction.
2. `work_ratio_vs_mc`: variance × runtime trade-off.
3. `pathwise-rqmc` vs `bucketed-rqmc`: this is the main comparison.
4. `full-sort-array-rqmc`: use as an upper-bound, not as the practical method.

## Next implementation targets

- Sweep `phaseBins={4,8,16,32,64}`.
- Add `groupEvery={1,2,4,8}`.
- Add block-local grouping experiments with `blockSize={32,64,128,256,512}`.
- Replace the dependency-free QMC engine with OpenQMC as an optional backend.
- Add a 2D polygon/FCPW backend after the toy results stabilize.
