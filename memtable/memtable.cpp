#include "memtable.h"

void MemTable::insert(Key key, std::shared_ptr<Value> value) {
  std::lock_guard<std::mutex> guard(lock);
  table[key] = value;
}

std::shared_ptr<Value> MemTable::get(Key key) {
  std::lock_guard<std::mutex> guard(lock);

  if (table.find(key) == table.end()) {
    return nullptr;
  }

  return table[key];
}

void MemTable::remove(Key key) {
  std::lock_guard<std::mutex> guard(lock);

  auto v = std::make_shared<Value>();
  v->tombstone = true;

  table[key] = v;
}