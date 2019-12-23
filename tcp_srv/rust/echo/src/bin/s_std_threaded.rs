use std::io::BufRead;
use std::io::BufReader;
use std::io::Write;
use std::net::SocketAddr;
use std::net::TcpListener;
use std::net::TcpStream;
use std::thread;

fn main() {
    let addr = SocketAddr::from(([127, 0, 0, 1], 10000));
    let listener = match TcpListener::bind(addr) {
        Ok(s) => s,
        Err(e) => panic!("Listener failed: {:?}", e),
    };

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                thread::spawn(move || {
                    serve(stream);
                });
            }
            Err(e) => panic!("Accept failed: {:?}", e),
        }
    }
}

fn serve(mut stream: TcpStream) {
    let addr = match stream.peer_addr() {
        Ok(s) => s,
        Err(e) => {
            println!("err: failed to get connection info: {:?}", e);
            return;
        }
    };

    println!("{:?}: new client", addr);

    let mut r_stream = match stream.try_clone() {
        Ok(s) => s,
        Err(e) => {
            println!("err: {:?}: stream clone failed: {:?}", addr, e);
            return;
        }
    };

    let mut rb_stream = BufReader::new(&mut r_stream);
    let mut msg = String::new();

    loop {
        if let Err(e) = rb_stream.read_line(&mut msg) {
            println!("err: {:?}: write failed: {:?}", addr, e);
            return;
        }

        if msg.is_empty() {
            println!("{:?}: exit...", addr);
            return;
        }

        println!("client {:?}: {}", addr, msg);

        if let Err(e) = stream.write(msg.as_bytes()) {
            println!("err: {:?}: writing to stream failed: {:?}", addr, e);
            return;
        }

        msg.clear();
    }
}
