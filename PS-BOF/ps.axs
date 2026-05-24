var metadata = {
    name: "PS-BOF",
    description: "Process management: ps list, ps kill, ps run, ps grep, ps suspend, ps resume",
};

var cmd_ps_list = ax.create_command("list", "List all running processes", "ps list");
cmd_ps_list.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let bof_path = ax.script_dir() + "_bin/list." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}"`, "BOF: ps list");
});

var cmd_ps_kill = ax.create_command("kill", "Terminate a process by PID", "ps kill 1234");
cmd_ps_kill.addArgInt("pid", true);
cmd_ps_kill.addArgInt("exit_code", false);
cmd_ps_kill.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let pid      = parsed_json["pid"];
    let bof_path = ax.script_dir() + "_bin/kill." + ax.arch(id) + ".o";
    let bof_params;
    if (parsed_json["exit_code"] !== undefined && parsed_json["exit_code"] !== null) {
        bof_params = ax.bof_pack("int,int", [pid, parsed_json["exit_code"]]);
    } else {
        bof_params = ax.bof_pack("int,int", [pid, 1]);
    }
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: ps kill");
});

var cmd_ps_run = ax.create_command("run", "Create a new process", "ps run --command \"cmd.exe /c whoami\" --pipe");
cmd_ps_run.addArgFlagString("--command", "command", true, "Command line to execute");
cmd_ps_run.addArgFlagString("--state", "state", false, "Process state: suspended or standard");
cmd_ps_run.addArgBool("--pipe", "Capture stdout/stderr via anonymous pipe", false);
cmd_ps_run.addArgFlagInt("--ppid", "ppid", "Parent PID for PPID spoofing", 0);
cmd_ps_run.addArgFlagString("--domain", "domain", false, "Domain (CreateProcessWithLogon)");
cmd_ps_run.addArgFlagString("--username", "username", false, "Username (CreateProcessWithLogon)");
cmd_ps_run.addArgFlagString("--password", "password", false, "Password (CreateProcessWithLogon)");
cmd_ps_run.addArgFlagInt("--token", "token", "Token handle (CreateProcessWithToken)", 0);
cmd_ps_run.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let cmd    = parsed_json["command"] || "";
    let state  = parsed_json["state"] === "suspended" ? 1 : 0;
    // addArgBool stores its key with the full flag name including leading dashes,
    // unlike addArgFlagString which strips the dashes and uses the separate key arg.
    let pipe   = parsed_json["--pipe"] ? 1 : 0;
    let ppid   = parsed_json["ppid"] || 0;
    let domain = parsed_json["domain"] || "";
    let user   = parsed_json["username"] || "";
    let pass   = parsed_json["password"] || "";
    let token  = parsed_json["token"] || 0;

    let method = 0;
    if (token && token !== 0) { method = 2; }
    else if (domain || user || pass) { method = 1; }

    let bof_params = ax.bof_pack("int,wstr,int,int,int,wstr,wstr,wstr,int",
                                  [method, cmd, state, pipe, ppid, domain, user, pass, token]);
    let bof_path = ax.script_dir() + "_bin/run." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: ps run");
});

var cmd_ps_grep = ax.create_command("grep", "Inspect a process by PID", "ps grep 1234");
cmd_ps_grep.addArgInt("pid", true);
cmd_ps_grep.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let pid        = parsed_json["pid"];
    let bof_path   = ax.script_dir() + "_bin/grep." + ax.arch(id) + ".o";
    let bof_params = ax.bof_pack("int", [pid]);
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: ps grep");
});

var cmd_ps_suspend = ax.create_command("suspend", "Suspend a process by PID", "ps suspend 1234");
cmd_ps_suspend.addArgInt("pid", true);
cmd_ps_suspend.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let pid        = parsed_json["pid"];
    let bof_path   = ax.script_dir() + "_bin/suspend." + ax.arch(id) + ".o";
    let bof_params = ax.bof_pack("int", [pid]);
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: ps suspend");
});

var cmd_ps_resume = ax.create_command("resume", "Resume a suspended process by PID", "ps resume 1234");
cmd_ps_resume.addArgInt("pid", true);
cmd_ps_resume.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let pid        = parsed_json["pid"];
    let bof_path   = ax.script_dir() + "_bin/resume." + ax.arch(id) + ".o";
    let bof_params = ax.bof_pack("int", [pid]);
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: ps resume");
});

var cmd_ps = ax.create_command("ps", "Process management");
cmd_ps.addSubCommands([cmd_ps_list, cmd_ps_kill, cmd_ps_run, cmd_ps_grep, cmd_ps_suspend, cmd_ps_resume]);

var group_ps = ax.create_commands_group("PS-BOF", [cmd_ps]);
ax.register_commands_group(group_ps, ["beacon", "gopher", "kharon"], ["windows"], []);
