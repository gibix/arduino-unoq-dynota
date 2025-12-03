#include "llext_loader.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/llext/buf_loader.h>
#include <zephyr/llext/llext.h>
#include <zephyr/sys/util.h>

// Convert hex string to binary (used to decode ELF data from Bridge)
size_t hex2bin(const char *hex, size_t hexlen, uint8_t *buf, size_t buflen) {
  uint8_t dec;

  if (buflen < (hexlen / 2U + hexlen % 2U)) {
    return 0;
  }

  if ((hexlen % 2U) != 0) {
    if (char2hex(hex[0], &dec) < 0) {
      return 0;
    }
    buf[0] = dec;
    hex++;
    buf++;
  }

  for (size_t i = 0; i < (hexlen / 2U); i++) {
    if (char2hex(hex[2U * i], &dec) < 0) {
      return 0;
    }
    buf[i] = dec << 4;

    if (char2hex(hex[2U * i + 1U], &dec) < 0) {
      return 0;
    }
    buf[i] += dec;
  }

  return hexlen / 2U + hexlen % 2U;
}

static bool llext_xfer_in_progress = false;
static String current_filename = "";

// Buffer for assembling ELF binary from hex chunks received via Bridge
#define LLEXT_BUF_SIZE (60 * 1024)
static uint8_t llext_buf[LLEXT_BUF_SIZE];
static size_t llext_buf_len = 0;

static int llext_load_counter = 0;

// Receive and decode a hex-encoded chunk of ELF data
void load_elf_chunk(String chunk) {
  const char *chunk_str = chunk.c_str();
  size_t chunk_len = chunk.length();
  
  size_t decoded = hex2bin(chunk_str, chunk_len,
                           llext_buf + llext_buf_len,
                           LLEXT_BUF_SIZE - llext_buf_len);
  llext_buf_len += decoded;
}

// Initialize buffer for receiving new ELF file
void start_llext_xfer(String filename) {
  llext_xfer_in_progress = true;
  llext_buf_len = 0;
  memset(llext_buf, 0, LLEXT_BUF_SIZE);
  current_filename = filename;
}

// Thread that loads and runs the uploaded ELF binary using llext
void llext_loader_thread(void *p1, void *p2, void *p3) {
  struct llext_buf_loader buf_loader =
      LLEXT_BUF_LOADER(llext_buf, llext_buf_len);

  
  struct llext_loader *ldr = &buf_loader.loader;

  char ext_name[32];
  snprintf(ext_name, sizeof(ext_name), "ext_%d", llext_load_counter++);

  struct llext_load_param ldr_parm = LLEXT_LOAD_PARAM_DEFAULT;
  struct llext *ext;
  
  int res = llext_load(ldr, ext_name, &ext, &ldr_parm);

  if (res != 0) {
    return;
  }

  // Execute the loaded extension's main() function
  llext_call_fn(ext, "main");

  (void)res;
}

// Finalize transfer and spawn thread to dynamically load the ELF
String stop_llext_xfer() {
  llext_xfer_in_progress = false;

  // Dynamically allocate thread stack (needed for llext loading)
  k_thread_stack_t *loader_stack = k_thread_stack_alloc(30 * 1024, 0);
  if (loader_stack == NULL) {
    return "Failed to allocate loader stack";
  }

  k_thread *loader_thread = (k_thread *)k_malloc(sizeof(k_thread));
  if (loader_thread == NULL) {
    k_thread_stack_free(loader_stack);
    return "Failed to allocate loader thread";
  }


  k_tid_t loader_tid =
      k_thread_create(loader_thread, loader_stack, 30 * 1024,
                      llext_loader_thread, NULL, NULL, NULL, 10, 0, K_NO_WAIT);

  // Name the thread after the uploaded file for easy identification
  char thread_name[32];
  snprintf(thread_name, sizeof(thread_name), "%s",
           current_filename.c_str());
  k_thread_name_set(loader_tid, thread_name);
  
  return "OK";
}
