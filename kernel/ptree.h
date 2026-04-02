struct ptreeinfo {
    int pid;        // Process ID
    int ppid;       // Parent process ID
    int state;      // Process state
    uint64 memsize; // User memory size (bytes)
    char name[16];  // Process name
};
