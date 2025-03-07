#include <limits.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#define SAVE_FILE_NAME "phonebook.txt"
// For spacing purposes
#define COLUMN_TAB_WIDTH "\t\t\t\t"
#define DIVIDER                                                                \
    "------------------------------------------------------------------------" \
    "------------"

// Define Person class.
class Person {
  public:
    std::string first;
    std::string last;
    std::string phone_number;
    Person(std::string first, std::string last, std::string phone_number)
        : first(first), last(last), phone_number(phone_number) {}
    Person(std::string first, std::string last) : first(first), last(last) {}

    void display_person() {
        // Helper function to print data.
        std::cout << this->first << COLUMN_TAB_WIDTH << this->last
                  << COLUMN_TAB_WIDTH << this->phone_number << std::endl;
    }

    // Encode person data for save file.
    std::string encode() { return first + "," + last + "," + phone_number; }
    static Person decode(std::string s) {
        // Create Person object from a line in the save file.
        std::string first, last, phone_number;
        int comma_count = 0;
        for (size_t i = 0; i < s.length(); i++) {
            if (s[i] == ',') {
                comma_count++;
                continue;
            }

            switch (comma_count) {
            case 0:
                first.push_back(s[i]);
                break;
            case 1:
                last.push_back(s[i]);
                break;
            case 2:
                phone_number.push_back(s[i]);
                break;
            default:
                break;
            }
        }

        return Person(first, last, phone_number);
    }
};

class BST_Node {
  public:
    Person person;
    BST_Node *left;
    BST_Node *right;

    BST_Node(std::string first, std::string last, std::string phone_number)
        : person(first, last, phone_number), left(nullptr), right(nullptr) {}
};

class Book {
  public:
    Book() : head(nullptr), count(0) {}

    bool add_entry(std::string first, std::string last,
                   std::string phone_number) {
        // Create a new node on the heap.
        first_last_to_upper(first, last);
        BST_Node *new_node = new BST_Node(first, last, phone_number);

        // Check if the tree is empty.
        if (!head) {
            // If empty, make the new node the new head.
            this->head = new_node;
            count++;
            return true;
        }

        BST_Node *ptr = head;
        // Return a boolean value depending on success.
        return !!this->insertion(ptr, new_node);
    }

    void display_book() {
        // Perform an inorder traversal on the tree.
        if (!head) {
            std::cout << "\nNo records\n" << std::endl;
            return;
        }

        std::cout << "Phonebook contains " << count << " entries." << std::endl;
        std::cout << "First" << COLUMN_TAB_WIDTH << "Last" << COLUMN_TAB_WIDTH
                  << "Phone Number" << std::endl;
        std::cout << DIVIDER << std::endl;
        // Call recursive function to perform in order traversal.
        inorder_display(head);
        std::cout << DIVIDER << std::endl;
    }

    BST_Node *find_entry(std::string first, std::string last) {
        BST_Node *ptr = head;
        // Convert first and last name to uppercase.
        first_last_to_upper(first, last);
        // Run locate node recursive search.
        Person p{first, last};
        return locate_node(ptr, &p, false);
    }

    Person *change_entry(std::string first, std::string last,
                         std::string phone_number) {
        // Convert first and last to uppercase.
        first_last_to_upper(first, last);

        // Locate the node.
        BST_Node *entry = find_entry(first, last);

        if (!entry) {
            std::cout << "\nCould not locate entry" << std::endl;
            return nullptr;
        }

        if (phone_number.length() <= 0) {
            std::cout << "\nPhone number cannot be blank" << std::endl;
            return nullptr;
        }

        entry->person.phone_number = phone_number;
        return &entry->person;
    }

