use structopt::StructOpt;

#[derive(StructOpt, Debug)]
#[structopt(
    name = "mcast",
    version = "0.1",
    about = "Multicast test tool command line options"
)]
enum Opt {
    Server {
        #[structopt(short = "a", long = "addr", help = "multicast address")]
        addr: String,

        #[structopt(
            short = "p",
            long = "port",
            help = "multicast port, default value is 9999",
            default_value = "9999"
        )]
        port: u32,

        #[structopt(short = "i", long = "ifname", help = "network interface to bind")]
        ifname: String,

        #[structopt(long = "dump", help = "server packet hexdump mode")]
        dump: bool,
    },
    Client {
        #[structopt(short = "a", long = "addr", help = "multicast address")]
        addr: String,

        #[structopt(
            short = "p",
            long = "port",
            help = "multicast port, default value is 9999",
            default_value = "9999"
        )]
        port: u32,

        #[structopt(short = "i", long = "ifname", help = "network interface to bind")]
        ifname: String,

        #[structopt(
            short = "m",
            long = "message",
            help = "client multicast payload message",
            default_value = "test"
        )]
        message: String,

        #[structopt(
            long = "hops",
            help = "multicast packet lifetime",
            default_value = "255"
        )]
        hops: u32,

        #[structopt(long = "loopback", help = "enable client transmission loopback")]
        loopback: bool,

        #[structopt(long = "join", help = "enable client membership in multicast group")]
        join: bool,

        #[structopt(long = "cont", help = "enable client continuous mode")]
        cont: bool,
    },
}

fn main() {
    let opt = Opt::from_args();

    let (addr, port, ifname) = match &opt {
        Opt::Client {
            addr, port, ifname, ..
        } => (addr, port, ifname),
        Opt::Server {
            addr, port, ifname, ..
        } => (addr, port, ifname),
    };

    // TODO: basic multicast socket setup
    println!("addr: {} port: {} ifname: {}", addr, port, ifname);

    match opt {
        Opt::Client { .. } => mcast_client(opt),
        Opt::Server { .. } => mcast_server(opt),
    };
}

fn mcast_client(opt: Opt) {
    // TODO: multicast server implementation
    println!("client processing: {:?}", opt);
}

fn mcast_server(opt: Opt) {
    // TODO: multicast client implementation
    println!("server processing: {:?}", opt);
}
