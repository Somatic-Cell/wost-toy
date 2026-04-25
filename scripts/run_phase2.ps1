# Run from repository root after building with CMake.
$Exe = "build\Release\mixed_boundary_benchmark.exe"
if (!(Test-Path $Exe)) { $Exe = "build\mixed_boundary_benchmark.exe" }

& $Exe --methods all --ks 1,4,8,16 --N 1024 --M 128 --phase-bins 16 --dist-bins 1 --group-every 1 --out results\summary.csv --out-trials results\trials.csv
python scripts\plot_results.py results\summary.csv --out-dir results\plots
