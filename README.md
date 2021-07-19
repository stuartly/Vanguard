
## Install

+ install llvm-3.6.0 and clang-3.6.0
	+ download llvm-3.6.0 and clang-3.6.0 from the [link](http://releases.llvm.org/download.html#3.6.0)
	+ install them as the [tutorial](https://clang.llvm.org/get_started.html)


+ install missing check tool
	```
	cp -rf missing-check/Vanguard llvm-3.6.0/llvm-3.6.0.src/tools/clang/tools/
	vim llvm-3.6.0/llvm-3.6.0.src/tools/clang/tools/Cmakelist.txt and add "add_subdirectory(Vanguard)"
	cd llvm-3.6.0
	mkdir llvm-3.6.0.build
	cd llvm-3.6.0.build
	cmake ../llvm-3.6.0.src
	make -j10
	sudo make install
	```



## Usage
	./Vanguard astList.txt config.json


## Demo
	cd Demo/tainted
	./linux_x86.sh
	./llvm-3.6/llvm-3.6.0.build/bin/Vanguard Demo/tainted/astList.txt Demo/tainted/config.json
	
## Reference
```
[1] Situ L Y, Wang L Z, Liu Y, et al. Automatic Detection and Repair Recommendation for Missing Checks[J]. Journal of Computer Science and Technology, 2019, 34(5): 972-992.
[2] Situ L, Wang L, Liu Y, et al. Vanguard: Detecting missing checks for prognosing potential vulnerabilities[C]//Proceedings of the Tenth Asia-Pacific Symposium on Internetware. 2018: 1-10.]
```





