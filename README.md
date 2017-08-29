## Vanguard: Detecting Missing Check for Identifying Insufficient Attack Protection


## Complier:
+ put huawei-checker to the directory of llvm-3.6.0.src/tools/clang/tools
+ mkdir llvm-3.6.0-build
+ cd llvm-3.6.0-build
+ cmake ../llvm-3.6.0.src
+ make


## Usage:
+ huawei-checker astList.txt config.json


## Example:
+ cd Test-code/tainted
+ ./linux_x86.sh
+ huawei-checker astList.txt config.json

