#include "exceptions.h"

namespace data_flow {
namespace {
struct Context {
  std::ostringstream oss;
  int32_t stack_idx = 0;
};

const char* signal_name(int sig) {
  switch (sig) {
    case SIGSEGV:
      return "SIGSEGV (Segmentation fault)";
    case SIGABRT:
      return "SIGABRT (Abort)";
    case SIGFPE:
      return "SIGFPE (Floating point exception)";
    case SIGILL:
      return "SIGILL (Illegal instruction)";
    case SIGBUS:
      return "SIGBUS (Bus error)";
    default:
      return "Unknown signal";
  }
}
}  // namespace

void error_callback(void* data, const char* msg, int errnum) {
  auto ctx = reinterpret_cast<Context*>(data);
  ctx->oss << "backtrace error: " << msg << " (" << errnum << ")" << std::endl;
};

int print_stack(void* data, uintptr_t pc, const char* filename, int lineno, const char* function) {
  auto ctx = reinterpret_cast<Context*>(data);
  std::string func_name = function ? demangle_str_name(function) : "??";
  ctx->oss << "#" << ctx->stack_idx++ << "  " << func_name;
  if (filename) {
    ctx->oss << " at " << filename << ":" << lineno;
  }
  ctx->oss << " [0x" << std::hex << pc << std::dec << "]\n";
  return 0;  // continue
};

void print_stack_trace(int sig) {
  static backtrace_state* bt_state = nullptr;

  Context ctx;
  int32_t stack_idx = 0;

  if (!bt_state) {
    bt_state = backtrace_create_state(nullptr, 0, error_callback, &ctx);
  }
  ctx.oss << std::endl;
  ctx.oss << "========================== Crash Detected ===========================\n";
  ctx.oss << "signal: " << sig << " (" << signal_name(sig) << ")\n";
  ctx.oss << "stack trace:\n";

  backtrace_full(bt_state, 0, print_stack, error_callback, &ctx);

  ctx.oss << "=====================================================================\n";
  std::cerr << ctx.oss.str();
}

void signal_handler(int sig) {
  print_stack_trace(sig);
  signal(sig, SIG_DFL);  // 恢复默认处理
  raise(sig);            // 再次触发信号，让系统生成 core dump
}

void setup_signal_handler() {
  signal(SIGSEGV, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGFPE, signal_handler);
  signal(SIGILL, signal_handler);
  signal(SIGBUS, signal_handler);
}

}  // namespace data_flow