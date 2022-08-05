#include "options.h"


#include <algorithm>
#include <iterator>
#include <set>
#include <unordered_map>

namespace apie {
namespace common {


namespace internal {

void CheckExpectedOptionsImpl(std::set<std::type_index> const& expected,
                              Options const& opts, char const* const caller) {
  for (auto const& p : opts.m_) {
    if (!Contains(expected, p.first)) {
      GCP_LOG(WARNING) << caller << ": Unexpected option (mangled name): "
                       << p.first.name();
    }
  }
}

Options MergeOptions(Options preferred, Options alternatives) {
  if (preferred.m_.empty()) return alternatives;
  preferred.m_.insert(std::make_move_iterator(alternatives.m_.begin()),
                      std::make_move_iterator(alternatives.m_.end()));
  return preferred;
}

namespace {

// The prevailing options for the current operation.  Thread local, so
// additional propagation must be done whenever work for the operation
// is done in another thread.
Options& ThreadLocalOptions() {
  thread_local Options current_options;
  return current_options;
}

}  // namespace

Options const& CurrentOptions() { return ThreadLocalOptions(); }

OptionsSpan::OptionsSpan(Options opts) : opts_(std::move(opts)) {
  using std::swap;
  swap(opts_, ThreadLocalOptions());
}

OptionsSpan::~OptionsSpan() { ThreadLocalOptions() = std::move(opts_); }

}  // namespace internal

}  // namespace cloud
}  // namespace google
