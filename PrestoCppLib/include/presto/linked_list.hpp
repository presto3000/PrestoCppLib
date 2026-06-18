#pragma once

#include <iostream>
#include <unordered_set>
#include <memory>
#include <stdexcept>
#include <functional>

/**
 * Singly linked list implementation.
 * Provides O(1) head/tail operations and various utility methods.
 *
 * @tparam T The data type to store (must be copyable/moveable)
 */
template <typename T>
class LinkedList {

private:
	/**
	* Generic Node for a singly linked list.
	* @tparam T The data type to store in each node
	*/
	template <typename T>
	class Node {
	public:
		explicit Node(const T& val) : value(val), next(nullptr) {}
		explicit Node(T&& val) : value(std::move(val)), next(nullptr) {}

		T value;
		Node* next;
	};

	// ============ Helper Methods ============

	/**
	 * Gets the node at a specific index without bounds checking.
	 * Internal use only.
	 */
	Node<T>* getNodeAt(int index) const {
		Node<T>* current = head;
		for (int i = 0; i < index; i++) {
			current = current->next;
		}
		return current;
	}

	// ============ Member Variables ============
	Node<T>* head;
	Node<T>* tail;
	int length;

public:

	// ================= ITERATOR =================
	class Iterator {
		friend class LinkedList;

	private:
		Node<T>* node;

		explicit Iterator(Node<T>* n) : node(n) {}

	public:
		Iterator() : node(nullptr) {}

		T& operator*() { return node->value; }
		const T& operator*() const { return node->value; }

		Iterator& operator++() {
			node = node->next;
			return *this;
		}

		bool operator!=(const Iterator& other) const {
			return node != other.node;
		}

		bool operator==(const Iterator& other) const {
			return node == other.node;
		}
	};

	/**
	 * Creates an empty linked list.
	 */
	LinkedList() : head(nullptr), tail(nullptr), length(0) {}

	/**
	 * Creates a linked list with an initial value.
	 * @param value The initial value
	 */
	explicit LinkedList(const T& value) : length(0) {
		Node<T>* newNode = new Node<T>(value);
		head = newNode;
		tail = newNode;
		length = 1;
	}

	~LinkedList() {
		clear();
	}

	// ============ Rule of Five (Prevent shallow copying) ============

	LinkedList(const LinkedList&) = delete;
	LinkedList& operator=(const LinkedList&) = delete;
	// Move constructor - efficiently transfers ownership.
	LinkedList(LinkedList&& other) noexcept
		: head(other.head), tail(other.tail), length(other.length) {
		other.head = nullptr;
		other.tail = nullptr;
		other.length = 0;
	}

	LinkedList& operator=(LinkedList&& other) noexcept {
		if (this != &other) {
			clear();
			head = other.head;
			tail = other.tail;
			length = other.length;
			other.head = nullptr;
			other.tail = nullptr;
			other.length = 0;
		}
		return *this;
	}

	// ============ Accessors ============

	 // Prints all values in the list (space-separated).
	void printList() const {
		std::cout << "[ ";
		Node<T>* temp = head;
		while (temp) {
			std::cout << temp->value << " ";
			temp = temp->next;
		}
		std::cout << "]\n";
	}

	const T& getHead() const {
		if (!head) {
			throw std::out_of_range("List is empty");
		}
		return head->value;
	}

	const T& getTail() const {
		if (!tail) {
			throw std::out_of_range("List is empty");
		}
		return tail->value;
	}

	int getLength() const {
		return length;
	}

	bool isEmpty() const {
		return length == 0;
	}

	Iterator begin() { return Iterator(head); }
	Iterator end() { return Iterator(nullptr); }

	// ============ Insertion Operations ============

	 // Adds a value to the end of the list. O(1)
	void append(const T& value) {
		Node<T>* newNode = new Node<T>(value);
		if (!head) {
			head = newNode;
			tail = newNode;
		}
		else {
			tail->next = newNode;
			tail = newNode;
		}
		length++;
	}

