# WindowedMap — TTL Key-Value Store

## Problem Statement

Implement a `WindowedMap`, a key-value store where every entry expires after a fixed amount of time `EXPIRY_TIME`.

The store supports three APIs:

* `put(key, value)` → Insert or update a key with a value.
* `get(key)` → Return the value if the key exists and has not expired.
* `get_average()` → Return the average of all **non-expired** values.

### Requirements

1. Every `put()` creates/updates an entry with:

   ```text
   expireAt = currentTime + EXPIRY_TIME
   ```
2. Expired entries should behave as if they don't exist.
3. Updating an existing key should replace its old value and reset its expiration time.
4. `get_average()` should only consider non-expired entries.
5. Avoid scanning the entire map on every `get_average()`.

---

# Approach

Use:

1. **HashMap** — stores the current value of each key.
2. **Min Heap** — stores entries ordered by expiration time.
3. **Running Sum + Count** — allows calculating the average without scanning.
4. **Versioning** — handles updates to the same key safely.
5. **Lazy Deletion** — expired/stale heap entries are removed only when they reach the top.

---

## 1. HashMap

Store:

```text
key → Entry
```

Each `Entry` contains:

```text
key
value
expireAt
version
```

The HashMap always contains the **latest version** of a key.

HashMap lookup:

```text
O(1) average
```

---

## 2. Min Heap

Maintain a min heap ordered by `expireAt`.

Each heap entry contains:

```text
expireAt
key
version
```

The top of the heap is always the entry that expires first.

Therefore, expiration cleanup only needs to process entries from the top:

```cpp
while (!minHeap.empty() && minHeap.top().expireAt <= now) {
    ...
}
```

---

# 3. Lazy Deletion

We don't need to immediately remove an old entry from the heap when a key is updated.

Example:

```text
put(A, 10) at t = 0
    ↓
A expires at t = 10

put(A, 20) at t = 5
    ↓
A now expires at t = 15
```

The heap contains:

```text
(A, version 1, expires at 10)   ← stale
(A, version 2, expires at 15)   ← current
```

At `t = 10`, the old entry reaches the top.

We check its version against the current HashMap entry:

```cpp
if (heapEntry.version != currentEntry.version)
    continue;
```

Since the versions don't match, the heap entry is stale and we simply discard it.

At `t = 15`, the current entry reaches the top and is actually removed.

---

# 4. Versioning

Every time a key is updated, increment its version.

Example:

```text
put(A, 10) → version 1
put(A, 20) → version 2
put(A, 30) → version 3
```

The HashMap contains:

```text
A → value 30, version 3
```

The heap may contain:

```text
A → version 1
A → version 2
A → version 3
```

When an old heap entry expires:

```cpp
if (entry.version != entriesMap[entry.key].version)
    continue;
```

This prevents an old expiration record from deleting a newer value.

---

# 5. Running Sum and Count

Maintain:

```text
runningSum
runningCount
```

Example:

```text
A = 10
B = 20
C = 30

runningSum = 60
runningCount = 3
```

Then:

```text
average = runningSum / runningCount
        = 60 / 3
        = 20
```

When an entry expires:

```cpp
runningSum -= expiredValue;
runningCount--;
```

Therefore, `get_average()` doesn't need to iterate over all entries.

---

# Operations

## put(key, value)

1. Remove expired entries.
2. Check whether the key already exists.
3. If it exists, remove its old value from `runningSum` and `runningCount`.
4. Increment the key's version.
5. Create a new entry:

   ```text
   expireAt = now + EXPIRY_TIME
   ```
6. Store the entry in the HashMap.
7. Push the entry into the min heap.
8. Add the new value to `runningSum` and `runningCount`.

### Complexity

```text
O(log N)
```

because inserting into the min heap costs `O(log N)`.

---

## get(key)

1. Remove expired entries.
2. Look up the key in the HashMap.
3. If it doesn't exist, return `not_found`.
4. Otherwise return its value.

### Complexity

```text
O(1) normally
O(log N) amortized when cleanup is required
```

---

## get_average()

1. Remove expired entries.
2. If there are no active entries, return `0`.
3. Return:

   ```text
   runningSum / runningCount
   ```

### Complexity

```text
O(1) normally
O(log N) amortized when cleanup is required
```

---

# Expiration Cleanup

```cpp
void removeExpired() {

    auto now = chrono::steady_clock::now();

    while (!minHeap.empty() &&
           minHeap.top().expireAt <= now) {

        Entry entry = minHeap.top();
        minHeap.pop();

        // Key was already removed
        if (!entriesMap.count(entry.key))
            continue;

        // Stale heap entry
        if (entry.version != entriesMap[entry.key].version)
            continue;

        // Current entry has actually expired
        runningSum -= entry.value;
        runningCount--;

        entriesMap.erase(entry.key);
    }
}
```

---

# Complexity Analysis

Let `N` be the number of entries.

| Operation       | Complexity                                         |
| --------------- | -------------------------------------------------- |
| HashMap lookup  | `O(1)` average                                     |
| `put()`         | `O(log N)`                                         |
| `get()`         | `O(1)` normally, `O(log N)` amortized with cleanup |
| `get_average()` | `O(1)` normally, `O(log N)` amortized with cleanup |
| Heap insertion  | `O(log N)`                                         |
| Heap removal    | `O(log N)`                                         |
| Space           | `O(N)`                                             |

