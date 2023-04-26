#include <algorithm>
#include <cstdio>
#include <map>
#include <set>
#include <string.h>
#include <string>
#include <vector>

static std::map<void *, long> current;
static std::map<void *, long> stored;
static std::map<void *, std::string> type_map;

struct State {
  long ref_cnt_diff;
  void *original_ptr;
  std::string type;
};
std::vector<State> flushed;
long total_ref = 0;

void breakpoint() {}

extern "C" {
extern void state_change(void *ptr, long rc, long diff, const char *type,
                         long current_total_ref);
extern void store_state();
extern void check_with_stored_state();

const char *get_type_name(void *ptr) { return type_map[ptr].c_str(); }

extern void state_change(void *ptr, long rc, long diff, const char *type,
                         long current_total_ref) {
  total_ref += diff;
  if (total_ref != current_total_ref) {
    fprintf(stderr, "unexpected total ref. here: %ld (%+ld) there %ld\n",
            total_ref, diff, current_total_ref);
    total_ref = current_total_ref;
    breakpoint();
  }

  if (rc < 0)
    return;

  if (diff && current[ptr] && current[ptr] + diff != rc) {
    fprintf(stderr,
            "unexpected ref count change of %p: %ld, (%s)%ld -> (%s)%ld,\n",
            ptr, diff, type_map[ptr].c_str(), current[ptr],
            type ? type : "__UNKNOWN__", rc);
    if (rc < 1000000) // not immortal
      breakpoint();
  }

  if (rc != 0) {
    current[ptr] = rc;
    // store type name early, so that we can print something when unexpected
    // things happen
    if (type)
      type_map[ptr] = type;
    return;
  }

  if (type)
    type_map[ptr] = type;
  else if (type_map[ptr].size() == 0)
    type_map[ptr] =
        "__UNKNOWN__"; // handles the case when PyObject is uninitialized

  if (stored[ptr])
    flushed.emplace_back(-stored[ptr], ptr, type_map[ptr]);
  current.erase(ptr);
  stored.erase(ptr);
  type_map.erase(ptr);
}

extern void store_state() {
  stored = current;
  flushed.clear();
}
extern void check_with_stored_state() {
  if (stored.size() == 0)
    return;

  std::set<void *> diff;
  std::map<std::string, long> typed_diff;

  for (auto i : current)
    if (stored[i.first] != i.second)
      diff.insert(i.first);

  for (auto i : stored)
    if (current[i.first] != i.second)
      diff.insert(i.first);

  long rc_change = 0;

  for (auto i : diff) {
    rc_change += current[i] - stored[i];
    typed_diff[type_map[i]] += current[i] - stored[i];
  }

  for (auto state : flushed) {
    rc_change += state.ref_cnt_diff;
    typed_diff[state.type] += state.ref_cnt_diff;
  }

  for (auto it = typed_diff.begin(); it != typed_diff.end();)
    if (it->second == 0)
      it = typed_diff.erase(it);
    else
      ++it;

  if (typed_diff.empty())
    return;

  for (auto i : diff)
    fprintf(stderr, "ref change of living %p (%s): %+ld,  %ld -> %ld\n", i,
            type_map[i].c_str(), current[i] - stored[i], stored[i], current[i]);

  for (auto state : flushed)
    fprintf(stderr, "ref change of died   %p (%s): %+ld\n", state.original_ptr,
            state.type.c_str(), state.ref_cnt_diff);

  for (auto i : typed_diff)
    fprintf(stderr, "ref change of type %s: %+ld\n", i.first.c_str(), i.second);

  fprintf(stderr, "total ref change %+ld\n", rc_change);
}
}