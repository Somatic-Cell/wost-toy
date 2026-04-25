.\build\Release\mixed_boundary_benchmark.exe --methods all --ks 1,4 --N 256 --M 16 --x0 0.35 --y0 0.37 --phase-bins 165 --dist-bins 1 --group-every 1 --out results\smoke_summary.csv --out-trials results\smoke_trials.csv
.\build\Release\mixed_boundary_benchmark.exe ^  
  --methods mc,pathwise,step,full,bucket ^
  --ks 1,4,8,16 ^
  --N 1024 ^
  --M 128 ^
  --x0 0.35 ^
  --y0 0.37 ^
  --phase-bins 16 ^
  --dist-bins 1 ^
  --group-every 1 ^
  --out results\summary_x035_M128.csv ^
  --out-trials results\trials_x035_M128.csv

.\build\Release\mixed_boundary_benchmark.exe ^  
--methods mc,pathwise,step,full,bucket,block-bucket ^
  --ks 1,4,8,16 ^
  --N 1024 ^
  --M 512 ^
  --x0 0.65 ^
  --y0 0.37 ^
  --phase-bins 16 ^
  --dist-bins 1 ^
  --group-every 1 ^
  --block-size 256 ^
  --out results\summary_x065_M512.csv ^
  --out-trials results\trials_x065_M512.csv