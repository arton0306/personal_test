use std::env;
use std::path::Path;

fn get_db_path() -> Option<String> {
    let default_path = env::home_dir()
                           .map(|mut p| {
                               p.push(".rustjump.db");
                               p.to_str().unwrap().to_string()
                           });
    env::var_os("RUSTJUMP_DB_PATH")
        .map(|x| x.to_str().unwrap().to_string())
        .or_else(|| default_path)
}

fn get_record_cnt() -> u32 {
    let default_record_cnt = 100;
    env::var_os("RUSTJUMP_RECORD_COUNT")
        .map(|x| x.to_str().unwrap().to_string()
                  .parse::<u32>()
                  .ok().expect("RUSTJUMP_RECORD_COUNT must be an integer!"))
        .or_else(|| Some(default_record_cnt))
        .unwrap()
}

struct Record {
    score: f32,
    path: Path,
}

struct Db {
    // filepath: Path,
    record: Vec<Record>,
}

fn main() {
    let notfound_msg = "Cannot find env RUSTJUMP_DB_PATH and HOME dir. Please set one.";
    println!("rust jump db path: {}", get_db_path().expect(notfound_msg));
    println!("rust jump record count: {}", get_record_cnt());
}
