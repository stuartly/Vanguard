sed -i 's/opencc/clang/g' ast_gen.sh                             
sed -i 's/ -c / -emit-ast /g' ast_gen.sh
sed -i 's/ -o\s*\([a-zA-Z0-9_]*\)\.o / -o ast\/\1.ast /g' ast_gen.sh
sed -i 's/\s-D[a-zA-Z0-9_]*//g' ast_gen.sh
chmod +x ast_gen.sh
mkdir ast
