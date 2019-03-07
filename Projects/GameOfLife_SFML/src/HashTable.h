#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <vector>
#include <utility>

template<typename Key, typename T, typename HashFunctionObject> class HashTable {
public:
	typedef std::size_t size_type;
	typedef T value_type;
	typedef T &reference;
	typedef const T &const_reference;

private:
	class iterator;
	class const_iterator;
	friend iterator;
	friend const_iterator;

	class Record {
	public:
		Record(Key k, value_type val, bool act) : key{ k }, value{ val }, active{ act } {}

		const bool &getActive() const noexcept {
			return active;
		}

		bool &getActive() noexcept {
			return active;
		}

		void setActive(bool b) {
			active = b;
		}

		const_reference getValue() const noexcept {
			return value;
		}

		reference getValue() noexcept {
			return value;
		}

		void setValue(value_type val) {
			value = val;
		}

		void setKey(Key k) {
			key = k;
		}

		Key &getKey() noexcept {
			return key;
		}

		const Key &getKey() const noexcept {
			return key;
		}

	private:
		value_type value;
		Key key;
		bool active;
	};

	typedef std::vector<Record> hashTableContainerType;
	typedef std::vector<Key> activeContainerType;

public:

	HashTable() : hashTableContainer{ 128,  Record(Key(), value_type(), false) } {}

	class iterator {
		friend HashTable;
	public:
		friend bool operator==(const iterator &it1, const iterator &it2) {
			return it1.index == it2.index;
		}
		friend bool operator!=(const iterator &it1, const iterator &it2) {
			return it1.index != it2.index;
		}

		reference operator*();
		value_type *operator->();
		iterator &operator++() {
			++index;
			return *this;
		}
		iterator &operator--() {
			--index;
			return *this;
		}
		iterator operator++(int) {
			return iterator(base, index++);
		}
		iterator operator--(int) {
			return iterator(base, index--);
		}

	private:
		iterator(HashTable &b, typename activeContainerType::size_type i) : index{ i }, base{ b } {}

		typename activeContainerType::size_type index;
		HashTable &base;
	};

	class const_iterator {
		friend HashTable;
	public:
		friend bool operator==(const const_iterator &it1, const const_iterator &it2) {
			return it1.index == it2.index;
		}
		friend bool operator!=(const const_iterator &it1, const const_iterator &it2) {
			return it1.index != it2.index;
		}

		const_reference operator*();
		const value_type *operator->();
		const_iterator &operator++() {
			++index;
			return *this;
		}
		const_iterator &operator--() {
			--index;
			return *this;
		}
		const_iterator operator++(int) {
			return const_iterator(base, index++);
		}
		const_iterator operator--(int) {
			return const_iterator(base, index--);
		}

	private:
		const_iterator(typename const HashTable &b, typename activeContainerType::size_type i) : index{ i }, base{ b } {}

		typename activeContainerType::size_type index;
		const HashTable &base;
	};

	reference operator[](typename activeContainerType::size_type index) noexcept {
		return *find(activeContainer[index]);
	}

	const_reference operator[](typename activeContainerType::size_type index) const noexcept {
		return *find(activeContainer[index]);
	}

	value_type *find(Key key);

	reference insert(std::pair<Key, value_type> pair);

	void erase(Key position);

	size_type size() const {
		return hashTableContainer.size(); //Must be a power of 2.
	}

	iterator begin() {
		return iterator(*this, 0);
	}

	const_iterator cbegin() const {
		return const_iterator(*this, 0);
	}

	iterator end() {
		return iterator(*this, activeContainer.size());
	}

	const_iterator cend() const {
		return const_iterator(*this, activeContainer.size());
	}

private:
	HashTable(std::size_t s) : hashTableContainer{ s, Record(Key(), value_type(), false) } {}

	size_type getProbeCount(Record r, size_type index) {
		auto s = size();
		auto hashIndex = getIndexFromHash(getHash(r.getKey()), s);

		return index - hashIndex;
	}

	std::size_t getHash(Key pos) const noexcept {
		return HashFunctionObject{}(pos);
	}

	size_type getIndexFromHash(std::size_t hash, size_type containerSize) const noexcept {
		return hash & (containerSize - 1)
	}

	void rehash();

	mutable hashTableContainerType hashTableContainer;
	activeContainerType activeContainer;
	size_type recordsUsed = 0;
};

#endif