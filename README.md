# mesa-band-cracker
find structure seeds with user specified mesa band colors. could be useful for seedcracking/seedfinding

terracottaSim117.c uses an optimization that makes it a lot faster, but has a very small chance to miss structure seeds
terracottaSim117NOOPT.c dosent have that optimization, so it may be useful for crosschecking

both are multithreaded. compile with
gcc <filename>.c -o <filename> -Ofast -lpthread
