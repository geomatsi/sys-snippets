# default qemu gdb server local address:port tuple
target remote :1234

# load python scripts with custom gdb commands
source scripts/hello.py
source scripts/stack.py
