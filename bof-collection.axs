var metadata = {
    name: "BOF-Collection",
    description: "Filesystem and process-control BOFs: type, mkdir, copy, move, del, rmdir, pwd, cd, exit process/thread",
    nosave: true
};

var path = ax.script_dir();
ax.script_load(path + "FS-BOF/fs.axs");
ax.script_load(path + "Exit-BOF/exit.axs");
ax.script_load(path + "PS-BOF/ps.axs");
