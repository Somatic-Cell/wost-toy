$Exe = "build\Release\mixed_boundary_benchmark.exe"
if (!(Test-Path $Exe)) { $Exe = "build\mixed_boundary_benchmark.exe" }

# $Exe = "build\Release\mixed_boundary_benchmark.exe"
# if (!(Test-Path $Exe)) { $Exe = "build\mixed_boundary_benchmark.exe" }

# & $Exe `
#   --methods mc,pathwise,step,full,bucket,block-bucket `
#   --ks 1,4,8,16 `
#   --N 1024 `
#   --M 512 `
#   --x0 0.65 `
#   --y0 0.37 `
#   --phase-bins 16 `
#   --dist-bins 1 `
#   --group-every 1 `
#   --block-size 256 `
#   --out results\summary_x065_M512.csv `
#   --out-trials results\trials_x065_M512.csv

# python scripts\plot_results.py results\summary_x065_M512.csv --out-dir results\plots_x065_M512

# foreach ($x0 in @(0.35, 0.65, 0.80)) {
#   $tag = ("x{0}" -f $x0).Replace(".", "")

#   & $Exe `
#     --methods mc,pathwise,full,bucket,block-bucket `
#     --ks 1,4,8,16 `
#     --N 1024 `
#     --M 256 `
#     --x0 $x0 `
#     --y0 0.37 `
#     --phase-bins 16 `
#     --dist-bins 1 `
#     --group-every 1 `
#     --block-size 256 `
#     --out "results\summary_${tag}_M256.csv" `
#     --out-trials "results\trials_${tag}_M256.csv"

#   python scripts\plot_results.py "results\summary_${tag}_M256.csv" --out-dir "results\plots_${tag}_M256"
# }
# foreach ($B in @(4, 8, 16, 32, 64)) {
#   & $Exe `
#     --methods mc,pathwise,full,bucket `
#     --ks 1,4,8,16 `
#     --N 1024 `
#     --M 256 `
#     --x0 0.65 `
#     --y0 0.37 `
#     --phase-bins $B `
#     --dist-bins 1 `
#     --group-every 1 `
#     --out "results\summary_phasebins_${B}_x065.csv" `
#     --out-trials "results\trials_phasebins_${B}_x065.csv"

#   python scripts\plot_results.py "results\summary_phasebins_${B}_x065.csv" --out-dir "results\plots_phasebins_${B}_x065"
# }

# foreach ($B in @(4, 8, 16, 32, 64)) {
#   & $Exe `
#     --methods mc,pathwise,bucket `
#     --ks 1,4,8,16 `
#     --N 1024 `
#     --M 512 `
#     --x0 0.65 `
#     --y0 0.37 `
#     --phase-bins $B `
#     --dist-bins 1 `
#     --group-every 1 `
#     --out "results\test3\summary_phasebins_${B}_x065_M512.csv" `
#     --out-trials "results\test3\trials_phasebins_${B}_x065_M512.csv"

#   python scripts\plot_results.py "results\test3\summary_phasebins_${B}_x065_M512.csv" --out-dir "results\test3\plots_phasebins_${B}_x065_M512"
# }

foreach ($BS in @(32, 64, 128, 256, 512, 1024)) {
  & $Exe `
    --methods mc,pathwise,bucket,block-bucket `
    --ks 1,4,8,16 `
    --N 1024 `
    --M 512 `
    --x0 0.65 `
    --y0 0.37 `
    --phase-bins 16 `
    --dist-bins 1 `
    --group-every 1 `
    --block-size $BS `
    --out "results\block-size-sweep\summary_block_${BS}_x065_M512.csv" `
    --out-trials "results\block-size-sweep\trials_block_${BS}_x065_M512.csv"

  python scripts\plot_results.py "results\summary_block_${BS}_x065_M512.csv" --out-dir "results\plots_block_${BS}_x065_M512"
}