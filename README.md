
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



