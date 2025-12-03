#ifndef LLEXT_LOADER_H
#define LLEXT_LOADER_H

#include <Arduino.h>

// Start the llext transfer process
void start_llext_xfer(String filename);

// Load a chunk of ELF data (in hex format)
void load_elf_chunk(String chunk);

// Stop the llext transfer and trigger loading
// Returns "OK" on success or an error message on failure
String stop_llext_xfer(void);

#endif // LLEXT_LOADER_H
