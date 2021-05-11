# __SnitchHarrier__

This tool searches arbitrary data input for AES keys based on the existence of corresponding key schedules and prints the offset position of the AES keys, that were found.

This tool is based on OpenCL and does this search in a parallel manner, which results in short search times (~ 3 seconds for a 1GB file).

SnitchHarrier supports 128-,192- and 256 bit keys.

## Usage
```
python snitchharrier.py -i <input file> -kl <keylength> [-kf <Path to file containing kernel code>]
```
keylength must be 128, 192 or 256.
If -kf is not specified, the local kernel.c file will be loaded as the default kernel
