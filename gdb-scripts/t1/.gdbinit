# default qemu gdb server local address:port tuple
target remote :1234

# load python scripts with custom gdb commands
source hello.py
source stack.py
