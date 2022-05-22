Hazelnet library: CAN FD bus encryption, authentication, and freshness
======================================================================

Hazelnet implements the Client and Server roles of the
[CAN Bus Security (CBS) protocol](https://matjaz.it/cbs/), which secures the
**CAN FD** traffic providing message **encryption, authenticity, and freshness**.
CBS focuses on lightweight encryption using **symmetric** primitives only,
**multicast** communication, and an assurance of **freshness** of the messages
to prevent replay attacks.

The user of the library must handle the physical transmission and reception
manually as this library only handles the building of messages to transmit and
processing of received messages. This is done to guarantee better portability
across systems. The internal library state keeps track of ongoing handshakes,
timeouts and other events per each Group.

The library uses standard C11 code and is hardware-independent. The compile
targets for a desktop OS add some features like heap memory allocation and use
the time and TRNG functionality the OS provides. The any-platform version
uses user-provided structs to operate on and requires the user to provide
function pointers to custom timestamping and random-number-generators of the
used platform; recommended for embedded systems.


Dependencies
---------------------------------------

Hazelnet depends on other projects from the same author:

- required: [LibAscon](https://github.com/TheMatjaz/LibAscon) crypto library
  (CC0 license).
- for testing only: [Atto](https://github.com/TheMatjaz/atto) minimal unit test
  framework (BSD 3-clause license)
- for config generation only:
  [HzlConfig](https://github.com/TheMatjaz/HazelnetConfig) to write the
  configuration of the entire bus in one JSON file and generate the binary
  configurations of each Hazelnet instance (CBS Party).

All dependencies are **included in this project as Git Submodules**, so be sure
to clone this repository with the `--recurse-submodules` option. If you
have already cloned it normally, then run
`git submodule update --init --recursive`.

You only need the C99/C11 standard library to compile the library for any
platform, including embedded. No heap-allocation required (malloc).

- `stdint.h`
- `stdbool.h`
- `stddef.h`
- `string.h`

On the other hand, when compiling for a desktop OS, additional features are
enabled. In particular configurations can be loaded from files, the library
context is heap-allocated, and the OS provides the current time and true
randomness.

- `stdlib.h` for heap-allocation (calloc)
- `stdio.h` to access configuration files
- On Windows also
  - `sysinfoapi.h` to get the current time in milliseconds
  - `bcrypt.h` to get random numbers
- On Unix-like systems also
  - `sys/time.h` to get the current time in milliseconds


Known limitations
---------------------------------------

The library is **not thread safe**. All API calls on the **same** context
should be performed within the same thread (or RTOS task) or mutual exclusion
locks should be placed by the library user around said API calls to protect the
library from race conditions. For the time being, no thread-safe protections
have been implemented as they are not easily portable.


Project structure
---------------------------------------

- The `inc` folder contains the public headers of the library.
  - `hzl.h` is a common header for both the Client and Server. You always
    need this.
  - `hzl_Client.h` and `hzl_Server.h` are the API header of the
    Client and Server libraries (respectively) when compiled for any platform.
    Pick one of the two roles.
  - `hzl_ClientOs.h` and `hzl_ServerOs.h` are extensions of the API
    for the Client and Server libraries (respectively) when compiled for
    desktop operating systems (assuming a file system and heap-memory
    allocation).
- The `src` folder contains the library sources:
  - `src/common` is code shared between Client and Server
  - `src/client` and `src/server` folder contain sources for the respective
    Parties
  - The `.c` files are generally named after the user-facing API function they
    implement.


Example usage of the library API
---------------------------------------

> See it in practice in the
> [Hazelnet Demo Platform](https://github.com/TheMatjaz/HazelnetDemoPlatform)
> showcasing a few microcontrollers exchanging dummy encrypted messages.
> In particular the [`Sources/hzlPlatform_TaskHzl.c`](https://github.com/TheMatjaz/HazelnetDemoPlatform/blob/v1.1.0/Sources/hzlPlatform_TaskHzl.c)
> file.

### Client library API

To use the client library, the following headers are required:

```c
#include "hzl.h"
#include "hzl_Client.h"
#include "hzl_ClientOs.h" /* Optional, for a desktop OS only. */
```

#### On a desktop OS

```c
// Load the configuration from a binary file, use the OS for the current time
// and random number generation. A Python package is available in the
// `external/hazelnetconfig` project to generate the config files from a JSON
// file.
hzl_Err_t err;
hzl_ClientCtx* pCtx;
err = hzl_ClientNew(&pCtx, "myconfigfile.hzl");
if (err != HZL_OK) { custom_error_handling(err); }
hzl_CbsPduMsg_t* pPdu; // Packed data, fits into one CAN FD message

// Let's allocate the memory for a message we want to send
err = hzl_ClientNewMsg(&pPdu);
if (err != HZL_OK) { custom_error_handling(err); }

// To start secured communication within a Group, we need to start a handshake
hzl_Gid_t destinationGroupId = 2; // The set of nodes we want to talk to
err = hzl_ClientBuildRequest(pPdu, ctx, destinationGroupId);
if (err != HZL_OK) { custom_error_handling(err); }
// The Request message is built, the user must transmit it manually
myCustomTransmission(pPdu->data, pPdu->dataLen);
// The library takes care of storing the building timestamp, the current status
// and timeout checks. The user must only take care of transmission and
// reception.

// After some time, the Server will transmit a Response. To process received
// messages (often in a separate thread/task), do as follows:
hzl_RxSduMsg_t userData;
err = hzl_ClientProcessReceived(
    pPdu,                  // automatic internal reaction message, if required
    &userData,             // decrypted user-data, if the message had any
    pCtx,
    receivedPackedData,    // the CAN FD payload as received from the layer below
    receivedPackedDataLen, // the CAN FD payload length in bytes (CAN DLC)
    receivedCanId          // the CAN ID of the message had
);
// The error codes are many, including the cases when the message is simply
// ignored or when it contains critical security errors. Be sure to check them!
if (err != HZL_OK) { custom_error_handling(err); }
// Transmit an automatic reaction message
if (pPdu->dataLen > 0) { myCustomTransmission(pPdu->data, pPdu->dataLen); }

// If the Response from the Server was received, we can now build a secured
// data message for the Group that had the handshake completed.
uint8_t secretMessage[] = "hello world";
err = hzl_ClientBuildSecuredFd(pPdu,
                               pCtx,
                               secretMessage,
                               sizeof(secretMessage),
                               destinationGroupId);
if (err != HZL_OK) { custom_error_handling(err); }
myCustomTransmission(pPdu->data, pPdu->dataLen);

// At any point in time, unsecured messages may be sent, even for Groups
// that don't have a handshake completed or are not in the configuration.
// USE IT ONLY FOR ALREADY SECURED or NON SENSITIVE DATA.
uint8_t publicMessage[] = "just some debug info";
err = hzl_ClientBuildSecuredFd(pPdu,
                               pCtx,
                               publicMessage,
                               sizeof(publicMessage),
                               0);  // Group 0 is a broadcast
if (err != HZL_OK) { custom_error_handling(err); }
myCustomTransmission(pPdu->data, pPdu->dataLen);

// Free the heap memory when done
hzl_ClientFree(&pCtx);
hzl_ClientFreeMsg(&pPdu);
```

#### On an embedded system

The usage is the same except compared to the desktop OS case, except for the
Context init and deinit as it cannot necessarily happen on the heap or be
loaded from a file. The user must prepare it manually:

```c
// Prepare the Context manually.
hzl_Err_t err;
hazelnetGroupStates[10]; // Assuming 10 groups. No need to initialise!
hzl_ClientCtx ctx = {
    // Add the pointers to the constant configuration structs (which are NEVER
    // written to by the library). E.g. a pointer to the prepared data in the
    // persistent flash memory. STRUCTS MUST INCLUDE ANY PADDINGS!
    .clientConfig = MY_ADDR_OF_THE_HAZELNET_CONFIG_OF_THIS_CLIENT,
    .groupConfigs = MY_ADDR_OF_THE_HAZELNET_GROUP_CONFIGS_OF_THIS_CLIENT,
    // and the memory used for the group states, which is initialised
    // and manager by the library.
    .groupStates = hazelnetGroupStates,
    // Finally the timestamping and random number generation functions
    // provided as function pointers.
    .io = {
        .trng = &myPlatformTrueRandomNumberGeneratorWrappedForHazelnet,
        .currentTime = &myPlatformCurrentTimeFuncWrappedForHazelnet
    }
};
// The init function will run many checks verifying if the configuration is OK.
err = hzl_ClientInit(&ctx); // Instead of ClientNew()
if (err != HZL_OK) { custom_error_handling(err); }

// -----------
// Use the other library functions as in the Desktop OS example above.
// -----------

// When done or before entering sleep-mode/low-power-mode, deinit the context
// to erase security-critical information.
err = hzl_ClientDeInit(&ctx); // Instead of ClientFree()
if (err != HZL_OK) { custom_error_handling(err); }
```

### Server library API

The Server library has an analogous API to the Client, with the same interface
when it comes to processing of the messages. The main difference is that the
Server requires an additional configuration field in its context, namely
the array of per-Client configurations.

To use the Server library, the following headers are required:

```c
#include "hzl.h"
#include "hzl_Server.h"
#include "hzl_ServerOs.h" /* Optional, for a desktop OS only. */
```

Including the library in your project
---------------------------------------

There are multiple valid ways of doing so. The project is CMake-enabled,
so this should be the easiest way. For embedded systems where CMake is not
an option, the project may be also compiled using any custom build system.

### Cloning with submodules

The dependencies are included in this project as Git Submodules, so be sure
to clone this repository with the `--recurse-submodules` option.

```
git clone --recurse-submodules https://github.com/TheMatjaz/Hazelnet
```

If you have already cloned the repo, but not the submodules, then run

```
git submodule init
git submodule update
```

### Compiling with CMake

In the project root folder, run the following to create an out-of-source build:

```
mkdir build                   # `build*/` folders are already ignored
cd build
cmake ..                      # Prepare the makefiles, default to MinSizeRel
cmake --build . --parallel    # Equivalent to `make all`
```

By default, an optimised-for-size release build is performed.
To change it, append the `-DCMAKE_BUILD_TYPE=Release` or `Debug`
to the `cmake ..` command and build again.

#### Compiling with CMake with MSVC

You may already know this, which makes this section mostly a note for my future
self. When compiling from the Windows command line using the MSVC toolchain:

- You will need
  [the environment variables](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-160#developer_command_file_locations)
  to use the MSVC toolchain from the command line
  - Hint: search for "x64 Native tools" in the start menu, the first option
  - Alternative: open the VS tools installer and click on "Launch"
- Be sure to select the generator NMake Makefiles when configuring CMake
- Use the CMake that comes with MSVC. Mine was installed in
  `"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"`
  but yours may be different, especially for different VS versions.
  This is required if you already installed CMake through MSYS.

```
mkdir build
cd build
"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" .. -G "NMake Makefiles"
"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build .
```

#### CMake targets explanation

- `hzl_client_any`, `hzl_server_any`: Client- and Server-side static libraries
  for any platform, including embedded. No assumptions about the system, no
  heap memory, no files.
- `hzl_client_desktop`, `hzl_server_desktop`: Client- and Server-side static
  libraries for desktop operating systems, assuming malloc, a file system and
  using the OS-provided TRNG.
- `hzl_client_desktop_shared`, `hzl_server_desktop_shared`: like
  `hzl_client_desktop` and `hzl_server_desktop` but shared (dynamic)
  libraries.

All other targets are internal dependencies or test targets: the user should
not worry about them.

### Compiling the library from sources using a custom build system

1. Include the following directories in the search path for header files
   (`-Iinc` compiler option for GCC):

   ```text
   inc/
   src/common/
   src/client/ XOR src/server/ -- THEY ARE MUTUALLY EXCLUSIVE
   external/atto/src/
   external/libascon/inc
   external/libascon/src
   ```

   For the test suite, also add

   ```text
   tst/
   ```

2. Include the following directories in the search path for source files:

   ```text
   src/
   external/atto/src/
   external/libascon/src/
   ```

   For the test suite, also add

   ```text
   tst/
   tst/client/ XOR src/server/ XOR tst/interop/ -- THEY ARE MUTUALLY EXCLUSIVE
   ```

3. Compile with your favourite toolchain. For the test suite, the
   `tst/(client|server|interop)/hzl(Client|Server|Interop)Test_Main.c` file
   runs all tests, depending which set of tests you are compiling.


Testing the library
---------------------------------------

### With CMake (recommended)

You can run the test suite with `ctest`:

```
ctest --output-on-failure --parallel
```

or by directly executing the test runner executables:

```
test_hzl_client_desktop.exe
test_hzl_client_desktop_shared.exe
test_hzl_server_desktop.exe
test_hzl_server_desktop_shared.exe
test_hzl_interop_desktop.exe
test_hzl_interop_desktop_shared.exe
```

### On your embedded system

To run the unit tests in an embedded environment, rename the `main()` function
of the test runners
`tst/(client|server)/hzl(Client|Server)Test_Main.c`
into something else (e.g. `runAllHazelnetTests()`), integrate it into your
embedded firware and call it from your embedded project. The Atto framework
assumes you have `printf` available for the reports, but you can
search-and-replace its instances with another function, if you prefer.

The `interop` tests are meant for desktops only, because it does not make
a lot of sense for an embedded device to make an interoperability test
with itself.


Doxygen
---------------------------------------

Doxygen is build separately (not part of `make all`) to avoid running it
every time the library is recompiled during development:

```
cmake --build . --target hzl_doxygen
```


References
---------------------------------------

- [CAN Bus Security protocol specification](https://matjaz.it/cbs/),
  protocol version: **v1.3**, document revision: **4**
