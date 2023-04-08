build() {
	mkdir -p build
	cd build
	cmake ..
	make all
	compilation_result=$?
	cd ..

	if [ $compilation_result -ne 0 ]
	then
		# Compilation failed, we probably want to stop here in that case.
		echo "\e[31m\e[1mCompilation failed >.<\e[22m\e[39m"
		exit 1
	fi
}


help_function() {
	echo "aled"
}

run() {
	cd bin
	./wtf $@
	cd ..
}


build
## Parsing Command Line arguments
while getopts ':rh?' a
do
	case $a in
		r) shift $(($OPTIND - 1)); run "$@";;
		h) help_function;;
	esac
done
