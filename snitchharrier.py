import sys
import pyopencl as cl
import numpy as np
import time


print("Using PyOpenCL Version", cl.VERSION_TEXT)

ctx = cl.create_some_context(interactive=True)
queue = cl.CommandQueue(ctx)

def print_usage():
  print("Usage:", sys.argv[0],  "-i", "<input file>", "-kl", "<keylength>", "[-kf <Path to file containing kernel code>]")
  print("keylength must be 128, 192 or 256")
  print("If -kf is not specified, the local kernel.c file will be loaded as the default kernel")


allowed_keylengths = [128, 192, 256]

if len(sys.argv) < 4:
  print_usage()
  exit()

kernel_code_input = "kernel.c"
for idx in range(0, len(sys.argv)):
  if sys.argv[idx] == "-i":
    input_file = sys.argv[idx+1]
  elif sys.argv[idx] == "-kl":
    keylength = int(sys.argv[idx+1])
    if keylength not in allowed_keylengths:
      print_usage()
      exit()
  elif sys.argv[idx] == "-kf":
    kernel_code_input = sys.argv[idx+1]


#Read memory from given file path
dmp_array = np.fromfile(input_file, dtype=np.ubyte, count=-1, sep="")
print("Read", dmp_array.nbytes, "bytes from file")

#Create global OpenCL buffer from the input bytes
dmp_array_g = cl.Buffer(ctx, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=dmp_array)

#Load kernel code from file
kernel_code_str = ""
with open(kernel_code_input) as f:
  kernel_code_str =  f.read()

#Create OpenCL kernel program
kernel_prg = cl.Program(ctx, kernel_code_str).build()

#Create buffer that will hold the result
res_g = cl.Buffer(ctx, cl.mem_flags.WRITE_ONLY, dmp_array.nbytes)

#Start the kernel program
kernel_prg.calc_schedules(queue, dmp_array.shape, None, dmp_array_g, res_g,  np.int32(keylength))