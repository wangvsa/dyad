#ifndef DYAD_TEST_SHUFFLE_CLI_ARGS_H
#define DYAD_TEST_SHUFFLE_CLI_ARGS_H
#include <getopt.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

struct ProgramOptions {
    // Input mode
    bool use_count = false;
    long count = 0;
    std::string list_file;

    // Directory options
    std::string managed_dir;
    bool is_local = false;
    bool is_local_set = false;

    // File handling
    bool generate = false;
    std::string shared_dir;
    size_t size = 4096;

    // Run options
    unsigned n_epochs = 1u;
    unsigned seed = static_cast<unsigned> (time (nullptr));
    bool seed_set = false;
};

void print_usage (const char* prog, std::ostream& os)
{
    os << "usage: " << prog << " [options]\n"
       << "  --count,      -c <N>      number of sample files to auto-generate\n"
       << "  --list,       -l <file>   file containing list of filenames, one per line\n"
       << "  --dir,        -d <path>   DYAD managed directory\n"
       << "  --is-local,   -i <0|1>    1 if managed_dir is local, 0 if shared\n"
       << "  --generate,   -g          generate files (default: off)\n"
       << "  --shared-dir, -S <path>   shared storage path to stage files from\n"
       << "  --epochs,     -e <n>      number of epochs (default: 1)\n"
       << "  --seed,       -s <n>      random seed (default: random)\n"
       << "  --help,       -h          print this message\n";
}

bool parse_size(const char* str, size_t& out)
{
    char* end;
    long long val = strtoll(str, &end, 10);
    if (val <= 0 || end == str)
        return false;

    switch (*end) {
        case 'K': case 'k': out = static_cast<size_t>(val) * 1024ULL;                    break;
        case 'M': case 'm': out = static_cast<size_t>(val) * 1024ULL * 1024ULL;          break;
        case 'G': case 'g': out = static_cast<size_t>(val) * 1024ULL * 1024ULL * 1024ULL; break;
        case '\0':          out = static_cast<size_t>(val);                               break;
        default: return false;  // unrecognized suffix
    }
    return true;
}

int parse_args (int argc, char** argv, ProgramOptions& opts)
{
    static struct option long_options[] = {{"count", required_argument, nullptr, 'c'},
                                           {"list", required_argument, nullptr, 'l'},
                                           {"dir", required_argument, nullptr, 'd'},
                                           {"is-local", required_argument, nullptr, 'i'},
                                           {"generate", no_argument, nullptr, 'g'},
                                           {"shared-dir", required_argument, nullptr, 'S'},
                                           { "size", required_argument, nullptr, 'z' },
                                           {"epochs", required_argument, nullptr, 'e'},
                                           {"seed", required_argument, nullptr, 's'},
                                           {"help", no_argument, nullptr, 'h'},
                                           {nullptr, 0, nullptr, 0}};

    int opt;
    while ((opt = getopt_long (argc, argv, "c:l:d:i:gS:e:s:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'c':
                opts.use_count = true;
                opts.count = strtol (optarg, nullptr, 10);
                break;
            case 'l':
                opts.list_file = optarg;
                break;
            case 'd':
                opts.managed_dir = optarg;
                break;
            case 'i':
                opts.is_local = strtol (optarg, nullptr, 10) != 0;
                opts.is_local_set = true;
                break;
            case 'g':
                opts.generate = true;
                break;
            case 'S':
                opts.shared_dir = optarg;
                break;
            case 'z':
                if (!parse_size(optarg, opts.size)) {
                    std::cerr << "error: invalid --size value '" << optarg
                              << "'; expected a positive integer with optional suffix K, M, or G\n";
                    return EXIT_FAILURE;
                }
                break;
            case 'e':
                opts.n_epochs = static_cast<unsigned> (strtol (optarg, nullptr, 10));
                break;
            case 's':
                opts.seed = static_cast<unsigned> (strtoul (optarg, nullptr, 10));
                opts.seed_set = true;
                break;
            case 'h':
                print_usage (argv[0], std::cout);
                return EXIT_SUCCESS;
            default:
                print_usage (argv[0], std::cerr);
                return EXIT_FAILURE;
        }
    }

    // --- Validate ---

    // Exactly one of --count or --list required
    if (opts.use_count && !opts.list_file.empty ()) {
        std::cerr << "error: --count and --list are mutually exclusive\n";
        return EXIT_FAILURE;
    }
    if (!opts.use_count && opts.list_file.empty ()) {
        std::cerr << "error: one of --count or --list is required\n";
        return EXIT_FAILURE;
    }

    // --dir and --is-local always required
    if (opts.managed_dir.empty ()) {
        std::cerr << "error: --dir is required\n";
        return EXIT_FAILURE;
    }
    if (!opts.is_local_set) {
        std::cerr << "error: --is_local is required\n";
        return EXIT_FAILURE;
    }

    // --count rules
    if (opts.use_count) {
        if (opts.count <= 0) {
            std::cerr << "error: --count must be a positive integer\n";
            return EXIT_FAILURE;
        }

        if (!opts.is_local && !opts.shared_dir.empty ()) {
            std::cerr << "error: --shared-dir is not applicable when generating files into shared "
                         "managed_dir\n";
            return EXIT_FAILURE;
        }

        // --generate is implicit; explicit is not an error
        opts.generate = true;
    }

    if (opts.generate && !opts.is_local && !opts.shared_dir.empty ()) {
        std::cerr << "error:  --shared-dir is not useful when generating into shared managed_dir\n";
        return EXIT_FAILURE;
    }

    // --list rules
    if (!opts.list_file.empty ()) {
        if (opts.is_local) {
            // With is_local=true: exactly one of --generate or --shared-dir required
            if (!opts.generate && opts.shared_dir.empty ()) {
                std::cerr << "error: --list with --is-local=1 requires one of --generate or "
                             "--shared-dir\n";
                return EXIT_FAILURE;
            }
        } else {
            // With is_local=false: --shared-dir is not applicable
            if (!opts.shared_dir.empty ()) {
                std::cerr << "error: --shared-dir is not applicable when files exist on shared "
                             "managed_dir\n";
                return EXIT_FAILURE;
            }
            // --generate is optional; default is files already exist
        }
    }

    // --size is only valid when generating
    if (opts.size != 4096 && !opts.generate) {
        std::cerr << "error: --size is only applicable when files are being generated\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
#endif
