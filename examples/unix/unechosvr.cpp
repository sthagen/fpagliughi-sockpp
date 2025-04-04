// unechosvr.cpp
//
// A multi-threaded UNIX-domain echo server for sockpp library.
// This is a simple thread-per-connection UNIX server.
//
// USAGE:
//  	unechosvr [path]
//
// --------------------------------------------------------------------------
// This file is part of the "sockpp" C++ socket library.
//
// Copyright (c) 2014-2017 Frank Pagliughi
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// --------------------------------------------------------------------------

#include <iostream>
#include <thread>

#include "sockpp/unix_acceptor.h"
#include "sockpp/version.h"

using namespace std;

// --------------------------------------------------------------------------
// The thread function. This is run in a separate thread for each socket.
// Ownership of the socket object is transferred to the thread, so when this
// function exits, the socket is automatically closed.

void run_echo(sockpp::unix_socket sock) {
    char buf[512];
    sockpp::result<size_t> res;

    while ((res = sock.read(buf, sizeof(buf))) && res.value() > 0)
        sock.write_n(buf, res.value());

    cout << "Connection closed" << endl;
}

// --------------------------------------------------------------------------
// The main thread runs the UNIX acceptor.
// Each time a connection is made, a new thread is spawned to handle it,
// leaving this main thread to immediately wait for the next connection.

int main(int argc, char* argv[]) {
    cout << "Sample Unix-domain echo server for 'sockpp' " << sockpp::SOCKPP_VERSION << '\n'
         << endl;

#if defined(_WIN32)
    const string DFLT_PATH = "C:\\TEMP\\unechosvr.sock"s;
#else
    string DFLT_PATH = "/tmp/unechosvr.sock"s;
#endif

    const string path = (argc > 1) ? argv[1] : DFLT_PATH;

    sockpp::initialize();
    sockpp::unix_acceptor acc;

    auto res = acc.open(sockpp::unix_address(path));

    if (!res) {
        cerr << "Error creating the acceptor: " << res.error_message() << endl;
        return 1;
    }
    cout << "Acceptor bound to address: '" << acc.address() << "'..." << endl;

    while (true) {
        // Accept a new client connection
        if (auto res = acc.accept(); !res) {
            cerr << "Error accepting incoming connection: " << res.error_message() << endl;
        }
        else {
            cout << "Received a connection" << endl;
            // Create a thread and transfer the new stream to it.
            thread thr(run_echo, res.release());
            thr.detach();
        }
    }

    return 0;
}