    bool delete_entry(std::string first, std::string last) {
        /***
         * I did deletion a little weird here. I wanted a certain node's memory
         * address to always point to the same value throughout that node's
         * entire lifetime. When deleting an node with both a left and right
         * child, normally we perform a value swapping operation with the node
         * and its in order successor. However, this could lead to unexpected
         * behavior. Say you have a pointer to Node B. Node B happens to be the
         * in order successor to Node A. Their values get swapped and then Node
         * B gets deleted. From your pointer variable's perspective this was
         * unexpected behavior and now your pointer is broken. In order to fix
         * this, I do deletion without any swapping. This means that the
         * pointers are maintained throughout a node's life.
         */

        // Convert to uppercase.
        first_last_to_upper(first, last);
        BST_Node *ptr = head;
        Person p{first, last};

        int direction =
            0; // -1 Means the node to delete is on the left, 0 means it's equal
               // to the parent, and 1 means it's on the right.

        // Locate the parent of the node we wish to delete
        BST_Node *parent = locate_node(ptr, &p, true);

        // locate_node should never return nullptr in this case. So, end here
        // if the value to delete doesn't exist. locate_node will return
        // the value itself if it has no parent (ie. it's the root node).
        if (!parent)
            return false;

        // A pointer to keep track of the actual node to delete.
        BST_Node *entry;

        // We set the node depending on whether or not it's the left or right
        // child of the parent.
        if (parent->left && compare_names(parent->left->person, p) == 0) {
            direction = -1;
            entry = parent->left;
        } else if (parent->right &&
                   compare_names(parent->right->person, p) == 0) {
            direction = 1;
            entry = parent->right;
        } else {
            // This means that we're dealing with the root node as the parent is
            // equal to the node to delete.
            entry = parent;
        }

        // This set of if statements determines how many children the node has.
        if (entry->left && entry->right) {
            // The node has both its children.
            // We need to find the minimum element of the right subtree.
            BST_Node *next_element = entry->right;
            BST_Node *next_element_parent =
                entry; // Keep track of the next element's parent as well.

            // Recurse down the left side of the node's right subtree.
            while (next_element && next_element->left) {
                next_element_parent = next_element;
                next_element = next_element->left;
            }

            // If the node is the right subchild of its parent
            if (direction == 1) {
                // Move the parent's right pointer to the node's next greatest
                // element.
                parent->right = next_element;
            } else if (direction == -1) {
                // Do the opposite of the above.
                parent->left = next_element;
            } else {
                // If direction == 0, then we need to set head to be the next
                // element.
                head = next_element;
            }

            if (next_element != entry->right) {
                // If the next element isn't the node's direct child, then we
                // need to reposition its parent to point to the next value
                // beyond the next element.
                next_element_parent->left = next_element->left;
                // We also need to move the right pointer of the right child of
                // the deleted node.
                next_element->right = entry->right;
            }

            // The replacement node's left child should match the deleted node's
            // left child.
            next_element->left = entry->left;
            // Deallocate the deleted node.
            delete entry;
            // Decrement the counter.
            count--;
            return true;
        } else if (entry->left) {
            // Node only has left child. Depending on the direction, jump over
            // the node to be deleted.
            if (direction == -1) {
                parent->left = entry->left;
            } else if (direction == 1) {
                parent->right = entry->left;
            } else {
                head = entry->left;
            }

            // Deallocate the deleted node.
            delete entry;
            count--;
        } else if (entry->right) {
            // Same process as above, just the opposite side.
            if (direction == -1) {
                parent->left = entry->right;
            } else if (direction == 1) {
                parent->right = entry->right;
            } else {
                head = entry->right;
            }
            delete entry;
            count--;
        } else {
            // Node is a leaf node. Set it to nullptr and update its parent.
            delete entry;
            count--;
            entry = nullptr;
            if (direction == 1) {
                parent->right = nullptr;
            } else if (direction == -1) {
                parent->left = nullptr;
            } else {
                head = nullptr;
            }
        }

        return true;
    }

    bool save() {

        // Nothing to save.
        if (!head) {
            return false;
        }

        // Create a list to store the nodes as we do a preorder traversal.
        BST_Node **preorder_list = new BST_Node *[count];
        BST_Node *ptr = head;
        size_t counter = 0;
        build_preorder_list(ptr, preorder_list, counter);

        std::ofstream File(SAVE_FILE_NAME);
        File.clear();

        // Encode a line for each node we visit.
        for (size_t i = 0; i < count; i++) {
            Person p = preorder_list[i]->person;
            File << p.encode() << "\n";
        }
        // Clean up the list we used.
        delete[] preorder_list;
        // Close the file.
        File.close();
        return true;
    }

