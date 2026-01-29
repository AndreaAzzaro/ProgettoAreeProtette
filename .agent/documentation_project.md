This document is a collection of all rules that MUST be followed when writing the documentation of the project.

## SETUP RULES

- When given instruction to add a new section to the documentation, you must add it to the file README.at the end of the file. do not remove any part of the file if not requested directly.
- the documentation must be written in the file README.md
- you are NOT ALLOWED to make any changes to the project files
- for writing the documentation you must use markdown language
- for writing the code snippets you must use code blocks
- for writing the comments you must use doxygen style comments
- for writing the documentation you must use italian language

- you can use the systemOverview.md file as a reference for the system architecture
- you can directly use the code from the project as a reference for the code implementation
- you can use the consegna.md file as a reference for the rules that were followed in the project

## General Rules

- all the order you will be given from the chat will be in english, however the documentation must be in italian
- the documentation must be written in a way that is easy to understand for a human reader
- the documentation must be written in a detailed and exhaustive way, with examples and explanations of each component used, such as semaphores, shared memory, etc.
- when a core parallel programming solution is used, it must be explained in detail, with a general overview of why it is used, when it is used and how it works
- when a diagram is used, it must be explained in detail, with a general overview of why it is used, when it is used and how it works
  -your range should be around 200 - 500 lines of explanation for each given section and sub section

## Example Rules

- when explaining a code implementation, a code snippet directly from the project must be provided inside a code block
- when a comment is present in the snippet it must be translated in italian in the snippet
- when a variable is used in the snippet it must be explained in detail, with its purpose and its role in the code
- when a function is used in the snippet it must be explained in detail, with its purpose and its role in the code, using doxygen style comments
- when a data structure is used in the snippet it must be explained in detail, with its purpose and its role in the code, using doxygen style comments
- when a semaphore is used in the snippet it must be explained in detail, with its purpose and its role in the code, using doxygen style comments
- when a shared memory is used in the snippet it must be explained in detail, with its purpose and its role in the code, using doxygen style comments
- when a queue is used in the snippet it must be explained in detail, with its purpose and its role in the code, using doxygen style comments

## Solution Explanation Rules

-when explaining a complex solution you must first give a general overview of the solution, then explain the details of the solution, with examples and explanations of each component used, such as semaphores, shared memory, etc.
-after explaining the solution, you must provide a code snippet that shows the implementation of the solution, using doxygen style comments
-after that start breaking down the solution into smaller parts, explaining each part in detail, with examples and explanations of each component used, such as semaphores, shared memory, etc. using code blocks
