#include <signal.h>
#include <ctime>

#include <iostream>
#include <boost/atomic/atomic.hpp>

#include <other/Log.h>
#include <other/Utils.h>
#include <crypto/SslSetup.h>

#include <Captain.h>

void init_signal_handle();
void usage();
void show_vcs_info();
int create_process_pid();

char cfgFile[PATH_MAX]{};
bool daemonize = false;

int main(int argc, char* argv[]) {

    strncpy(cfgFile, program_invocation_short_name, strlen(program_invocation_short_name));
    strcat(cfgFile, ".conf");
    int opt_g = 0;
    while ((opt_g = getopt(argc, argv, "c:dhv")) != -1) {
        switch (opt_g) {
            case 'c':
                memset(cfgFile, 0, sizeof(cfgFile));
                strncpy(cfgFile, optarg, PATH_MAX);
                break;
            case 'd':
                daemonize = true;
                break;
            case 'v':
                show_vcs_info();
                ::exit(EXIT_SUCCESS);
            case 'h':
            default:
                usage();
                ::exit(EXIT_SUCCESS);
        }
    }

    std::cout << "current system cfg file [" <<  cfgFile << "]."  << std::endl;
    // display build version info.
    show_vcs_info();

    // test boost::atomic
    boost::atomic<int> atomic_int;
    if (atomic_int.is_lock_free()) {
        roo::log_warning(">>> GOOD <<<, your system atomic is lock_free ...");
    } else {
        roo::log_err(">>> BAD <<<, your system atomic is not lock_free, which may impact performance ...");
    }


    // SSL 环境设置
    if (!roo::Ssl_thread_setup()) {
        roo::log_err("SSL env setup error!");
        ::exit(EXIT_FAILURE);
    }


    // daemonize should before any thread creation...
    if (daemonize) {
        roo::log_warning("daemonize this service...");

        bool chdir = false; // leave the current working directory in case
                            // the user has specified relative paths for
        // the config file, etc
        bool close = true;  // close stdin, stdout, stderr
        if (::daemon(!chdir, !close) != 0) {
            roo::log_err("Call daemon() failed with %d(%s).", errno, strerror(errno));
            ::exit(EXIT_FAILURE);
        }
    }

    (void)Captain::instance(); // create object first!

    create_process_pid();
    init_signal_handle();
    roo::backtrace_init();

    if (!Captain::instance().init(cfgFile)) {
        roo::log_err("Captain init failed!");
        ::exit(EXIT_FAILURE);
    }

    std::time_t now = boost::chrono::system_clock::to_time_t(boost::chrono::system_clock::now());
    char mbstr[32]{};
    std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&now));
    roo::log_warning("service started at %s.", mbstr);
    roo::log_warning("service initialized successfully!");

    Captain::instance().service_joinall();

    roo::Ssl_thread_clean();
    return 0;
}

