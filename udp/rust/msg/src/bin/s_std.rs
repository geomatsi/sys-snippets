use std::net::UdpSocket;

fn main() {
    let sock = match UdpSocket::bind("127.0.0.1:20001") {
        Ok(s) => s,
        Err(e) => panic!("Connection failed: {:?}", e),
    };

    let mut buf = [0; 256];

    loop {
        let (size, peer) = match sock.recv_from(&mut buf) {
            Ok((s, p)) => (s, p),
            Err(e) => panic!("recv_from failed: {:?}", e),
        };

        println!(
            "recv {} bytes from {:?}: {}",
            size,
            peer,
            String::from_utf8_lossy(&buf)
        );

        for e in buf.iter_mut() {
            *e = 0;
        }
    }
}
