use std::str;
use std::io::prelude::*;
use std::net::{TcpListener, TcpStream};
use std::thread;

fn print_u8_slice(slice: &[u8]) {
    for b in slice{
        print!("{}", *b as char);
    }
}

fn response() -> String {
    //let html = String::from("img <img src='abc.png' /> <img src='def.jpg' />");
    //let html = String::from("img <img src='abc.png' />");
    let html = String::from("link <a href='abc.png'>abc</a>");
    // let html = String::from("hello world");
    let content_length_header = format!("Content-Length: {}\r\n", html.len());
    let content_type_header = format!("Content-Type: text/html\r\n");

    let mut data = String::from("");
    data.push_str("HTTP/1.1 200 OK\r\n");
    data.push_str("Server: rust_server/0.1.0\r\n");
    data.push_str("Connection: Closed\r\n");
    data.push_str(&content_length_header);
    data.push_str(&content_type_header);
    data.push_str("\r\n");
    data.push_str(&html);

    data
}

fn not_found() -> String {
    let mut data = String::from("");
    data.push_str("HTTP/1.1 404 Not Found\r\n");
    data.push_str("Connection: Closed\r\n");
    data.push_str("\r\n");

    data
}

fn handle_request(req: &String) {
    println!("{}", req);
}

fn print_request(req: &String) {
    println!("------req------");
    print!("{}", req);
    println!("---------------");
}

fn handle_client(mut stream: TcpStream) {
    let mut req = String::from("");
    let mut i = 0;
    loop {
        let mut buf = [0u8; 1024];
        let got = stream.read(&mut buf).unwrap();
        if got == 0 {
            break;
        } else {
            req.push_str(str::from_utf8(&buf[0 .. got]).unwrap());
            if req.ends_with("\r\n\r\n") {
                print_request(&req);

                if i == 0 {
                    stream.write(response().as_bytes()).unwrap();
                    println!("response 202 ok!");
                    i += 1;
                } else {
                    stream.write(not_found().as_bytes()).unwrap();
                    println!("response 404 not found!");
                    i += 1;
                }
            }
        }
    }

    println!("handling clinet done!");
}

fn main() {

    if false {
        let str = format!("Hello, {}!", "world");
        println!("{}", str);
        println!("{}", response());
    } else {
        let listener = TcpListener::bind("192.169.172.134:80").unwrap();
        for stream in listener.incoming() {
            match stream {
                Ok(stream) => {
                    println!("a stream coming");
                    thread::spawn(move|| {
                        handle_client(stream)
                    });
                }
                Err(e) => {
                    println!("{}, connection failed", e);
                }
            }
        }

        drop(listener);
        println!("close socket!");
    }
}