    bool load() {
        // Load a saved phonebook.
        if (!std::filesystem::exists(SAVE_FILE_NAME)) {
            // Ensure the save file exists.
            std::cout << "No save file located, please save a phonebook "
                         "before loading."
                      << std::endl;
            return false;
        }

        std::ifstream File(SAVE_FILE_NAME);
        std::string line;

        // Clear the phonebook if we're going to load a new one in.
        clear();
        while (getline(File, line)) {
            // Decode the file line by line and build a new tree based
            // on the nodes read.
            Person p = Person::decode(line);
            add_entry(p.first, p.last, p.phone_number);
        }

        File.close();
        return true;
    }

    void clear() {
        // Clear out the BST.
        clear_BST(head);
        head = nullptr;
        count = 0;
    }

  private:
    BST_Node *head;
    int count;

    BST_Node *insertion(BST_Node *ptr, BST_Node *new_node) {
        // Check whether or not the new node is less than the pointer
        if (compare_names(ptr->person, new_node->person) == 1) {
            // If new_node belongs to the left of the pointer but the left child
            // is nullptr then we can assign it to be the new_node.
            if (!ptr->left) {
                ptr->left = new_node;
                count++;
                return new_node;
            }

            // Otherwise, we can recurse down to the left.
            return insertion(ptr->left, new_node);
        } else if (compare_names(new_node->person, ptr->person) == 1) {
            // Same process as above just for the right side.
            if (!ptr->right) {
                ptr->right = new_node;
                count++;
                return new_node;
            }
            return insertion(ptr->right, new_node);
        } else {
            std::cout << "\nName already exists in phonebook\n" << std::endl;
            return nullptr;
        }

        return nullptr;
    }

    void inorder_display(BST_Node *ptr) {
        // Recursive base case
        if (!ptr) {
            return;
        }
        // Recurse down left subtree
        inorder_display(ptr->left);
        // /Once down the left sub-tree as the function calls get popped off the
        // stack they return here in their execution and will print the
        // information.
        ptr->person.display_person();
        inorder_display(ptr->right);
        // Go down the right side of the tree.
    }

    BST_Node *locate_node(BST_Node *ptr, Person *p, bool return_parent) {
        if (!ptr) {
            // If the pointer is nullptr, return nullptr.
            // This means that the tree is either empty
            // or the target isn't in the tree.
            return nullptr;
        }

        if (compare_names(ptr->person, *p) == 0) {
            return ptr;
        }
        // We can compare strings alphabetically so that's what's happening
        // here.
        else if (compare_names(*p, ptr->person) == -1) {
            // If we're returning the parent of the node then we check if we
            // need to return with this if statement.
            if (return_parent && ptr->left &&
                compare_names(ptr->left->person, *p) == 0) {
                return ptr;
            }
            // Recurse down left subtree.
            return locate_node(ptr->left, p, return_parent);
        } else {
            if (return_parent && ptr->right &&
                compare_names(ptr->right->person, *p) == 0) {
                return ptr;
            }
            // Recurse down right subtree.
            return locate_node(ptr->right, p, return_parent);
        }
    }

    void build_preorder_list(BST_Node *ptr, BST_Node **l, size_t &counter) {
        // Perform preorder traversal. Root, left, right.
        if (ptr == nullptr) {
            return;
        }

        // The counter is passed by reference to be able to keep track
        // of it despite the recursive calls. This would not work if
        // it were passed by value.
        l[counter++] = ptr;
        build_preorder_list(ptr->left, l, counter);
        build_preorder_list(ptr->right, l, counter);
    }

    void clear_BST(BST_Node *ptr) {
        // Perform post order traversal to clear the tree.
        if (ptr == nullptr) {
            return;
        }

        clear_BST(ptr->left);
        clear_BST(ptr->right);
        delete ptr;
    }

    int compare_names(Person &p1, Person &p2) {
        // Perform alphabetical comparisons on last and first names.
        if (p1.last > p2.last) {
            return 1;
        } else if (p2.last > p1.last) {
            return -1;
        } else {
            if (p1.first > p2.first) {
                return 1;
            } else if (p2.first > p1.first) {
                return -1;
            } else {
                return 0;
            }
        }
    }

    void first_last_to_upper(std::string &first, std::string &last) {
        // Convert first and last name to upper case in place.
        std::transform(first.begin(), first.end(), first.begin(), ::toupper);
        std::transform(last.begin(), last.end(), last.begin(), ::toupper);
    }
};