## Why is cleanup amortized?

Each heap entry is:

* Inserted once → `O(log N)`
* Removed once → `O(log N)`

Therefore, even if one API call removes many expired entries, the total cleanup cost across all operations is:

```text
O(log N) amortized per heap entry
```

---

# Thread Safety

If multiple threads can call `get()`, `put()`, and `get_average()` simultaneously, the class is **not thread-safe** in its current form.

The following shared state can be modified concurrently:

```text
entriesMap
minHeap
runningSum
runningCount
```

We need to protect these operations using a mutex.

## 1. Add a mutex

Include:

```cpp
#include <mutex>
```

Then add:

```cpp
mutable std::mutex mtx;
```

inside `KeyStore`.

---

## 2. Lock each public API

The simplest approach is to use `std::lock_guard`.

### put()

```cpp
void put(int key, int value) {

    std::lock_guard<std::mutex> lock(mtx);

    removeExpired();

    // existing put logic...
}
```

### get()

```cpp
int get(int key) {

    std::lock_guard<std::mutex> lock(mtx);

    removeExpired();

    auto it = entriesMap.find(key);

    if (it == entriesMap.end())
        return not_found;

    return it->second.value;
}
```

### average()

```cpp
double average() {

    std::lock_guard<std::mutex> lock(mtx);

    removeExpired();

    if (runningCount == 0)
        return 0.0;

    return (double)runningSum / runningCount;
}
```

---

## 3. Important: Don't lock `removeExpired()` separately

`removeExpired()` is called by `get()`, `put()`, and `average()`.

If those methods already hold the mutex, don't do this:

```cpp
void removeExpired() {
    std::lock_guard<std::mutex> lock(mtx);  // ❌
}
```

Otherwise you would try to lock the same non-recursive mutex twice:

```text
get()
 ↓
lock mutex
 ↓
removeExpired()
 ↓
lock mutex again
 ↓
DEADLOCK
```

Instead, make `removeExpired()` a private helper that assumes the caller already holds the lock:

```cpp
private:

void removeExpired() {
    // No locking here
}
```

---

# Thread-Safe Structure

```cpp
class KeyStore {

private:

    mutable std::mutex mtx;

    chrono::seconds ttl;

    unordered_map<int, Entry> entriesMap;

    long long runningSum = 0;
    int runningCount = 0;

    priority_queue<
        Entry,
        vector<Entry>,
        greater<Entry>
    > minHeap;

    void removeExpired() {
        // Caller must already hold mtx

        auto now = chrono::steady_clock::now();

        while (!minHeap.empty() &&
               minHeap.top().expireAt <= now) {

            Entry entry = minHeap.top();
            minHeap.pop();

            auto it = entriesMap.find(entry.key);

            if (it == entriesMap.end())
                continue;

            if (entry.version != it->second.version)
                continue;

            runningSum -= it->second.value;
            runningCount--;

            entriesMap.erase(it);
        }
    }

public:

    void put(int key, int value) {

        std::lock_guard<std::mutex> lock(mtx);

        removeExpired();

        // put logic
    }

    int get(int key) {

        std::lock_guard<std::mutex> lock(mtx);

        removeExpired();

        auto it = entriesMap.find(key);

        if (it == entriesMap.end())
            return not_found;

        return it->second.value;
    }

    double average() {

        std::lock_guard<std::mutex> lock(mtx);

        removeExpired();

        if (runningCount == 0)
            return 0.0;

        return (double)runningSum / runningCount;
    }
};
```

---

# Why One Mutex Is Enough

All operations modify/read related pieces of state:

```text
entriesMap
minHeap
runningSum
runningCount
```

These must remain consistent with each other.

For example, this update:

```cpp
entriesMap[key] = entry;
runningSum += value;
runningCount++;
minHeap.push(entry);
```

must be treated as **one atomic operation** from the perspective of other threads.

Using one mutex guarantees that another thread cannot observe a partially updated state.

---

# Can We Use shared_mutex?

A `std::shared_mutex` could theoretically allow multiple readers:

```text
get()          → shared lock
average()      → shared lock
put()          → exclusive lock
```

But there is an important problem:

`get()` and `average()` call `removeExpired()`, and expiration cleanup **modifies**:

```text
entriesMap
minHeap
runningSum
runningCount
```

Therefore, they aren't actually read-only.

So with the current lazy-deletion design, a normal:

```cpp
std::mutex
```

is the simplest and safest solution.

If you wanted maximum read concurrency, you would need a more sophisticated design where expiration cleanup is separated from read operations.

---

# Final Interview Summary

### Single-threaded

```text
HashMap
    ↓
O(1) lookup

Min Heap
    ↓
O(log N) expiration handling

Versioning
    ↓
Handles stale entries after updates

Running Sum + Count
    ↓
O(1) average
```

### Multi-threaded

Protect all shared state with:

```cpp
std::mutex
```

and:

```cpp
std::lock_guard<std::mutex> lock(mtx);
```

at the beginning of every public API.

The key rule is:

> **Hold the mutex for the entire operation, including expiration cleanup, because cleanup modifies the shared state.**
