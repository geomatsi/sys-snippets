use std::io::{stdin, BufRead, BufReader, Write};
use std::net::TcpStream;

fn main() {
    let mut stream = match TcpStream::connect("127.0.0.1:10000") {
        Ok(s) => s,
        Err(e) => panic!("Connection failed: {:?}", e),
    };

    let mut r_stream = match stream.try_clone() {
        Ok(s) => s,
        Err(e) => panic!("Stream clone failed: {:?}", e),
    };

    let addr = match stream.local_addr() {
        Ok(s) => s,
        Err(e) => panic!("Failed to get connection info: {:?}", e),
    };

    let mut rb_stream = BufReader::new(&mut r_stream);
    let mut msg = String::new();

    loop {
        println!("Enter string:");
        if let Err(e) = stdin().read_line(&mut msg) {
            panic!("Read stdin failed: {:?}", e)
        };

        if msg.is_empty() {
            println!("client {:?} completed", addr);
            return;
        }

        if let Err(e) = stream.write(msg.as_bytes()) {
            panic!("Writing to stream failed: {:?}", e)
        }

        print!("xmit: {}", msg);
        msg.clear();

        if let Err(e) = rb_stream.read_line(&mut msg) {
            panic!("Writing to stream failed: {:?}", e)
        }

        print!("recv: {}", msg);
        msg.clear();
    }
}