class UserInterface {
    // UI class to help organize functions and provide a clean interface.
  public:
    // Store a pointer to a book. This allows us to add a prebuild book if we
    // wanted.
    Book *phonebook;
    UserInterface(Book &phonebook) : phonebook(&phonebook) {
        std::cout << "Initializing program..." << std::endl;
        main_loop();
    }

    void main_loop() {
        // Loop continuously to gather input.
        while (1) {
            display_menu();
            int selection = get_int_input("Please select an option\n: ");
            if (selection < 0) {
                break;
            }

            // This is mostly self explanatory. Just provide menu options for
            // each BST function. Add some basic error handling and then present
            // the information.
            switch (selection) {
            // Each case corresponds to a menu number.
            case 1: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Find entry" << std::endl;
                std::cout << DIVIDER << std::endl;
                std::string first_name = get_string_input("First name: ", true);
                std::string last_name = get_string_input("Last name: ", true);
                BST_Node *entry = phonebook->find_entry(first_name, last_name);

                if (entry) {
                    std::cout << "\nRecord found:\n\n";
                    std::cout << "First" << COLUMN_TAB_WIDTH << "Last"
                              << COLUMN_TAB_WIDTH << "Phone Number"
                              << std::endl;
                    std::cout << DIVIDER << std::endl;
                    entry->person.display_person();
                    std::cout << "\n" << std::endl;
                } else {
                    std::cout << "\nEntry not found\n" << std::endl;
                }
                wait_for_key();
                break;
            }
            case 2: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Add entry" << std::endl;
                std::cout << DIVIDER << std::endl;
                std::string first_name = get_string_input("First name: ", true);
                std::string last_name = get_string_input("Last name: ", true);
                std::string phone_number = get_string_input("Phone number: ");
                std::cout << "\nConfirm this information looks correct? (y/n)"
                          << std::endl;
                std::cout << "First" << COLUMN_TAB_WIDTH << "Last"
                          << COLUMN_TAB_WIDTH << "Phone Number" << std::endl;
                std::cout << DIVIDER << std::endl;
                std::cout << first_name << COLUMN_TAB_WIDTH << last_name
                          << COLUMN_TAB_WIDTH << phone_number << std::endl;
                bool confirmation = get_confirmation(": ");
                if (confirmation) {
                    if (phonebook->add_entry(first_name, last_name,
                                             phone_number)) {
                        std::cout << "\nAdded new entry to phonebook\n"
                                  << std::endl;
                    }
                } else {
                    std::cout << "\nCancelled\n" << std::endl;
                }
                wait_for_key();
                break;
            }
            case 3: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Delete entry" << std::endl;
                std::cout << DIVIDER << std::endl;
                std::string first_name = get_string_input("First name: ", true);
                std::string last_name = get_string_input("Last name: ", true);
                std::cout
                    << "\n Are you sure you wish to delete this entry? (y/n)"
                    << std::endl;
                std::cout << "\t ->" << first_name << " " << last_name << "\t"
                          << std::endl;
                bool confirmation = get_confirmation(": ");
                if (confirmation) {
                    bool result =
                        phonebook->delete_entry(first_name, last_name);
                    if (result) {
                        std::cout << "\nSuccessfully deleted\n" << std::endl;
                    } else {
                        std::cout << "\nEntry not found\n" << std::endl;
                    }
                }
                wait_for_key();
                break;
            }
            case 4: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Change entry" << std::endl;
                std::cout << DIVIDER << std::endl;
                std::string first_name = get_string_input("First name: ", true);
                std::string last_name = get_string_input("Last name: ", true);
                std::string phone_number =
                    get_string_input("New phone number: ");
                std::cout << "\nConfirm this information looks correct? (y/n)"
                          << std::endl;
                std::cout << "\t ->" << first_name << " " << last_name << "\t"
                          << phone_number << std::endl;
                bool confirmation = get_confirmation(": ");
                if (confirmation) {
                    Person *p = phonebook->change_entry(first_name, last_name,
                                                        phone_number);
                    if (!p) {
                        std::cout << "\nFailed" << std::endl;
                    } else {
                        std::cout << "\nSuccess" << std::endl;
                    }
                }

                wait_for_key();

