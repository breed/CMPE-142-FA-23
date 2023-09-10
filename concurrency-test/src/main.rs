use clickrs::command;
use nix::unistd;
use std::error::Error;
use nix::sys::stat;
use std::ffi::CString;
use std::process::exit;
use std::collections::HashMap;
use std::sync::Mutex;
use std::fs::{canonicalize, remove_file};
use std::process::Command;
use std::path::PathBuf;
use std::io::BufRead;
use lazy_static::lazy_static;
use std::future::Future;

// we want to delete the named pipes at the end
lazy_static! {
    static ref TO_DELETE: Mutex<HashMap<PathBuf, i32>> = Mutex::new(HashMap::new());
}

fn finish() {
    println!("removing files");
    for k in TO_DELETE.lock().unwrap().keys() {
        println!("removing {}", k.display());
        remove_file(k);
    }
}

fn mkpipe(prefix: &String, i: i32) -> impl Future<Output = ()> {
    let file_str = format!("{}{}", prefix, i);
    let file_cstring = CString::new(file_str.as_str()).unwrap();
    let rc = unistd::mkfifo(file_cstring.as_c_str(), stat::Mode::S_IRWXU);
    perror(&file_str, rc);
    println!("creating {}!", &file_str);
    TO_DELETE.lock().unwrap().insert(canonicalize(&file_str).unwrap(), 1);
    let boxed_file_str = Box::new(file_str);
    async move {
        async_fs::OpenOptions::new().write(true).read(false).open(*boxed_file_str.clone()).await;
        println!("opened {}", *boxed_file_str)
    }
}

// pointer from https://stackoverflow.com/questions/57860613/how-to-add-a-shutdown-hook-to-a-rust-program
struct Cleanup;

impl Drop for Cleanup {
    fn drop(&mut self) {
        finish();
    }
}

#[command(name = "test_concurrency", about="create a bunch of named pipes and then see if a child process start to read those pipes concurrently.")]
#[argument("prefix", help="the prefix of the generated named pipes")]
#[argument("count", help="the number of generated named pipes. names will take the form prefixC, were C is the named pipe number starting at 0.")]
#[argument("command", help="the command to run")]
#[argument("args", help="arguments to pass to the command")]
fn main(
    prefix: String,
    count: i32,
    command: String,
    args: Vec<String>
) {
    // create the named pipes
    let _cleanup = Cleanup;
    let mut pipes = Vec::new();
    for i in 0..count {
        pipes.push(mkpipe(&prefix, i));
    }

    // create the command process
    let mut cmd = Command::new(command);
    cmd.args(args);
    let mut child = perror(&format!("starting {:?}", &cmd), cmd.spawn());
    println!("child pid {}", child.id());
    
    futures::executor::block_on(futures::future::join_all(pipes));

    std::thread::sleep(std::time::Duration::from_secs(2));

    let pid_re = regex::Regex::new(r"^/proc/([0-9]+)$").unwrap();

    // find children of the command in /proc index by ppid
    let mut procs = HashMap::new();

    for entry in std::fs::read_dir("/proc").unwrap() {
        let entry = entry.unwrap();
        if entry.path().is_dir() && pid_re.is_match(entry.path().to_str().unwrap()) {
            let status = format!("{}/{}", entry.path().to_str().unwrap(), "status");
            if let Ok(file) = std::fs::File::open(status) {
                let mut ppid: u32 = 0;
                let mut pid: u32 = 0;
                for line in std::io::BufReader::new(file).lines() {
                    if let Ok(data) = line {
                        let mut split = data.split('\t');
                        if let Some(tag) = split.next() {
                            if tag == "Pid:" {
                                pid = split.next().unwrap().parse().unwrap();
                            }
                            if tag == "PPid:" {
                                ppid = split.next().unwrap().parse().unwrap();
                            }
                        }
                    }
                }
                let mut e = procs.entry(ppid).or_insert(Vec::new());
                e.push(pid);
            }
        }
    };

    // use procs to get the descendants of the child
    let mut descendants = Vec::new();
    collect_descendants(child.id(), &mut descendants, &procs);
    println!("{:?}", &descendants);

    let mut reading_files = Vec::new();
    // system call number 0 is read
    for pid in descendants {
        let syscall_path = format!("/proc/{}/syscall", pid);
        let line = std::fs::read_to_string(&syscall_path);
        match line {
        Ok(line) => {
            println!("{}", line);
            let mut parts = line.split(' ');
            let nr = parts.next().unwrap_or("-1");
            if nr != "0" { continue; } 
            let fd: i32 = parts.next().unwrap_or("0x-1").get(2..).unwrap().parse().unwrap();
            let link = std::fs::read_link(format!("/proc/{}/fd/{}", pid, fd));
            println!("{:?}", link);
            reading_files.push(link.unwrap());
        }
        Err(err) => {
            println!("problem with {} {}", syscall_path, err);
        }
        }
    }

    let mut map = TO_DELETE.lock().unwrap();
    println!("{:?}", map);
    println!("{:?}", reading_files);
    for file in reading_files {
        if map.contains_key(&file) {
            map.insert(file, 0);
        } else {
            println!("skipping {}", file.to_str().unwrap());
        }
    }
    child.kill();
    child.wait();
    for (k, v) in map.iter() {
        if *v != 0 {
            println!("not reading {}", k.to_str().unwrap());
        }
    }
}

fn collect_descendants(pid: u32, descendants: &mut Vec<u32>, procs: &HashMap<u32, Vec<u32>>) {
    descendants.push(pid);
    if let Some(cpids) = procs.get(&pid) {
        for cpid in cpids {
            let p: u32 = *cpid;
            collect_descendants(p, descendants, procs)
        }
    }
}


fn perror<T, E: Error>(desc: &String, result: Result<T,E>) -> T {
    match result {
        Ok(r) => r,
        Err(e) => {
            println!("{}: {:?}", desc, e);
            finish();
            exit(2)
        }
    }
}