	void append(T&& value) {
		Node<T>* newNode = new Node<T>(std::move(value));
		if (!head) {
			head = newNode;
			tail = newNode;
		}
		else {
			tail->next = newNode;
			tail = newNode;
		}
		length++;
	}

	// Adds a value to the beginning of the list. O(1)
	// @param value The value to prepend
	void prepend(const T& value) {
		Node<T>* newNode = new Node<T>(value);
		if (!head) {
			head = newNode;
			tail = newNode;
		}
		else {
			newNode->next = head;
			head = newNode;
		}
		length++;
	}

	void prepend(T&& value) {
		Node<T>* newNode = new Node<T>(std::move(value));
		if (!head) {
			head = newNode;
			tail = newNode;
		}
		else {
			newNode->next = head;
			head = newNode;
		}
		length++;
	}

	// Inserts a value at a specific index. O(n)
	void insert(int index, const T& value) {
		if (index < 0 || index > length) {
			throw std::out_of_range("Index out of bounds");
		}

		if (index == 0) {
			prepend(value);
			return;
		}

		if (index == length) {
			append(value);
			return;
		}

		Node<T>* newNode = new Node<T>(value);
		Node<T>* prev = getNodeAt(index - 1);

		newNode->next = prev->next;
		prev->next = newNode;
		length++;
	}

	void insert(int index, T&& value) {
		if (index < 0 || index > length)
			throw std::out_of_range("Index out of bounds");

		if (index == 0) {
			prepend(std::move(value));
			return;
		}

		if (index == length) {
			append(std::move(value));
			return;
		}

		Node<T>* newNode = new Node<T>(std::move(value));
		Node<T>* prev = getNodeAt(index - 1);

		newNode->next = prev->next;
		prev->next = newNode;

		++length;
	}

	// ============ Deletion Operations ============

	void deleteFirst() {
		if (!head) {
			throw std::out_of_range("Cannot delete from empty list");
		}

		Node<T>* temp = head;
		head = head->next;
		delete temp;

		if (!head) {
			tail = nullptr;
		}
		length--;
	}

	void deleteLast() {
		if (!head) {
			throw std::out_of_range("Cannot delete from empty list");
		}

		if (head == tail) {
			delete head;
			head = nullptr;
			tail = nullptr;
		}
		else {
			Node<T>* current = head;
			while (current->next != tail) {
				current = current->next;
			}
			delete tail;
			tail = current;
			tail->next = nullptr;
		}
		length--;
	}

	Iterator erase(int index) {
		if (index < 0 || index >= length)
			return end();

		auto it = begin();

		for (int i = 0; i < index; ++i)
			++it;

		return erase(it);
	}

	Iterator erase(Iterator it) {
		if (!it.node)
			return end();

		// erase head
		if (it.node == head) {
			Node<T>* old = head;
			head = head->next;

			if (tail == old)
				tail = nullptr;

			delete old;
			--length;

			return Iterator(head);
		}

		// find previous (STL tradeoff: singly list needs this)
		Node<T>* prev = head;
		while (prev && prev->next != it.node)
			prev = prev->next;

		if (!prev)
			return end();

		prev->next = it.node->next;

		if (it.node == tail)
			tail = prev;

		Iterator next_it(prev->next);

		delete it.node;
		--length;

		return next_it;
	}

	/**
	 * Removes all nodes from the list.
	 */
	void clear() {
		Node<T>* current = head;
		while (current) {
			Node<T>* nextNode = current->next;
			delete current;
			current = nextNode;
		}
		head = nullptr;
		tail = nullptr;
		length = 0;
	}

	// ============ Value Operations ============

	const T& at(int index) const {
		if (index < 0 || index >= length) {
			throw std::out_of_range("Index out of bounds");
		}
		Node<T>* node = getNodeAt(index);
		return node->value;
	}