                break;
            }
            case 5: {
                std::cout << "\n";
                phonebook->display_book();
                wait_for_key();
                break;
            }
            case 6: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Clearing phonebook" << std::endl;
                std::cout << DIVIDER << std::endl;
                bool confirmation =
                    get_confirmation("Are you sure you wish to delete the "
                                     "entire phonebook? (y/n)\n: ");
                if (confirmation) {
                    phonebook->clear();
                    std::cout << "\nSuccessfully cleared phonebook\n"
                              << std::endl;
                    wait_for_key();
                } else {
                    std::cout << "\nCancelled\n" << std::endl;
                }
                break;
            }
            case 7: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Save Phonebook" << std::endl;
                std::cout << DIVIDER << std::endl;
                if (phonebook->save()) {
                    std::cout << "\nSuccessfully saved as " << SAVE_FILE_NAME
                              << std::endl;
                } else {
                    std::cout
                        << "\nSave failed. Ensure the phonebook isn't empty."
                        << std::endl;
                }

                wait_for_key();
                break;
            }
            case 8: {
                std::cout << DIVIDER << std::endl;
                std::cout << "Loading phonebook" << std::endl;
                std::cout << DIVIDER << std::endl;
                bool confirmation = get_confirmation(
                    "Are you sure you wish to load a new phonebook? It will "
                    "clear your current entires. (y/n)\n: ");
                if (confirmation) {
                    std::cout << "\n";
                    phonebook->load();
                    phonebook->display_book();
                    std::cout << "\nSuccessfully loaded phonebook\n"
                              << std::endl;
                    wait_for_key();
                } else {
                    std::cout << "\nCancelled\n" << std::endl;
                }
                break;
            }
            case 9: {
                std::cout << "Goodbye" << std::endl;
                return;
            }

            default:
                std::cout << "\nNot an option, please select a number between "
                             "1 and 5\n"
                          << std::endl;
                break;
            }
        }
    }

    void wait_for_key() {
        // As the string below indicates, this waits for user to input something
        std::cin.ignore();
        std::cout << "Press ENTER to continue..." << std::endl;
        std::cin.get();
    }

    std::string get_string_input(std::string prompt, bool upcase) {
        // Gather user input, optionally convert it to uppercase.
        std::string user_input;
        std::cout << prompt;
        std::cin.clear();
        if (std::cin >> user_input && user_input.length() < 250) {
            if (upcase) {
                std::transform(user_input.begin(), user_input.end(),
                               user_input.begin(), ::toupper);
            }
            return user_input;
        }
        return "";
    }

    std::string get_string_input(std::string prompt) {
        // Overload for not converting to uppercase.
        return get_string_input(prompt, false);
    }

    int get_int_input(std::string prompt) {
        // Gather integer input
        std::string user_input;
        int selection;

        while (true) {
            std::cout << prompt;
            std::cin.clear();
            // Limiting user input to 25 characters.
            if (std::cin >> user_input && user_input.length() < 25) {
                try {
                    // Attempt to convert to an int.
                    selection = std::stoi(user_input);
                    if (selection <= INT_MAX) {
                        return selection;
                    }
                } catch (const std::exception &e) {
                    std::cout << "\nInvalid input, please input a number\n"
                              << std::endl;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                                    '\n');
                }
            } else {
                return -1;
            }
        }
    }

    bool get_confirmation(std::string prompt) {
        // Provide a confirmation dialog. Return yes or no result.
        std::string confirmation = get_string_input(prompt, false);
        return (confirmation == "yes" || confirmation == "y" ||
                confirmation == "yee" || confirmation == "ye");
    }

    void display_menu() {
        // Display a menu for the user to look at.
        std::cout << DIVIDER << std::endl;
        std::cout << "Phonebook Program Menu" << std::endl;
        std::cout << DIVIDER << std::endl;
        std::cout << "1. Find entry" << std::endl;
        std::cout << "2. Add entry" << std::endl;
        std::cout << "3. Delete entry" << std::endl;
        std::cout << "4. Change entry" << std::endl;
        std::cout << "5. Display all entries" << std::endl;
        std::cout << "6. Clear all entries" << std::endl;
        std::cout << "7. Save phonebook" << std::endl;
        std::cout << "8. Load phonebook" << std::endl;
        std::cout << "9. Quit\n" << std::endl;
    }
};

int main() {
    Book b = Book();
    UserInterface ui{b};
}