use nix::ifaddrs::getifaddrs;
use nix::sys::socket;
use pretty_hex::*;
use std::io::stdin;
use std::net::IpAddr;
use std::net::Ipv4Addr;
use std::net::UdpSocket;
use std::process::exit;
use std::thread;
use std::time;
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
        port: u16,

        #[structopt(
            short = "b",
            long = "bind",
            help = "network interface name or IPv4 address to bind"
        )]
        bind: String,

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
        port: u16,

        #[structopt(
            short = "b",
            long = "bind",
            help = "network interface name or IPv4 address to bind"
        )]
        bind: String,

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

fn run_app() -> Result<(), String> {
    let opt = Opt::from_args();

    let (addr, bind) = match &opt {
        Opt::Client { addr, bind, .. } => (addr, bind),
        Opt::Server { addr, bind, .. } => (addr, bind),
    };

    let saddr: Ipv4Addr = match parse_bind_param(bind) {
        Ok(s) => s,
        Err(e) => return Err(e),
    };

    let maddr: Ipv4Addr = match addr.parse() {
        Ok(s) => s,
        Err(e) => {
            return Err(format!(
                "failed to parse multicast address {}: {:?}",
                addr, e
            ))
        }
    };

    if !maddr.is_multicast() {
        return Err(format!("address {} is not multicast", addr));
    }

    match opt {
        Opt::Client { .. } => mcast_client(saddr, maddr, opt),
        Opt::Server { .. } => mcast_server(saddr, maddr, opt),
    }
}

fn parse_bind_param(param: &str) -> Result<Ipv4Addr, String> {
    // at first try to consider 'bind' param as IPv4 address
    if let Ok(saddr) = param.parse() {
        return Ok(saddr);
    }

    // then try to consider 'bind' param as network interface name
    // and return its first IPv4 address if any
    let addrs = match getifaddrs() {
        Ok(s) => s,
        Err(e) => {
            return Err(format!(
                "failed to get list of addresses for '{}': {:?}",
                param, e
            ))
        }
    };

    for ifaddr in addrs {
        if ifaddr.interface_name != *param {
            continue;
        }

        match ifaddr.address {
            Some(address) => match address {
                socket::SockAddr::Inet(s) => match s.to_std().ip() {
                    IpAddr::V4(s) => {
                        return Ok(s);
                    }
                    _ => continue,
                },
                _ => continue,
            },
            None => continue,
        }
    }

    Err(format!(
        "{} is neither IPv4 address nor interface name",
        param
    ))
}

fn mcast_client(saddr: Ipv4Addr, maddr: Ipv4Addr, opt: Opt) -> Result<(), String> {
    let (port, message, hops, loopback, join, cont) = match opt {
        Opt::Client {
            port,
            message,
            hops,
            loopback,
            join,
            cont,
            ..
        } => (port, message, hops, loopback, join, cont),
        Opt::Server { .. } => return Err(format!("unexpected enum variant: {:?}", opt)),
    };

    println!(
        "Client: saddr {:?}, maddr {:?}, port {}, message {}, hops {}, loopback {}, join {}, cont {}",
        saddr, maddr, port, message, hops, loopback, join, cont
    );

    let sock = match UdpSocket::bind((saddr, 0)) {
        Ok(s) => s,
        Err(e) => {
            return Err(format!(
                "failed to bind socket to multicast address: {:?}",
                e
            ))
        }
    };

    if let Err(e) = sock.set_multicast_ttl_v4(hops) {
        return Err(format!("failed to set multicast TTL: {:?}", e));
    }

    if let Err(e) = sock.set_multicast_loop_v4(loopback) {
        return Err(format!("failed to set mcast loop: {:?}", e));
    }

    if join {
        if let Err(e) = sock.join_multicast_v4(&maddr, &saddr) {
            return Err(format!("failed to join mutlicast group: {:?}", e));
        }
    }

    let mut nop = String::new();
    let mut cnt: u32 = 0;

    loop {
        if cont {
            let delay = time::Duration::from_millis(1000);
            thread::sleep(delay);
            println!("sending multicast next packet...");
        } else {
            println!("press enter to send multicast next packet:");
            if let Err(e) = stdin().read_line(&mut nop) {
                return Err(format!("stdio failure: {:?}", e));
            };

            if nop.is_empty() {
                println!("done...");
                break;
            }
        }

        match sock.send_to(format!("{}:{}\n", message, cnt).as_bytes(), (maddr, port)) {
            Ok(_) => {}
            Err(e) => return Err(format!("failed to send multicast packet: {:?}", e)),
        };

        cnt += 1;
    }

    Ok(())
}

fn mcast_server(saddr: Ipv4Addr, maddr: Ipv4Addr, opt: Opt) -> Result<(), String> {
    let (port, dump) = match opt {
        Opt::Client { .. } => return Err(format!("unexpected enum variant: {:?}", opt)),
        Opt::Server { port, dump, .. } => (port, dump),
    };

    println!(
        "Server: saddr {:?}, maddr {:?}, dump {}",
        saddr, maddr, dump
    );

    let sock = match UdpSocket::bind((maddr, port)) {
        Ok(s) => s,
        Err(e) => {
            return Err(format!(
                "failed to bind socket to multicast address: {:?}",
                e
            ))
        }
    };

    if let Err(e) = sock.join_multicast_v4(&maddr, &saddr) {
        return Err(format!("failed to join mutlicast group: {:?}", e));
    }

    let mut buf: [u8; 256] = [0; 256];

    loop {
        let (size, peer) = match sock.recv_from(&mut buf) {
            Ok((s, p)) => (s, p),
            Err(e) => return Err(format!("recv_from failed: {:?}", e)),
        };

        if dump {
            println!("{:?}", buf[0..size].to_vec().hex_dump());
        } else {
            println!(
                "recv {} bytes from {:?}: {}",
                size,
                peer,
                String::from_utf8_lossy(&buf)
            );
        }

        for e in buf.iter_mut() {
            *e = 0;
        }
    }
}

fn main() {
    exit(match run_app() {
        Ok(_) => 0,
        Err(err) => {
            eprintln!("error: {:?}", err);
            -1
        }
    });
}
