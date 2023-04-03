rm -rf ./build
mkdir build
cd build
cmake ..
cmake --build .
./ExposureFusion ../test_data/samples/mask_over.jpg ../test_data/samples/mask_mean.jpg ../test_data/samples/mask_under.jpg