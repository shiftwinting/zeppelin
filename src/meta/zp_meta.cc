#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <glog/logging.h>

#include "zp_meta_server.h"
#include "zp_options.h"

#include "env.h"

ZPMetaServer* zp_meta_server;

void Usage();
void ParseArgs(int argc, char* argv[], ZPOptions& options);

static void GlogInit(const ZPOptions& options) {
  if (!slash::FileExists(options.log_path)) {
    slash::CreatePath(options.log_path); 
  }

  FLAGS_alsologtostderr = true;

  FLAGS_log_dir = options.log_path;
  FLAGS_minloglevel = 0;
  FLAGS_max_log_size = 1800;
  ::google::InitGoogleLogging("zp");
}

static void IntSigHandle(const int sig) {
  LOG(INFO) << "Catch Signal " << sig << ", cleanup...";
  zp_meta_server->Stop();
}

static void SignalSetup() {
  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, &IntSigHandle);
  signal(SIGQUIT, &IntSigHandle);
  signal(SIGTERM, &IntSigHandle);
}

int main(int argc, char** argv) {
  ZPOptions options;

  ParseArgs(argc, argv, options);

  options.Dump();
  GlogInit(options);
  SignalSetup();

  zp_meta_server = new ZPMetaServer(options);

  zp_meta_server->Start();

  printf ("Exit\n");

  return 0;
}

void Usage() {
  printf ("Usage:\n"
          "  ./output/bin/zp-meta --seed_ip ip1 --seed_port port1 --local_ip local_ip --local_port local_port --data_path path --log_path path\n");
}

void ParseArgs(int argc, char* argv[], ZPOptions& options) {
  if (argc < 1) {
    Usage();
    exit(-1);
  }

  const struct option long_options[] = {
    {"seed_ip", required_argument, NULL, 'I'},
    {"seed_port", required_argument, NULL, 'P'},
    {"local_ip", required_argument, NULL, 'i'},
    {"local_port", required_argument, NULL, 'p'},
    {"data_path", required_argument, NULL, 'd'},
    {"log_path", required_argument, NULL, 'l'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}, };

  const char* short_options = "I:P:p:d:l:h";

  int ch, longindex;
  while ((ch = getopt_long(argc, argv, short_options, long_options,
                           &longindex)) >= 0) {
    switch (ch) {
      case 'I':
        options.seed_ip = optarg;
        break;
      case 'P':
        options.seed_port = atoi(optarg);
        break;
      case 'i':
        options.local_ip = optarg;
        break;
      case 'p':
        options.local_port = atoi(optarg);
        break;
      case 'd':
        options.data_path = optarg;
        break;
      case 'l':
        options.log_path = optarg;
        break;
      case 'h':
        Usage();
        exit(0);
      default:
        break;
    }
  }
}
