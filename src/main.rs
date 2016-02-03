use std::net::{TcpListener, TcpStream};
use std::thread;


fn handle_client(stream: TcpStream) {
    println!("handle_client: a stream coming");
    // ...
}

// accept connections and process them, spawning a new thread for each one
fn main() {
    // let listener = TcpListener::bind("127.0.0.1:80").unwrap();
    let listener = TcpListener::bind("192.169.172.134:80").unwrap();
    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                println!("a stream coming");
                thread::spawn(move|| {
                    // connection succeeded
                    handle_client(stream)
                });
            }
            Err(e) => { /* connection failed */ }
        }
    }

    // close the socket server
    drop(listener);
}
