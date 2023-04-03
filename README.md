# Exposure Fusion (C++ Implementation)

### Basic Intro

Exposure fusion technique is for HDR image fusion task, which merely relies on simple quality measures, like saturation and contrast, which prove to be very
effective. This technique does not need to convert LDR images to HDR first and then use tone mapping to display HDR in LDR range, it use LDRs of multiple exposures to generate fused LDR output directly.

Exposure fusion uses Laplacian Pyramid fusion to conduct seamless combination. The alpha mask is calculated based on the contrast, saturation and well-exposedness of the LDR images.

### Preliminary Results

input multi-exposure images

![multi_expo](./assets/multi_exposure.png)

fused output image

![output](./assets/fused_image.png)

### Codebase Usage

simply run `sh run.sh` in the repo root folder to build and run the demo using the sample data from `./test_data/samples`, the results will be saved in `./test_data/result` folder.

### Authors

jiazhuang (jzsherlock@163.com)

### Reference

```latex
@inproceedings{mertens2009exposure,
  title={Exposure fusion: A simple and practical alternative to high dynamic range photography},
  author={Mertens, Tom and Kautz, Jan and Van Reeth, Frank},
  booktitle={Computer graphics forum},
  volume={28},
  number={1},
  pages={161--171},
  year={2009},
  organization={Wiley Online Library}
}
```