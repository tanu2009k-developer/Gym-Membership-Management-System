# FitTrack: Gym Membership Management System

A console-based gym membership manager written in C. It keeps track of members, their plans and fee due dates, and warns the gym owner about memberships that have expired or are about to expire.

## Features
- Add members with a name, phone number, plan and joining date
- Four plans: Monthly, Quarterly, Half-Yearly and Yearly
- Automatic due-date calculation from the joining date and plan length
- Status for every member: **ACTIVE**, **DUE SOON** (within 7 days) or **EXPIRED**
- Renew memberships (expired members restart from today, active members are extended from their due date)
- Alerts view that lists only expired and due-soon members
- Search members by name (partial match, any case)
- Remove members
- Data is saved to a file, so it stays after closing the program

## Concepts Used
Structures, dynamic memory (`malloc`, `realloc`, `free`), pointers, arrays, strings, functions, `switch` statements, loops, file handling (binary read/write) and date/time handling with `time.h`.

## How to Run
Make sure you have a C compiler such as GCC installed.

    gcc gym.c -o gym
    ./gym          # on Windows PowerShell: .\gym

## How to Use
1. Choose `1` to add a member and follow the prompts.
2. Choose `2` to see all members and their status.
3. Choose `4` to see only expired and due-soon members.
4. Choose `7` to save and exit.

Member data is stored in `members.dat` in the folder where you run the program. Delete this file to start fresh.

## Author
Name - Tanu Kumari
UID - 26BAI10167
