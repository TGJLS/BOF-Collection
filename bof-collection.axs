var metadata = {
    name: "BOF-Collection",
    description: "Filesystem, process-control, token, post-exploitation, and exit BOFs",
    nosave: true
};

var path = ax.script_dir();
ax.script_load(path + "FS-BOF/fs.axs");
ax.script_load(path + "Exit-BOF/exit.axs");
ax.script_load(path + "PS-BOF/ps.axs");
ax.script_load(path + "TK-BOF/tk.axs");
ax.script_load(path + "Postex-BOF/postex.axs");
