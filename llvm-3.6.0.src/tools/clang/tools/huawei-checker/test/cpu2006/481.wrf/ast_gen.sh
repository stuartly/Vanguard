clang -emit-ast -o ast/wrf_num_bytes_between.ast   -I. -I./netcdf/include                 -O3     wrf_num_bytes_between.c
clang -emit-ast -o ast/pack_utils.ast   -I. -I./netcdf/include                 -O3     pack_utils.c
