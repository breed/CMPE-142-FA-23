use clickrs::command;
use futures::future::select;
use lazy_static::lazy_static;
use nix::sys::stat;
use nix::unistd;
use std::collections::HashMap;
use std::error::Error;
use std::ffi::CString;
use std::fs::{canonicalize, remove_file};
use std::future::Future;
use std::io::BufRead;
use std::path::PathBuf;
use std::process::exit;
use std::process::Command;
use std::sync::Mutex;

/*
 * for assignments that use fork start processes to read files, we want to test to
 * see if they are trying to do the reads concurrently.
 *
 * this program will:
 * 1) create the desired number of named pipes with a prefix and count.
 * 2) the last parameters are the program to run and arguments to pass. the arguments
 *    should cause the program to try to open the named pipes.
 * 3) this program will not write anything to the pipes, so the subprocesses should
 *    block on a read.
 * 4) we will go through all the subprocesses looking at the blocked system calls.
 *    we should see a process blocked on each of the files.
 * 5) list any files and are not being read and exit with the count of unread files
 */

// we want to delete the named pipes at the end
// keeping global so that the signal handle (eventually) can access it
lazy_static! {
    static ref TO_DELETE: Mutex<HashMap<PathBuf, i32>> = Mutex::new(HashMap::new());
}

// this should be called before exiting to clean up pipes
fn finish() {
    for k in TO_DELETE.lock().unwrap().keys() {
        pcheck(format!("rm {:?}", k), remove_file(k));
    }
}

// check a result and print an error. result is ignored
fn pcheck<T, E: Error>(desc: String, result: Result<T, E>) {
    if let Err(err) = result {
        println!("{}: {}", desc, err);
    }
}

// check a result. errors will be printed and exit called. result is returned
fn perror<T, E: Error>(desc: &String, result: Result<T, E>) -> T {
    match result {
        Ok(r) => r,
        Err(e) => {
            println!("{}: {:?}", desc, e);
            finish();
            exit(2)
        }
    }
}

// created a named pipe. it returns a future
fn mkpipe(prefix: &String, i: i32) -> impl Future<Output = std::io::Result<async_fs::File>> {
    let file_str = format!("{}{}", prefix, i);
    let file_cstring = CString::new(file_str.as_str()).unwrap();
    let rc = unistd::mkfifo(file_cstring.as_c_str(), stat::Mode::S_IRWXU);
    perror(&file_str, rc);
    TO_DELETE
        .lock()
        .unwrap()
        .insert(canonicalize(&file_str).unwrap(), 1);
    let boxed_file_str = Box::new(file_str);
    async move {
        let rc = async_fs::OpenOptions::new()
            .write(true)
            .read(false)
            .open(*boxed_file_str.clone())
            .await;
        // println!("opened {}", *boxed_file_str);
        rc
    }
}

#[command(
    name = "test_concurrency",
    about = "create a bunch of named pipes and then see if a child process start to read those pipes concurrently."
)]
#[argument("prefix", help = "the prefix of the generated named pipes")]
#[argument(
    "count",
    help = "the number of generated named pipes. names will take the form prefixC, were C is the named pipe number starting at 0."
)]
#[argument("command", help = "the command to run")]
#[argument("args", help = "arguments to pass to the command")]
fn main(prefix: String, count: i32, command: String, args: Vec<String>) {
    // create the named pipes
    let mut pipes = Vec::new();
    for i in 0..count {
        pipes.push(mkpipe(&prefix, i));
    }

    // create the command process
    let mut cmd = Command::new(command);
    cmd.args(args);
    let mut child = perror(&format!("starting {:?}", &cmd), cmd.spawn());

    let join = futures::future::join_all(pipes);
    let s = Box::pin(async { std::thread::sleep(std::time::Duration::from_secs(2)) });
    let _select_rc = futures::executor::block_on(select(join, s));

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
                let e = procs.entry(ppid).or_insert(Vec::new());
                e.push(pid);
            }
        }
    }

    // use procs to get the descendants of the child
    let mut descendants = Vec::new();
    collect_descendants(child.id(), &mut descendants, &procs);

    // find any files that descendants are blocked on reading
    let mut reading_files = Vec::new();
    // system call number 0 is read
    for pid in descendants {
        let syscall_path = format!("/proc/{}/syscall", pid);
        let line = std::fs::read_to_string(&syscall_path);
        match line {
            Ok(line) => {
                let mut parts = line.split(' ');
                let nr = parts.next().unwrap_or("-1");
                if nr != "0" {
                    continue;
                }
                let fd: i32 = i32::from_str_radix(parts
                    .next()
                    .unwrap_or("0x-1")
                    .get(2..)
                    .unwrap(), 16)
                    .unwrap();
                let link = std::fs::read_link(format!("/proc/{}/fd/{}", pid, fd));
                reading_files.push(link.unwrap());
            }
            Err(err) => {
                println!("problem with {} {}", syscall_path, err);
            }
        }
    }

    let rc = check_read_files(reading_files);

    // we are done now, make the child go away
    pcheck("child process couldn't be killed".to_string(), child.kill());
    pcheck("wait for child".to_string(), child.wait());

    if rc == 0 {
        println!("✅ all files are being read.")
    }
    finish();
    exit(rc);
}

fn check_read_files(reading_files: Vec<PathBuf>) -> i32 {
    // mark any files being read with the number 0
    let mut map = TO_DELETE.lock().unwrap();
    for file in reading_files {
        if map.contains_key(&file) {
            map.insert(file, 0);
        }
    }

    // check which files are not being read
    let mut count = 0;
    for (k, v) in map.iter() {
        if *v != 0 {
            println!("❌ not reading {}", k.to_str().unwrap());
            count += 1;
        }
    }
    return count;
}

// recursively find all the descendants of a pid and add to a vector
fn collect_descendants(pid: u32, descendants: &mut Vec<u32>, procs: &HashMap<u32, Vec<u32>>) {
    descendants.push(pid);
    if let Some(cpids) = procs.get(&pid) {
        for cpid in cpids {
            let p: u32 = *cpid;
            collect_descendants(p, descendants, procs)
        }
    }
}
