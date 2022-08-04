# ACID_data_structure

Consistent data structure based on C++ lists. All tasks are implemented on separate branches:

task_1: simple consistent data structure with operations for removing, adding, inserting new nodes. The main idea is that when one tries to delete a node it is not deleted completely because of the reference counter, but anyway this node can not be reached by iterators. When the reference counter reaches 0 then the node can be finally removed

task_2: Coarse-grained list. Iterators own the shared lock when reads or the unique lock when insert or change node values

task_3: Medium-graining. Only pointers which undergo operations are locked and the next and the previous ones.

task_4: Fine graining. Garbage collection is implemented. 

task_6: Used transactions and versions for list operations
