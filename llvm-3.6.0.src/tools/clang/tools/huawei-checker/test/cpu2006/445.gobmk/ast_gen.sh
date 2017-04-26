clang -emit-ast -o ast/sgf_utils.ast   -I. -I.. -I../include -I./include                  -O3     sgf/sgf_utils.c
clang -emit-ast -o ast/sgftree.ast   -I. -I.. -I../include -I./include                  -O3     sgf/sgftree.c
clang -emit-ast -o ast/sgfnode.ast   -I. -I.. -I../include -I./include                  -O3     sgf/sgfnode.c
