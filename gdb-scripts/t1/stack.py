import gdb


class StackTrace(gdb.Command):
    """
    Single step and dump the stack pointer continuously until a breakpoint is hit.
    Stack pointer values are written to the file specified in the argument.
    """

    def __init__(self):
        super(StackTrace, self).__init__("stack_tracer", gdb.COMMAND_USER)

    def complete(self, text, word):
        # the argument should be a single file name
        return gdb.COMPLETE_FILENAME

    def invoke(self, args, from_tty):
        outfile = "stack.log"

        if len(args.split()) > 0:
            outfile = args.split()[0]

        print("Dumping stack addrs to {}".format(outfile))

        # single-step until a breakpoint is hit
        bp_addresses = []
        for bp in gdb.breakpoints():
            try:
                location = gdb.parse_and_eval(bp.location)
                address = int(location.address)
            except gdb.error:
                print("Location: {}".format(bp.location))
            print("Breakpoint {}:  0x{:08x}".format(bp.number, address))
            bp_addresses.append(address)

        def hit_breakpoint():
            pc = int(gdb.parse_and_eval("$pc"))
            if pc in bp_addresses:
                print("Hit breakpoint at 0x{:08x}".format(pc))
                return True
            else:
                return False

        # TODO: read from object file using python gdb module
        ram_start = 0x80000000
        ram_end = 0x80200000

        with open(outfile, "w") as f:
            while True:
                try:
                    gdb.execute("si")
                    sp = int(gdb.parse_and_eval("$sp"))
                except gdb.error:
                    break

                # sanitize values: store only valid addrs after stack init
                if sp > ram_start:
                    f.write("{}\n".format(ram_end - sp))

                # check if we've reached a breakpoint
                if hit_breakpoint():
                    break

        f.close()


StackTrace()