	void set(int index, const T& value) {
		if (index < 0 || index >= length) {
			throw std::out_of_range("Index out of bounds");
		}
		Node<T>* node = getNodeAt(index);
		node->value = value;
	}

	// ============ Advanced Operations ============

	/**
	 * Reverses the entire linked list in-place. O(n)
	 */
	void reverse() {
		Node<T>* prev = nullptr;
		Node<T>* current = head;
		tail = head;

		while (current) {
			Node<T>* nextNode = current->next;
			current->next = prev;
			prev = current;
			current = nextNode;
		}
		head = prev;
	}

	/**
	 * Finds the middle node of the list. O(n)
	 * For even-length lists, returns the second of the two middle nodes.
	 * @return Value at the middle node
	 * @throw std::out_of_range if list is empty
	 */
	const T& getMiddle() const {
		if (!head) {
			throw std::out_of_range("List is empty");
		}

		Node<T>* slow = head;
		Node<T>* fast = head;

		while (fast && fast->next) {
			slow = slow->next;
			fast = fast->next->next;
		}

		return slow->value;
	}

	/**
	 * Gets the kth node from the end of the list. O(n)
	 * @param k Position from end (1-indexed, so k=1 is the last node)
	 * @return Value at the kth node from end
	 * @throw std::out_of_range if k is invalid
	 */
	const T& getKthFromEnd(int k) const {
		if (k <= 0 || k > length) {
			throw std::out_of_range("Invalid k value");
		}

		Node<T>* fast = head;
		Node<T>* slow = head;

		for (int i = 0; i < k; i++) {
			fast = fast->next;
		}

		while (fast) {
			fast = fast->next;
			slow = slow->next;
		}

		return slow->value;
	}

	/**
	 * Removes all duplicate values from the list (keeps first occurrence).
	 * Requires T to be hashable.
	 * Complexity: O(n) average case
	 */
	void removeDuplicates() {
		if (!head) return;

		std::unordered_set<T> seen;
		Node<T>* curr = head;
		Node<T>* prev = nullptr;

		while (curr) {
			if (seen.find(curr->value) != seen.end()) {
				Node<T>* temp = curr;

				if (prev) {
					prev->next = curr->next;
				}
				else {
					head = curr->next;
				}

				// Update tail if we're deleting the last node
				if (curr == tail) {
					tail = prev;
				}

				curr = curr->next;
				delete temp;
				length--;
			}
			else {
				seen.insert(curr->value);
				prev = curr;
				curr = curr->next;
			}
		}
	}

	/**
	 * Swaps adjacent pairs of nodes. O(n)
	 * Example: [1,2,3,4] becomes [2,1,4,3]
	 */
	void swapPairs() {
		if (!head || !head->next) return;

		Node<T>* dummy = new Node<T>(T());
		dummy->next = head;
		Node<T>* prev = dummy;

		while (prev->next && prev->next->next) {
			Node<T>* first = prev->next;
			Node<T>* second = first->next;

			first->next = second->next;
			second->next = first;
			prev->next = second;

			prev = first;
		}

		head = dummy->next;

		// Update tail
		Node<T>* curr = head;
		while (curr && curr->next) {
			curr = curr->next;
		}
		tail = curr;

		delete dummy;
	}

	/**
	 * Checks if a value exists in the list. O(n)
	 * @param value The value to search for
	 * @return true if found, false otherwise
	 */
	bool contains(const T& value) const {
		Node<T>* curr = head;
		while (curr) {
			if (curr->value == value) {
				return true;
			}
			curr = curr->next;
		}
		return false;
	}

	/**
	 * Applies a function to each element in the list. O(n)
	 * @param func Function to apply to each element
	 */
	void forEach(const std::function<void(const T&)>& func) const {
		Node<T>* curr = head;
		while (curr) {
			func(curr->value);
			curr = curr->next;
		}
	}

	void forEachFn(void (*func)(const T&)) const {
		Node<T>* curr = head;
		while (curr) {
			func(curr->value);
			curr = curr->next;
		}
	}
};

