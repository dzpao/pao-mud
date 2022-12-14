int exit(string args) {
    write("再见！\n");
    destruct(this_object());
    return 1;
}

int hello(string args) {
    tell_object(this_object(), "HELLO\nWORLD\n");
    return 1;
}

int do_command(string cmd) {
    object cobj;

    debug_message("CMD: " + cmd + "\n");

    cmd = "/command/" + cmd;
    cobj = find_object(cmd);
    if (cobj) {
        destruct(cobj);
    }

    cobj = load_object(cmd);
    if (cobj) {
        write("命令 " + cmd + " 编译成功。\n");
	    return (int)cobj->main(this_object());
    } else {
        return 0;
    }
}

int test(string args) {
    return do_command("test");
}

int FB;

int fb(string args) {
    int fd, ret;

    write( "即将进入副本……\n" );
    fd = socket_create(3, "on_backend_output", "on_backend_close");
    ret = socket_connect(fd, "127.0.0.1 8000", "on_backend_output", "on_backend_close");
    if (ret < 0) {
        write( "进入副本失败……\n" );
        write("ret = " + (string)ret + "\n");
        return 1;
    }

    ret = socket_release(fd, this_object(), "release_callback");
    debug_message("release ret = " + (string)ret + "\n");

    write("已经进入副本。\n");

    add_action("all_cmds", "", 2);

    write("> ");
    telnet_ga();
    prompt_off();

    FB = fd;

    return 1;
}

string GA;

int set_binary() {
    buffer buf1, buf2, buf, ga;

    buf1 = allocate_buffer(4);
    buf2 = allocate_buffer(4);
    buf = allocate_buffer(6);
    write_buffer(buf1, 0, 0xfffd0000);
    write_buffer(buf2, 0, 0xfffb0000);
    write_buffer(buf, 0, read_buffer(buf1, 0, 3));
    write_buffer(buf, 3, read_buffer(buf2, 0, 3));

    socket_write(FB, buf);

    ga = allocate_buffer(4);
    write_buffer(ga, 0, 0xfff90000);
    GA = read_buffer(ga, 0, 2);
}

int binary;

void on_backend_output(int fd, mixed message) {
    string msg;
    string ga;

    if (binary != 1) {
        set_binary();
        binary = 1;
    }

    ga = read_buffer(message, -2, 0);
    if (ga == GA) {
        msg = read_buffer(message, 0, sizeof(message)-2);
        tell_object(this_object(), "副本内容:\n" + msg);
        telnet_ga();
    }
    else {
        msg = read_buffer(message, 0, 0);
        tell_object(this_object(), "副本内容:\n" + msg);
    }
}

void on_backend_send(int fd)    {}

void on_backend_close(int fd) {
    tell_object(this_object(), "已经离开副本。\n");
    remove_action("all_cmds", "");
    prompt_on();
    binary = 0;
}

void release_callback(int fd, object ob) {
    int ret;
    ret = socket_acquire(fd, "on_backend_output", "on_backend_send", "on_backend_close");
    debug_message("acquire ret = " + (string)ret + "\n");
}

int all_cmds(string cmd) {
    int ret;

    if (cmd == "fb") {
        return 0;
    }

    ret = socket_write(FB, cmd + "\r\n");
    if (ret > 0) {
        write("命令已转发至副本: \e[1;32m" + cmd + "\e[0m\n");
    }
    else {
        write("命令转发出错: \e[1;31m" + cmd + "\e[0m\n");
    }

    if (cmd == "exit") {
        write("即将离开副本。\n");
        prompt_on();
    }

    return 1;
}

void logon() {
    add_action("exit", "exit");
    add_action("exit", "quit");
    add_action("hello", "hello");
    add_action("test", "test");
    add_action("fb", "fb");
    write("欢迎！\n");
    write("> ");
}

void init() {
}
