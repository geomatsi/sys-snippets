use std::io::stdin;
use std::net::{IpAddr, Ipv4Addr};
use std::net::{SocketAddr, UdpSocket};

fn main() {
    let sock = match UdpSocket::bind("127.0.0.1:0") {
        Ok(s) => s,
        Err(e) => panic!("Connection failed: {:?}", e),
    };

    let local_addr = match sock.local_addr() {
        Ok(s) => s,
        Err(e) => panic!("Failed to get connection info: {:?}", e),
    };

    let remote_addr = SocketAddr::new(IpAddr::V4(Ipv4Addr::new(127, 0, 0, 1)), 20001);

    let mut msg = String::new();

    loop {
        println!("Enter string:");
        if let Err(e) = stdin().read_line(&mut msg) {
            panic!("Read stdin failed: {:?}", e)
        };

        if msg.is_empty() {
            println!("client {:?} completed", local_addr);
            return;
        }

        let bytes = match sock.send_to(msg.as_bytes(), remote_addr) {
            Ok(s) => s,
            Err(e) => panic!("send_to failed: {:?}", e),
        };

        println!("xmit: {} bytes", bytes);
        msg.clear();
    }
}
