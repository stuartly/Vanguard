## Missing Check


## Install

+ install llvm-3.6.0 and clang-3.6.0
	+ download llvm-3.6.0 and clang-3.6.0 from the [link](http://releases.llvm.org/download.html#3.6.0)
	+ install them as the [tutoral](https://clang.llvm.org/get_started.html)


+ install missing check tool

	```
	cp -rf missing-check/huawei-checker llvm-3.6.0/llvm-3.6.0.src/tools/clang/tools/
	vim llvm-3.6.0/llvm-3.6.0.src/tools/clang/tools/Cmakelist.txt and add "add_subdirectory(huawei-checker)"
	cd llvm-3.6.0
	mkdir llvm-3.6.0.build
	cd llvm-3.6.0.build
	cmake ../llvm-3.6.0.src
	make -j10
	sudo make install
	```


## Usage
	```
	./huawei-checker astList.txt config.json
	```


## Demo
	```
	cd Demo/tainted
	./linux_x86.sh
	./llvm-3.6/llvm-3.6.0.build/bin/huawei-checker Demo/tainted/astList.txt Demo/tainted/config.json
	```

## Reference
	```
	@inproceedings{situ2018detecting,
	  title={Detecting missing checks for identifying insufficient attack protections},
	  author={Situ, Lingyun and Zou, Liang and Wang, Linzhang and Liu, Yang and Mao, Bing and Li, Xuandong},
	  booktitle={Proceedings of the 40th International Conference on Software Engineering: Companion Proceeedings},
	  pages={238--239},
	  year={2018},
	  organization={ACM}
	}
	```

