$Exe = "build-openqmc\Release\mixed_boundary_benchmark.exe"
if (!(Test-Path $Exe)) { $Exe = "build-openqmc\mixed_boundary_benchmark.exe" }

foreach ($B in @(8, 16, 32, 64)) {
  foreach ($D in @(1, 2, 4, 8)) {
    & $Exe `
      --qmc-backend openqmc-sobol `
      --methods mc,pathwise,full,bucket `
      --ks 1,4,8,16 `
      --N 1024 `
      --M 512 `
      --x0 0.65 `
      --y0 0.37 `
      --phase-bins $B `
      --dist-bins $D `
      --group-every 1 `
      --out "results\summary_openqmc_B${B}_D${D}.csv" `
      --out-trials "results\trials_openqmc_B${B}_D${D}.csv"
  }
}