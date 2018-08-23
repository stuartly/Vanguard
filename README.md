## Missing Check


## Install
+ cp -rf missing-check/huawei-checker llvm-3.6.0.src/tools/clang/tools/
+ mkdir llvm-3.6.0-build
+ cd llvm-3.6.0-build
+ cmake ../llvm-3.6.0.src
+ make


## Usage
+ huawei-checker astList.txt config.json


## Example
+ cd Demo/tainted
+ ./linux_x86.sh
+ huawei-checker astList.txt config.json

## Reference
@inproceedings{situ2018detecting,
  title={Detecting missing checks for identifying insufficient attack protections},
  author={Situ, Lingyun and Zou, Liang and Wang, Linzhang and Liu, Yang and Mao, Bing and Li, Xuandong},
  booktitle={Proceedings of the 40th International Conference on Software Engineering: Companion Proceeedings},
  pages={238--239},
  year={2018},
  organization={ACM}
}

