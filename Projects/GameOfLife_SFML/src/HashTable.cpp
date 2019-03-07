#include "HashTable.h"

#include <vector>
#include <utility>

template<typename Key, typename T, typename HashFunctionObject> typename HashTable<Key, T, HashFunctionObject>::reference
HashTable<Key, T, HashFunctionObject>::insert(std::pair<Key, value_type> pair) {
	using std::swap;

	if (recordsUsed > size() / 1.1)
		rehash();

	auto s = size();

	size_type currIndex = getIndexFromHash(getHash(key), s);
	size_type probeCount = 0;

	for (; currIndex < s; ++currIndex, ++probeCount) { //Linear probing.
		Record &currentRecord = hashTableContainer[currIndex];

		auto currRecordProbeCount = getProbeCount(currentRecord, currIndex);

		if (!currentRecord.getActive()) { //If the record is empty (not active).
			currentRecord.setKey(pair.first);
			currentRecord.setValue(pair.second);
			currentRecord.setActive(true);
			activeContainer.push_back(pair.first);
			++recordsUsed;

			return currentRecord.getValue();
		}
		else {
			if (currentRecord.getKey() == pair.first) {
				return currentRecord.getValue();
			}
			else if (currRecordProbeCount < probeCount) { //Robin Hood hashing.
				swap(currentRecord.getKey(), pair.first);
				swap(currentRecord.getValue(), pair.second);
				probeCount = currRecordProbeCount;
			}
		}
	}

	//No unactive record found. Rehash.
	rehash();
	return insert(pair); //Retry.
}

template<typename Key, typename T, typename HashFunctionObject>
typename HashTable<Key, T, HashFunctionObject>::value_type *HashTable<Key, T, HashFunctionObject>::find(Key key) {
	auto s = size();

	for (size_type start = getIndexFromHash(getHash(key), s); start < s; ++start) { //Linear probing.
		Record &currentRecord = hashTableContainer[start];
		if (!getRecordActive(currentRecord))
			return nullptr;

		else if (currentRecord.getKey() == key)
			return &currentRecord.getValue();
	}

	return nullptr;
}

template<typename Key, typename T, typename HashFunctionObject>
void HashTable<Key, T, HashFunctionObject>::erase(Key key) {
	using std::swap;

	auto s = size();
	for (auto start = getIndexFromHash(getHash(key)); start < s; ++start) {
		Record &currentRecord = hashTableContainer[start];

		if (!currentRecord.getActive())
			continue;
		else if (currentRecord.getKey() == key) {
			currentRecord.setActive(false); //Deactives current record.
			--recordsUsed;

			//Move elements with a greater probeCount closer to its starting hash index.
			for (; start + 1 < s; ++start) {
				currentRecord = hashTableContainer[start];
				auto nextRecord = hashTableContainer[start + 1];
				if (!currentRecord.getActive() && nextRecord.getActive() && getProbeCount(nextRecord, start + 1) > 0) {
					swap(currentRecord.getKey(), nextRecord.getKey());
					swap(currentRecord.getValue(), nextRecord.getValue());
					currentRecord.setActive(true);
					nextRecord.setActive(false);
				}
				else
					return;
			}
			return;
		}
	}
}

template<typename Key, typename T, typename HashFunctionObject>
void HashTable<Key, T, HashFunctionObject>::rehash() {
	auto newSize = size() * 2;
	HashTable newHashTable(newSize);

	for (Record &record : hashTableContainer) {
		if (record.getActive()) {
			newHashTable.insert(std::make_pair(record.getKey(), record.getValue()));
		}
	}
	*this = std::move(newHashTable);
}