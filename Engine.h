#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>   
#include <vector>     
#include "BST.h"      
#include "Record.h"
//add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine {
    vector<Record> heap;                  // the main data store (simulates a heap file)
    BST<int, int> idIndex;                // index by student ID
    BST<string, vector<int>> lastIndex;   // index by last name (can have duplicates)

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record &recIn) {
        heap.push_back(recIn); //Adds new record to heap (main storage)

        int rec_ind = heap.size() - 1; //Gets heap index value to act as value

        idIndex.insert(recIn.id, rec_ind); //Inserts student id (key) to idIndex (with rec_ind as value)

        vector<int> *lname = lastIndex.find(toLower(recIn.last)); //Gets vector associated with last name

        if(lname == nullptr) {//If this is the first instance of last name
            vector<int> vectIndex = {rec_ind}; //creates new vector
            lastIndex.insert(toLower(recIn.last), vectIndex); //adds new last:vector pair to lastIndex
        } else{
            lname->push_back(rec_ind); //Adds index to existing vector
        }

        idIndex.resetMetrics(); //Resets comparison counters
        lastIndex.resetMetrics();
        return rec_ind;
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id) {
        int *index = idIndex.find(id); //Finds index value from idIndex

        if(index == nullptr) return false; //If the id doesn't exist, returns false

        heap[*index].deleted = true; //soft delete from heap
        vector<int> *lname = lastIndex.find(toLower(heap[*index].last)); //Gets vector from last name

        for(int i = 0; i < lname->size(); i++) { //Loops through
            if(lname->at(i) == *index) {
                lname->erase(lname->begin() + i); //Erases index from lastIndex vector
                break; //Stops counting through everything
            }
        }

        idIndex.erase(id); //Erases id from idIndex

        idIndex.resetMetrics(); //Resets counter
        lastIndex.resetMetrics();
        return true;
    }

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record *findById(int id, int &cmpOut) {
        int *rid = idIndex.find(id);
        if(rid == nullptr) { //If no such id is found in idIndex
            cmpOut = idIndex.comparisons; //Sets to comparison counter in idIndex
            idIndex.resetMetrics();

            cout << cmpOut << endl; //prints comparisons
            return nullptr; //returns nullptr as no student found
        }

        Record *student = &heap.at(*rid); //Saves address of student record with idIndex

        cmpOut = idIndex.comparisons; //Sets to comparison counter in idIndex
        idIndex.resetMetrics();

        return student; //returns pointer to student record
    }

    // Returns all records with ID in the range [lo, hi].
    // Also reports the number of key comparisons performed.
    vector<const Record *> rangeById(int lo, int hi, int &cmpOut) {
        vector<const Record *> students; //Creates vector to be returned

        idIndex.rangeApply(lo, hi, [&](const int &k, int &rid) { //Runs through every key in lo-hi range
            students.push_back(&heap.at(rid)); //Pushes value from heap into students vector
        });

        cmpOut = idIndex.comparisons; //Gets comparisons
        idIndex.resetMetrics();

        return students; //Returns students
    }

    // Returns all records whose last name begins with a given prefix.
    // Case-insensitive using lowercase comparison.
    vector<const Record *> prefixByLast(const string &prefix, int &cmpOut) {
        vector<const Record *> students; //Creates vector to be returned

        string lowerpre = toLower(prefix);
        string lo = lowerpre; //Assuming prefix "Mc" would include "Mc" as a valid answer
        string hi = lowerpre + "z"; //Unsure of how to change "Mc" to "Md", so I set high to "Mcz" which covers the same bases (assuming inclusive)

        lastIndex.rangeApply(lo, hi, [&](const string &k, vector<int> &rid) {
            for(int i : rid) {
                students.push_back(&heap.at(i)); //Pushes value from each student in heap with index in last name vector to students output
            }
        });

        cmpOut = lastIndex.comparisons; //Gets comparisons
        lastIndex.resetMetrics();
        return students; //Returns students
    }
};

#endif
