#include <server/SealServerRunner.h>

#include <signal.h>
#include <cstdio>

SealServerRunner runner;

void sig_handler(int t)
{
    runner.sig_handler(t);
    exit(1);
}

int main(int argc, const char** argv)
{
    signal(SIGINT, sig_handler);
    runner.run("localhost:4567");
    return 0;
}