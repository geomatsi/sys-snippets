use mio::net::TcpListener;
use mio::net::TcpStream;
use mio::*;
use std::collections::HashMap;
use std::io::ErrorKind;
use std::io::{Read, Write};
use std::net::SocketAddr;

const SERVER: Token = Token(0);

struct Client {
    socket: TcpStream,
    peer_addr: SocketAddr,
    message: String,
}

fn main() {
    let addr = SocketAddr::from(([127, 0, 0, 1], 10000));

    let server = match TcpListener::bind(&addr) {
        Ok(s) => s,
        Err(e) => panic!("listener failed: {:?}", e),
    };

    let mut sockets: HashMap<Token, Client> = HashMap::new();
    let mut events = Events::with_capacity(1024);
    let mut counter: usize = 1;

    let poll = Poll::new().unwrap();

    poll.register(&server, SERVER, Ready::readable(), PollOpt::edge())
        .unwrap();

    loop {
        poll.poll(&mut events, None).unwrap();

        for event in events.iter() {
            match event.token() {
                SERVER => loop {
                    match server.accept() {
                        Ok((socket, peer_addr)) => {
                            let message = String::new();
                            let token = Token(counter);
                            counter += 1;

                            poll.register(&socket, token, Ready::readable(), PollOpt::edge())
                                .unwrap();
                            sockets.insert(
                                token,
                                Client {
                                    socket,
                                    peer_addr,
                                    message,
                                },
                            );
                            println!("connection accepted from {}", peer_addr);
                        }
                        Err(ref e) if e.kind() == ErrorKind::WouldBlock => break,
                        Err(e) => panic!("unexpected error: {}", e),
                    }
                },
                ref token if event.readiness().is_readable() => {
                    if let Some(client) = sockets.get_mut(token) {
                        loop {
                            match client.socket.read_to_string(&mut client.message) {
                                Ok(0) => {
                                    println!("connection from {} closed", client.peer_addr);
                                    sockets.remove(token);
                                    break;
                                }
                                Ok(_) => {
                                    println!("keep reading from {}...", client.peer_addr);
                                    continue;
                                }
                                Err(ref e) if e.kind() == ErrorKind::WouldBlock => break,
                                Err(e) => panic!("unexpected error: {}", e),
                            }
                        }
                    } else {
                        unreachable!("Could not find client with token {}", token.0);
                    }

                    if let Some(client) = sockets.get_mut(token) {
                        println!("read from {}: {}", client.peer_addr, client.message);
                        poll.reregister(
                            &client.socket,
                            *token,
                            Ready::writable(),
                            PollOpt::edge() | PollOpt::oneshot(),
                        )
                        .unwrap();
                    } else {
                        println!("client with token {} has been closed", token.0);
                    }
                }
                ref token if event.readiness().is_writable() => {
                    if let Some(client) = sockets.get_mut(token) {
                        println!("write back to {}: {}", client.peer_addr, client.message);
                        if client.socket.write_all(&client.message.as_bytes()).is_ok() {
                            client.message.clear();
                            poll.reregister(
                                &client.socket,
                                *token,
                                Ready::readable(),
                                PollOpt::edge(),
                            )
                            .unwrap();
                        } else {
                            println!(
                                "writing failed to {}, closing this socket",
                                client.peer_addr
                            );
                            sockets.remove(token);
                        }
                    } else {
                        unreachable!("Could not find client with token {}", token.0);
                    }
                }
                x => unreachable!("Could not find client with token {}", x.0),
            }
        }
    }
}
